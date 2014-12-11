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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import java.util.*;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.TreeCondition;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.FilterModes;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.LogicalOperators;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.source.filter.Conditions.Condition;
import com.jedox.etl.core.source.filter.Conditions.Modes;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.*;

/**
 *
 * @author Kais Haddadin
 *
 */
public class DimensionFilter {

	private HashMap<String,LinkedHashSet<String>> acceptedLists = new HashMap<String,LinkedHashSet<String>>();
	private HashMap<String,HashSet<String>> deniedLists = new HashMap<String,HashSet<String>>();
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
			for(String field: this.dimensionFilterDefinition.getFieldNames()){
			
				LinkedHashSet<String> accepted = new LinkedHashSet<String>();
				accepted.addAll(getElementNames());
				acceptedLists.put(field, accepted);
				LinkedHashSet<String> denied = new LinkedHashSet<String>();
				deniedLists.put(field, denied);
				
				// set the initial status is until here accept all
				// if no filters leave it as accept all and the Filter is an empty filter (not runnable)
				// or if there is no definition (only appear in case Cube Filter)
				if(dimensionFilterDefinition==null || dimensionFilterDefinition.getTreeConditions().size() == 0){
					return;
				}
				// if and only if the first filter is an accept then deny all
				else if(dimensionFilterDefinition.getTreeConditions(field).get(0).getMode().equals(Modes.ACCEPT)){
					denyAll(field);
				}
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

	protected void denyAll(String field) {
		deniedLists.get(field).addAll(acceptedLists.get(field));
		acceptedLists.get(field).clear();
	}

	protected void acceptAll(String field) {
		acceptedLists.get(field).addAll(deniedLists.get(field));
		deniedLists.get(field).clear();
	}
	protected void acceptOnlyBasis(String field) {
		acceptedLists.get(field).clear();
		deniedLists.get(field).clear();
		for(IElement e: getElements()){
			if(e.getChildCount()==0)
				acceptedLists.get(field).add(e.getName());
		}
	}

	protected void acceptRootToConsolidate(String field) {
		acceptedLists.get(field).clear();
		deniedLists.get(field).clear();
		for(IElement e:getElements()){
			if(e.getChildCount()!=0)
				acceptedLists.get(field).add(e.getName());
		}
	}

	public String getName() {
		return dimension.getName();
	}

	public boolean isEmpty(){
		return (dimensionFilterDefinition.getTreeConditions().size() == 0);
	}

	public void acceptRootElements(String field) {
		IElement[] elements = dimension.getRootElements(false);
		for (IElement element : elements) {
			acceptedLists.get(field).add(element.getName());
			deniedLists.get(field).remove(element.getName());
		}
	}


	public String[] process() {
		
		Set<String> fieldNames = this.dimensionFilterDefinition.getFieldNames();
		
		if(fieldNames.size()==0)
			return getElementNames().toArray(new String[0]);
		
		LinkedHashSet<String> finalResult = new LinkedHashSet<String>();
		if(this.dimensionFilterDefinition.getLogicalOperator().equals(LogicalOperators.OR)){
			for(String field:fieldNames){
				finalResult.addAll(acceptedLists.get(field));
			}
		}else{
			if(fieldNames.size()>0){
				Iterator<String> iter = fieldNames.iterator();
				finalResult.addAll(acceptedLists.get(iter.next()));
				while(iter.hasNext()){
					finalResult.retainAll(acceptedLists.get(iter.next()));
				}
			}
		}
		return finalResult.toArray(new String[finalResult.size()]);
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

	private void filterOnlyNodes(String field,Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		if(m.equals(Modes.ACCEPT)){
			for (String s: filteredNodes){
				acceptedLists.get(field).add(s);
				deniedLists.get(field).remove(s);
			}
		}
		else{
			for (String s: filteredNodes){
				acceptedLists.get(field).remove(s);
				deniedLists.get(field).add(s);
			}
		}
	}

	private void filterOnlyRoots(String field,Modes m, String[] filteredNodes){

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
				IElement e = dimension.getElementByName(s, withAttributes);
				if(e.getParentCount() != 0){
					String[] parents = new String[e.getParentCount()];
					IElement[] parentsElements =  e.getParents();
					for(int i=0;i<parents.length;i++)
						parents[i] = parentsElements[i].getName();
					filterOnlyRoots(field,m,parents);
				}
				else{
					if(m.equals(Modes.ACCEPT)){
						acceptedLists.get(field).add(s);
						deniedLists.get(field).remove(s);
					}
					else
					{
						acceptedLists.get(field).remove(s);
						deniedLists.get(field).add(s);
					}
				}
			}
		}
	}

	private void filterNodesToBases(String field,Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
			if(m.equals(Modes.ACCEPT)){
				acceptedLists.get(field).add(s);
				deniedLists.get(field).remove(s);
				}
				else
				{
					acceptedLists.get(field).remove(s);
					deniedLists.get(field).add(s);
				}
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e.getChildCount() != 0){
				String[] children = new String[e.getChildCount()];
				IElement[] childrenElements =  e.getChildren();
				for(int i=0;i<children.length;i++)
					children[i] = childrenElements[i].getName();
				filterNodesToBases(field,m,children);
			}
		}
		}
	}

	private void filterNodesToConsolidates(String field,Modes m, String[] filteredNodes){

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e.getChildCount() != 0){
				if(m.equals(Modes.ACCEPT)){
					acceptedLists.get(field).add(s);
					deniedLists.get(field).remove(s);
					}
					else
					{
						acceptedLists.get(field).remove(s);
						deniedLists.get(field).add(s);
					}
				String[] children = new String[e.getChildCount()];
				IElement[] childrenElements =  e.getChildren();
				for(int i=0;i<children.length;i++)
					children[i] = childrenElements[i].getName();
				filterNodesToConsolidates(field,m,children);
			}
		}
		}
	}

	private void filterRootToNodes(String field, Modes m, String[] filteredNodes){

		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		for (String s: filteredNodes){
			if(!elementVisited.contains(s)){
				elementVisited.add(s);
			if(m.equals(Modes.ACCEPT)){
				acceptedLists.get(field).add(s);
				deniedLists.get(field).remove(s);
				}
				else
				{
					acceptedLists.get(field).remove(s);
					deniedLists.get(field).add(s);
				}
			IElement e = dimension.getElementByName(s, withAttributes);
			if(e != null && e.getParentCount() != 0){
				String[] parents = new String[e.getParentCount()];
				IElement[] parentsElements =  e.getParents();
				for(int i=0;i<parents.length;i++)
					parents[i] = parentsElements[i].getName();
				filterRootToNodes(field,m,parents);
			}
		}
		}
	}

	private void filterRootToBases(String field, Modes m, String[] filteredNodes){
		/*if(filteredNodes.length == elementMap.size()){
			handleSpecialCase(m);
			return;
		}*/

		filterRootToNodes(field, m, filteredNodes);
		elementVisited.clear();
		filterNodesToBases(field,m, filteredNodes);
	}
	private void filterRootToConsolidates(String field,Modes m, String[] filteredNodes){

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

		filterRootToNodes(field,m, processedFilteredNodes.toArray(new String[processedFilteredNodes.size()]));
		elementVisited.clear();
		filterNodesToConsolidates(field,m, processedFilteredNodes.toArray(new String[processedFilteredNodes.size()]));
	}

	/**
	 * In case rootsToNodes, rootToBases,nodesToBases or onlyNodes, this handles the special case when the filteredNodes set is all elements
	 * @param fm
	 * @param m
	 * @param filteredNodes
	 */
	@SuppressWarnings("unused")
	private void handleSpecialCase(String field,Modes m) {
		List<String> names = getElementNames();	
		if(m.equals(Modes.ACCEPT)){
			acceptedLists.get(field).addAll(names);
			deniedLists.get(field).removeAll(names);
		}
		else
		{
			acceptedLists.get(field).removeAll(names);
			deniedLists.get(field).addAll(names);
		}
	}

	/**
	 * In case isOnlyBases or rootToConsildates, this handles the special case when the filteredNodes set is all elements
	 * @param fm
	 * @param m
	 * @param filteredNodes
	 */
	@SuppressWarnings("unused")
	private void handleSpecialCase(String field,FilterModes fm,Modes m,String[]filteredNodes ){
		for (String s: filteredNodes){
			IElement e = dimension.getElementByName(s, withAttributes);;
			if((e.getChildCount() != 0 && fm.equals(FilterModes.onlyBases)) || (e.getChildCount() == 0 && fm.equals(FilterModes.rootToConsolidates))){
				if(m.equals(Modes.ACCEPT)){
					acceptedLists.get(field).remove(s);
					deniedLists.get(field).add(s);
				}
				else
				{
					acceptedLists.get(field).add(s);
					deniedLists.get(field).remove(s);
				}
			}
			else{
				if(m.equals(Modes.ACCEPT)){
					acceptedLists.get(field).add(s);
					deniedLists.get(field).remove(s);
				}
				else
				{
					acceptedLists.get(field).remove(s);
					deniedLists.get(field).add(s);
				}
			}
		}
	}

	private void filterOnlyBases(String field, Modes m, String[]filteredNodes){

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
					filterOnlyBases(field,m,children);
				}
				else{
					if(m.equals(Modes.ACCEPT)){
						acceptedLists.get(field).add(s);
						deniedLists.get(field).remove(s);
					}
					else
					{
						acceptedLists.get(field).remove(s);
						deniedLists.get(field).add(s);
					}
				}
			}
		}
	}
	
	protected Map<String,List<String>> mapAttributeRange(String searchAttribute, HashSet<String> elementSet) {
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
		for(String field: dimensionFilterDefinition.getFieldNames()){
			List<TreeCondition> fieldConditions = dimensionFilterDefinition.getTreeConditions(field);
		for (int i=0;i<fieldConditions.size();i++) {
			LinkedHashSet<String> searchRange =  new LinkedHashSet<String>();
			TreeCondition tc = fieldConditions.get(i);
			if((tc.getMode().equals(Modes.ACCEPT))) 	
				searchRange.addAll(getElementNames());
			else
				searchRange.addAll(acceptedLists.get(field));

			HashSet<String> tempResult = null;
			if (tc.getFieldName().equals(NamingUtil.getElementnameElement())) {
				tempResult = filter(tc,searchRange);
			} else {
				 Map<String,List<String>> nameMap = mapAttributeRange(tc.getFieldName(),searchRange);
				 HashSet<String> attributeResult = filter(tc,nameMap.keySet());
				 tempResult = new HashSet<String>();
				 for (String s : attributeResult) {
					 tempResult.addAll(nameMap.get(s));
				 }
			}
			
			/*if(tc.getMode().equals(Modes.ACCEPT)){
				acceptedLists.get(field).addAll(tempResult);
			}
			
			if(tc.getMode().equals(Modes.DENY)){
				acceptedLists.get(field).removeAll(tempResult);
			}*/

			switch (tc.getFilterMode()) {
			case nodesToBases : {
				filterNodesToBases(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case rootToBases: {
				filterRootToBases(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case rootToNodes: {
				filterRootToNodes(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case onlyBases: {
				filterOnlyBases(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case onlyNodes: {
				filterOnlyNodes(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case onlyRoots: {
				filterOnlyRoots(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			case rootToConsolidates: {
				filterRootToConsolidates(field,tc.getMode(),tempResult.toArray(new String[tempResult.size()]));
				elementVisited.clear();
				break;
			}
			default: break;
			}
			/*for testing
			 * System.out.println( "  My result after " + a.getFilterMode());*/
			/*for(String s:acceptedLists.get(field)){
				System.out.println( s + " - ");
			}*/
		}
		}
	}


}
