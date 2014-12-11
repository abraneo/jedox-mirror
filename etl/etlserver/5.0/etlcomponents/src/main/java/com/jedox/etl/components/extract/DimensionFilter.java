/**
*   @brief <Description of Class>
*
*   @file
*
*   Copyright (C) 2008-2013 Jedox AG
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License (Version 2) as published
*   by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
*
*   This program is distributed in the hope that it will be useful, but WITHOUT
*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*   more details.
*
*   You should have received a copy of the GNU General Public License along with
*   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
*   Place, Suite 330, Boston, MA 02111-1307 USA
*
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible
*   OEM Commercial License.
*
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right
*   (commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import java.util.*;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.ConditionExtended;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.FilterModes;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.LogicalOperators;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.source.filter.Conditions.Condition;
import com.jedox.etl.core.source.filter.Conditions.Modes;
import com.jedox.palojlib.interfaces.*;

/**
 *
 * @author Kais Haddadin
 *
 */
public class DimensionFilter {

	private LinkedHashSet<String> accepted = new LinkedHashSet<String>();
	private HashSet<String> denied = new HashSet<String>();
	private HashSet<String> elementVisited = new HashSet<String>();
	private IDimension dimension;
	private DimensionFilterDefinition dimensionFilterDefinition;
	private static final Log log = new MessageHandler(LogFactory.getLog(DimensionFilter.class));	
	int cc = 0;
	boolean withAttributes;
	//private DimensionFilter_exp parentFilter = null;

	public DimensionFilter(IDimension dimension,DimensionFilterDefinition dimensionFilterDefinition, boolean withAttributes) {
			
			this.withAttributes = withAttributes;
			this.dimension = dimension;
			this.dimensionFilterDefinition = dimensionFilterDefinition;

			//Use dimension cache while filtering
			dimension.setCacheTrustExpiry(3000);
			
			accepted.addAll(getElementNames());
			
			// set the initial status is until here accept all
			// if no filters leave it as accept all and the Filter is an empty filter (not runnable)
			// or if there is no definition (only appear in case Cube Filter)
			if(dimensionFilterDefinition==null || dimensionFilterDefinition.getConditions().size() == 0){
				return;
			}
			// if and only if the first filter is an accept then deny all
			else if(dimensionFilterDefinition.getConditions().get(0).getMode().equals(Modes.ACCEPT)){
				denyAll();
			}
	}
	
	protected IElement[] getElements() {
		return dimension.getElements(withAttributes);
	}
	
	protected List<String> getElementNames() {
		List<String> result = new ArrayList<String>();
		for (IElement e : getElements()) {
			result.add(e.getName());
		}
		return result;
	}

	protected void denyAll() {
		denied.addAll(accepted);
		accepted.clear();
	}

	protected void acceptAll() {
		accepted.addAll(denied);
		denied.clear();
	}
	protected void acceptOnlyBasis() {
		accepted.clear();
		denied.clear();
		for(IElement e: getElements()){
			if(e.getChildCount()==0)
				accepted.add(e.getName());
		}
	}

	protected void acceptRootToConsolidate() {
		accepted.clear();
		denied.clear();
		for(IElement e:getElements()){
			if(e.getChildCount()!=0)
				accepted.add(e.getName());
		}
	}

	public String getName() {
		return dimension.getName();
	}

	public boolean isEmpty(){
		return (dimensionFilterDefinition.getConditionsExtended().size() == 0);
	}

	public void acceptRootElements() {
		IElement[] elements = dimension.getRootElements(false);
		for (IElement element : elements) {
			accepted.add(element.getName());
			denied.remove(element.getName());
		}
	}


	public String[] process() {
		return accepted.toArray(new String[accepted.size()]);
	}

	public HashSet<String> getAccepted() {
		return accepted;
	}

	public IElement[] getElements(String[] names) {
		IElement[] elements = new IElement[names.length];
		for (int i=0; i<names.length; i++) {
			String name = names[i];
			elements[i] = dimension.getElementByName(name, withAttributes);
		}
		return elements;
	}

	protected HashSet<String> filter(Condition a,Set<String> searchRange) {
		LinkedHashSet<String> tempResult =  new LinkedHashSet<String>();
		for(String s:searchRange){
			try {
				if(a.evaluate(s)){
						tempResult.add(s);
				}
			}
			catch (RuntimeException e) {
				log.warn("Cannot evaluate filter condition: "+e.getMessage());
			}

		}
		return tempResult;
	}

	private void filterOnlyNodes(Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		if(m.equals(Modes.ACCEPT)){
			for (String s: filteredNodes){
				accepted.add(s);
				denied.remove(s);
			}
		}
		else{
			for (String s: filteredNodes){
				accepted.remove(s);
				denied.add(s);
			}
		}
	}

	private void filterOnlyRoots(Modes m, String[] filteredNodes){

		// handle the special case
		/*if(filteredNodes.length == elementMap.size()){
			for (String s: filteredNodes){
				Element e = elementMap.get(s);
				if(e.getParentCount() == 0){
					if(m.equals(Modes.ACCEPT)){
						accepted.add(s);
						denied.remove(s);
						}
						else
						{
							accepted.remove(s);
							denied.add(s);
						}
				}
			}
			return;
		}*/

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
				IElement e = dimension.getElementByName(s, withAttributes);
				if(e.getParentCount() != 0){
					String[] parents = new String[e.getParentCount()];
					IElement[] parentsElements =  e.getParents();
					for(int i=0;i<parents.length;i++)
						parents[i] = parentsElements[i].getName();
					filterOnlyRoots(m,parents);
				}
				else{
					if(m.equals(Modes.ACCEPT)){
						accepted.add(s);
						denied.remove(s);
					}
					else
					{
						accepted.remove(s);
						denied.add(s);
					}
				}
			}
		}
	}

	private void filterNodesToBases(Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
			if(m.equals(Modes.ACCEPT)){
				accepted.add(s);
				denied.remove(s);
				}
				else
				{
					accepted.remove(s);
					denied.add(s);
				}
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e.getChildCount() != 0){
				String[] children = new String[e.getChildCount()];
				IElement[] childrenElements =  e.getChildren();
				for(int i=0;i<children.length;i++)
					children[i] = childrenElements[i].getName();
				filterNodesToBases(m,children);
			}
		}
		}
	}

	private void filterNodesToConsolidates(Modes m, String[] filteredNodes){

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e.getChildCount() != 0){
				if(m.equals(Modes.ACCEPT)){
					accepted.add(s);
					denied.remove(s);
					}
					else
					{
						accepted.remove(s);
						denied.add(s);
					}
				String[] children = new String[e.getChildCount()];
				IElement[] childrenElements =  e.getChildren();
				for(int i=0;i<children.length;i++)
					children[i] = childrenElements[i].getName();
				filterNodesToConsolidates(m,children);
			}
		}
		}
	}

	private void filterRootToNodes(Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
			if(m.equals(Modes.ACCEPT)){
				accepted.add(s);
				denied.remove(s);
				}
				else
				{
					accepted.remove(s);
					denied.add(s);
				}
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e != null && e.getParentCount() != 0){
				String[] parents = new String[e.getParentCount()];
				IElement[] parentsElements =  e.getParents();
				for(int i=0;i<parents.length;i++)
					parents[i] = parentsElements[i].getName();
				filterRootToNodes(m,parents);
			}
		}
		}
	}

	private void filterRootToBases(Modes m, String[] filteredNodes){
		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		filterRootToNodes(m, filteredNodes);
		elementVisited.clear();
		filterNodesToBases(m, filteredNodes);
	}
	private void filterRootToConsolidates(Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(FilterModes.rootToConsolidates,m,filteredNodes);
			return;
		}*/

		//replace the leaves with their parents if existed (if they are also roots, ignore them)
		ArrayList<String> processedFilteredNodes = new ArrayList<String>();
		for(String s:filteredNodes){
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e.getChildCount() == 0){
				for(IElement p:e.getParents()){
					processedFilteredNodes.add(p.getName());
				}
			}
			else{
				processedFilteredNodes.add(e.getName());
			}
		}

		filterRootToNodes(m, processedFilteredNodes.toArray(new String[processedFilteredNodes.size()]));
		elementVisited.clear();
		filterNodesToConsolidates(m, processedFilteredNodes.toArray(new String[processedFilteredNodes.size()]));
	}

	/**
	 * In case rootsToNodes, rootToBases,nodesToBases or onlyNodes, this handles the special case when the filteredNodes set is all elements
	 * @param fm
	 * @param m
	 * @param filteredNodes
	 */
	private void handleSpecialCase(Modes m) {
		List<String> names = getElementNames();	
		if(m.equals(Modes.ACCEPT)){
			accepted.addAll(names);
			denied.removeAll(names);
		}
		else
		{
			accepted.removeAll(names);
			denied.addAll(names);
		}
	}

	/**
	 * In case isOnlyBases or rootToConsildates, this handles the special case when the filteredNodes set is all elements
	 * @param fm
	 * @param m
	 * @param filteredNodes
	 */
	private void handleSpecialCase(FilterModes fm,Modes m,String[]filteredNodes ){
		for (String s: filteredNodes){
			IElement e = dimension.getElementByName(s, withAttributes);;
			if((e.getChildCount() != 0 && fm.equals(FilterModes.onlyBases)) || (e.getChildCount() == 0 && fm.equals(FilterModes.rootToConsolidates))){
				if(m.equals(Modes.ACCEPT)){
					accepted.remove(s);
					denied.add(s);
				}
				else
				{
					accepted.add(s);
					denied.remove(s);
				}
			}
			else{
				if(m.equals(Modes.ACCEPT)){
					accepted.add(s);
					denied.remove(s);
				}
				else
				{
					accepted.remove(s);
					denied.add(s);
				}
			}
		}
	}

	private void filterOnlyBases(Modes m, String[]filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(FilterModes.onlyBases,m,filteredNodes);
			return;
		}*/

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){

				elementVisited.add(s);
				IElement e = dimension.getElementByName(s, withAttributes);;
				if(e.getChildCount() != 0){
					String[] children = new String[e.getChildCount()];
					IElement[] childrenElements =  e.getChildren();
					for(int i=0;i<children.length;i++)
						children[i] = childrenElements[i].getName();
					filterOnlyBases(m,children);
				}
				else{
					if(m.equals(Modes.ACCEPT)){
						accepted.add(s);
						denied.remove(s);
					}
					else
					{
						accepted.remove(s);
						denied.add(s);
					}
				}
			}
		}
	}
	
	protected Map<String,List<String>> mapSearchRange(String searchAttribute, HashSet<String> elementSet) {
		Map<String,List<String>> result = new HashMap<String,List<String>>();
		for (String elementName : elementSet) {
			IElement element = dimension.getElementByName(elementName, withAttributes);;
			if (element != null) {
				Object attributeValue = element.getAttributeValue(searchAttribute);
				if (attributeValue == null || attributeValue.toString().trim().isEmpty()) {
					// to be able to filter attributes using isEmpty
					attributeValue = "";
				}
					
				List<String> elements = result.get(attributeValue);
				if (elements == null) {
					elements = new ArrayList<String>();
					result.put(attributeValue.toString(), elements);
				}
				elements.add(element.getName());
				
			}
		}
		return result;
	}

	public void configure() {
		//filter all elements
		for (ConditionExtended a : dimensionFilterDefinition.getConditionsExtended()) {

			LinkedHashSet<String> searchRange =  new LinkedHashSet<String>();
			if((a.getMode().name().equals("ACCEPT") && (a.getLogicalOperator()==null || a.getLogicalOperator().equals(LogicalOperators.OR)) ) || 
				(a.getMode().name().equals("DENY") && a.getLogicalOperator()!=null && a.getLogicalOperator().equals(LogicalOperators.OR) ) || 
				(a.getMode().name().equals("DENY") && a.equals(dimensionFilterDefinition.getConditions().get(0)))){
				searchRange.addAll(getElementNames());
			}
			//other cases where (No parent and it is a deny filter but it is not the first filter)
			// or (with a parent filter)
			else{
				searchRange.addAll(accepted);
			}

			//this is special to SubsetEvaluator since the filter needs the Dimension to know the elements the belong to the subset that is read in its configurator
			//TODO
			//if(a.getEvaluator() instanceof SubsetEvaluator ){
			//	((SubsetEvaluator)a.getEvaluator()).setElements(dimension);
			//}
			HashSet<String> tempResult = null;
			if (a.getSearchAttribute() == null) {
				tempResult = filter(a,searchRange);
			} else {
				 Map<String,List<String>> nameMap = mapSearchRange(a.getSearchAttribute(),searchRange);
				 HashSet<String> attributeResult = filter(a,nameMap.keySet());
				 tempResult = new HashSet<String>();
				 for (String s : attributeResult) {
					 tempResult.addAll(nameMap.get(s));
				 }
			}
			
			if(a.getMode().name().equals("ACCEPT") && a.getLogicalOperator()!=null && a.getLogicalOperator().equals(LogicalOperators.AND)){
				accepted.clear();
				accepted.addAll(tempResult);
			}
			
			if(a.getMode().name().equals("DENY") && a.getLogicalOperator()!=null && a.getLogicalOperator().equals(LogicalOperators.OR)){
				tempResult.removeAll(accepted);
				accepted.addAll(getElementNames());
			}

			switch (a.getFilterMode()) {
			case nodesToBases : {
				filterNodesToBases(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case rootToBases: {
				filterRootToBases(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case rootToNodes: {
				filterRootToNodes(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case onlyBases: {
				filterOnlyBases(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case onlyNodes: {
				filterOnlyNodes(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case onlyRoots: {
				filterOnlyRoots(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case rootToConsolidates: {
				filterRootToConsolidates(a.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			default: break;
			}
			/*for testing
			 * System.out.println( "  My result after " + a.getFilterMode());
			for(String s:getAccepted()){
				System.out.println( s + " - ");
			}*/
		}
	}


}
