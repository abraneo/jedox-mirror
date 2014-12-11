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
package com.jedox.etl.components.load;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;


public class ConsolidationLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(ConsolidationLoad.class);
	private int consolidateBulkSize;

	private class ConsolidationHandler {

		private HashMap<IElement, ArrayList<IElement>> children = new HashMap<IElement, ArrayList<IElement>>();
		private HashMap<IElement, ArrayList<Double>> weights = new HashMap<IElement, ArrayList<Double>>();
		//private HashMap<String,IElement> lookup = new HashMap<String,IElement>();
		
		//private HashMap<String, IElement[]> oldChildrenMap = new HashMap<String, IElement[]>();
		private HashMap<String, HashMap<String, Double>> oldParentWeightMap = new HashMap<String, HashMap<String, Double>>();

		int newConsolidationCount = 0;
		int removedConsolidationCount = 0;
		int changedWeightCount = 0;
		private IDimension dim;

		public ConsolidationHandler(IDimension dim, boolean readExisting) {
			this.dim = dim;
			dim.setCacheTrustExpiry(300);
			dim.getElements(false);
			/*for (IElement e : elements) {
				lookup.put(e.getName(), e);
			}*/
			if(readExisting){
				//oldChildrenMap = (HashMap<String, IElement[]>) dim.getChildrenMap();
				oldParentWeightMap = (HashMap<String, HashMap<String, Double>>) dim.getWeightsMap();
			}
		}
		
		private void initConsolidationList(IElement parent, ArrayList<IElement> childrenList, ArrayList<Double> weightsList, boolean addExisting) {
			children.put(parent, childrenList);
			weights.put(parent, weightsList);
			if (addExisting) {
				//add already existing consolidations
				//IElement[] oldChildrenList = oldChildrenMap.get(parent.getName());
				IElement[] oldChildrenList = dim.getElementByName(parent.getName(), false).getChildren();
				for (IElement child : oldChildrenList) {
					IElement parentElement = lookupByName(parent.getName());
					childrenList.add(child);
					double weight = oldParentWeightMap.get(child.getName()).get(parentElement.getName());
					weightsList.add(weight);
				}				
			}
		}
						
		public void add(IElement parent, IElement child, double weight, boolean addExisting) {
			ArrayList<IElement> childrenList = children.get(parent);
			ArrayList<Double> weightsList = weights.get(parent);
			if (childrenList == null || weightsList==null)  {
				childrenList = new ArrayList<IElement>();
				weightsList = new ArrayList<Double>();
				initConsolidationList(parent, childrenList, weightsList, addExisting);
			}
			int childIndex = childrenList.indexOf(child);
			if (childIndex==-1) {			
				 //add consolidation				
				childrenList.add(child);
				weightsList.add((Double)weight);
				newConsolidationCount++;
			}
			else if (weightsList.get(childIndex)!=weight) {
				// change weight of consolidation
				weightsList.set(childIndex, weight);
				changedWeightCount++;
		}
		}
		
		public void remove (IElement parent, IElement child) {
			ArrayList<IElement> childrenList = children.get(parent);
			ArrayList<Double> weightsList = weights.get(parent);
			if (childrenList == null || weightsList==null) { 
				childrenList = new ArrayList<IElement>();
				weightsList = new ArrayList<Double>();
				initConsolidationList(parent, childrenList, weightsList, true);
			}
			//delete consolidation
			int index = childrenList.indexOf(child); 
			if (index >= 0) {
				childrenList.remove(index);
				weightsList.remove(index);
				removedConsolidationCount++;
			}
		}
		
		public Set<IElement> getParentSet() {
			return children.keySet();
		}
		
		public ArrayList<IConsolidation> getConsolidations(IElement parent) {
			ArrayList<IElement> childrenList = children.get(parent);
			ArrayList<IConsolidation> consolidations = new ArrayList<IConsolidation>();				
			for (int i=0; i<childrenList.size(); i++) {
				IConsolidation cons = dim.newConsolidation(lookupByName(parent.getName()),childrenList.get(i),weights.get(parent).get(i));
				consolidations.add(cons);					
			}
			return consolidations;
			
		}
			
		public int getConsolidationsCount () {
			int count = 0;
			for (IElement parent : children.keySet()) {
				count+=children.get(parent).size();
			}	
			return count;
		}

		public int getNewConsolidationsCount () {
			return newConsolidationCount;
		}

		public int getRemovedConsolidationsCount () {
			return removedConsolidationCount;
		}

		public int getChangedWeightCount () {
			return changedWeightCount;
		}
		
		public IElement lookupByName(String name) {
			//return lookup.get(name);
			return dim.getElementByName(name, false);
		}
		
	}

	public ConsolidationLoad() {
		setConfigurator(new DimensionConfigurator());
	}

	public ConsolidationLoad(IConfigurator configurator) throws InitializationException {
		setConfigurator(configurator);
		init();
	}

	private void buildConsolidations(ConsolidationHandler conHandler, IConsolidation[] consolidations) {

		String parentName = "";
		String childName = "";		
		for (IConsolidation c : consolidations) {
			try {
				double weight = c.getWeight();
				parentName = c.getParent().getName();
				childName = c.getChild().getName();
				IElement parent = conHandler.lookupByName(parentName);
				IElement child = conHandler.lookupByName(childName);
				if ((parent != null) && (child != null)) {
					switch (getMode()) {
					case CREATE: 
					case UPDATE : {
						conHandler.add(parent, child, weight, false);
						break;
					}
					case INSERT:
					case ADD: {
						conHandler.add(parent, child, weight, true);
						break;
					}
					case DELETE: {
						conHandler.remove(parent, child);
						break;
					}
					}			
				}
			}
			catch (Exception e) {
				log.warn("Failed to consolidate "+parentName+" with "+childName+ "in load "+getName()+": "+e.getMessage());
			}
		}
	}

	private boolean hasSourceUniqueParents (IConsolidation[] consolidations) { 
		HashMap<IElement, IElement> map = new HashMap<IElement, IElement>();
		for (IConsolidation c: consolidations) {
			if (map.containsKey(c.getChild())) {
				if (!c.getParent().equals(map.get(c.getChild()))) {
					log.warn("Consolidation Mode INSERT cannot be applied on source "+getName()+" because element "+c.getChild().getName()+" has duplicate parents "+c.getParent().getName()+" and "+map.get(c.getChild())+". Mode ADD is used instead."); 
					return false;					
				}
			}
			else {
				map.put(c.getChild(), c.getParent());
			}
		}
		return true;
	
	}
	
	private void removeDuplicateParents (ConsolidationHandler conHandler, IConsolidation[] consolidations) {
		// in mode insert, if the elements (C,N) have a new parent in the new tree, the consolidation from old parents should be deleted.
		log.debug("Remove duplicate consolidations for Insert Mode");
		for (IConsolidation c : consolidations){ //go over the children from the new source
			IElement e = conHandler.lookupByName(c.getChild().getName());
			if (e != null) { 
				// Element exists in old tree and is not a root in the new source
				for (IElement p : e.getParents()) { //go over the already existing parents
					if (!p.getName().equals(c.getParent().getName())) {										
						// Remove Consolidations for different parent in Handler
						conHandler.remove(p, e);						
					}
				}
			}
		}
	}

	private void deleteAllConsolidations(IDimension dim, ConsolidationHandler conHandler) {
		IElement[] dimElements = dim.getElements(false);
		ArrayList<IElement> elementsLst = new ArrayList<IElement>(dimElements.length);
		for (IElement e : dim.getElements(false))
			elementsLst.add(e);
		// this should not be the same value as the loadbulksize
		int consolidateDeleteBulkSize = 500000;
		//then delete them in Bulk mode
		int parentsNum = elementsLst.size();
		if (parentsNum!=0) {
			int changedToNonParent = 0;
			log.info("Removing all consolidations for dimension update");
			log.debug("Removing Consolidations for " + parentsNum + " Elements");
			while(changedToNonParent!= parentsNum ){
				ArrayList<IElement> subList = new ArrayList<IElement>();
				int nextBulkEnd = changedToNonParent+consolidateDeleteBulkSize;
				if(parentsNum<=nextBulkEnd)
					nextBulkEnd = parentsNum;
				subList=new ArrayList<IElement>( elementsLst.subList(changedToNonParent, nextBulkEnd));
				dim.removeConsolidations(subList.toArray(new IElement[subList.size()]));
				changedToNonParent+=subList.size();
				log.debug("Number of parents for which consolidations are removed: " + nextBulkEnd);
			}
		}	
	}
	
	private void consolidateElementsSingle(IDimension dim, ConsolidationHandler conHandler) {
		int addedConsolidations = 0;
		log.debug("Write consolidations in Single Mode.");
		//now add consolidations to elements
		for (IElement parent : conHandler.getParentSet()) {
			ArrayList<IConsolidation> keyConsolidations = conHandler.getConsolidations(parent); 
//			IElement element = conHandler.elementLookup.get(parent.getName());		
			if (keyConsolidations.isEmpty()) {
				// Only in DELETE Mode when all consolidations of an element have been deleted
				IElement[] elements = new IElement[1];
				elements[0] = parent;
				dim.removeConsolidations(elements);
			}
			else {
				//log.debug("Consolidation element: "+element.getName());
				dim.updateConsolidations(keyConsolidations.toArray(new IConsolidation[keyConsolidations.size()]));
				addedConsolidations += keyConsolidations.size();
			}	
		}
		log.debug("Number of consolidations that are updated: "+addedConsolidations);
	}
	
	//Bulk modus available with the new Palo version in 14.01.2010
	private void consolidateElementsBulk(IDimension dim, ConsolidationHandler conHandler) {
		log.debug("Write consolidations in Bulk mode.");
		ArrayList<IConsolidation> allConsolidations = new ArrayList<IConsolidation>();
		ArrayList<IElement> baseElements = new ArrayList<IElement>();
		ArrayList<Integer> bulkSizes = new ArrayList<Integer>();
		bulkSizes.add(0); //
		int intervalCons = 0;
		//now add consolidations to elements
		for (IElement parent : conHandler.getParentSet()) {
			ArrayList<IConsolidation> keyConsolidations = conHandler.getConsolidations(parent); 
//			IElement element = conHandler.elementLookup.get(parent.getName());		
//			if (element == null) {
//				conHandler.getConsolidations().remove(key);
//			}
			if (keyConsolidations.isEmpty()) {
				// Only in DELETE Mode when all consolidations of an element have been deleted
				baseElements.add(parent);
			}
			else {
				allConsolidations.addAll(keyConsolidations);
				intervalCons += keyConsolidations.size();
				int nextPoint = bulkSizes.get(bulkSizes.size()-1)+intervalCons;
				if( intervalCons>=consolidateBulkSize){
					bulkSizes.add(nextPoint);
					intervalCons = 0;
				}
			}
		}
		if(bulkSizes.get(bulkSizes.size()-1) != allConsolidations.size()){
			bulkSizes.add(allConsolidations.size());
		}

		if (!baseElements.isEmpty()) {
			log.debug("Removing consolidations for "+baseElements.size()+" elements");
			dim.removeConsolidations(baseElements.toArray(new IElement[baseElements.size()]));
		}
		
		
		if (conHandler.getConsolidationsCount() != allConsolidations.size()){
			log.error("Error in consolidating.");// should never happens, when it happens this will be a program error
		}

		if (conHandler.getConsolidationsCount()!=0){
			int consolidationDone = 0;
			ArrayList<IConsolidation> subList = new ArrayList<IConsolidation>();
			for(int i=0; i< bulkSizes.size()-1;i++){
				subList = new ArrayList<IConsolidation>(allConsolidations.subList(bulkSizes.get(i), bulkSizes.get(i+1)));
				dim.updateConsolidations(subList.toArray(new IConsolidation[subList.size()]));
				consolidationDone+=subList.size();
				log.debug("Number of consolidations that are updated: " + consolidationDone);
			}
		}		
	}

	protected void exportConsolidations(ITreeManager manager) throws RuntimeException {
		IDimension dim=null;
		try {
			// Collect consolidations from the source in internal class.
			log.debug("Retrieving consolidations...");
									
			// Read all elements from the dimension and store them in Consolidation Handler
			dim = getDimension();
			boolean needExisting = false;
			if((getMode().equals(Modes.INSERT) || getMode().equals(Modes.ADD) || getMode().equals(Modes.DELETE))){
				needExisting = true;
			}
			
			ConsolidationHandler conHandler = new ConsolidationHandler(dim,needExisting);
			
			if ((getMode().equals(Modes.UPDATE) || getMode().equals(Modes.CREATE))) {
				// Remove all existing consolidations				
				deleteAllConsolidations(dim, conHandler);	
				dim.setCacheTrustExpiry(3000);
			}
			
			IConsolidation[] consolidations = manager.getConsolidations();
			log.debug("Building con handler");
			// Build up new and existing consolidations in Handler 			
			buildConsolidations(conHandler, consolidations);
			log.info("Loading consolidations");
			
			if (getMode().equals(Modes.INSERT)) {
				if (hasSourceUniqueParents(consolidations)) {
					removeDuplicateParents(conHandler, consolidations);
					dim.setCacheTrustExpiry(3000);
				}	
			}
			
			// Load Consolidations to Palo
			if (getConnection().isOlderVersion(3, 0, 555) || consolidateBulkSize<=1)
				consolidateElementsSingle(dim, conHandler);
   		 	else
				consolidateElementsBulk(dim, conHandler);
			
			if (conHandler.getRemovedConsolidationsCount()!=0)
				log.info("Consolidations deleted: "+conHandler.getRemovedConsolidationsCount());
			if (conHandler.getNewConsolidationsCount()==0)
				log.info("No new consolidations are loaded");
			else
				log.info("New consolidations loaded: " + conHandler.getNewConsolidationsCount());
			if (conHandler.getChangedWeightCount()!=0)
				log.info("Weights changed: "+conHandler.getChangedWeightCount());
		}
		catch (Exception e) {
			log.error("Failed to consolidate elements in load "+getName()+": "+e.getMessage());
		}finally{
			if(dim!=null)
				dim.setCacheTrustExpiry(0);
		}
	}

	public void execute() {
		log.info("Starting load of Consolidations: "+getName());
		try {
			if (isExecutable() && getDimension() != null) {
				exportConsolidations(getView().renderTree(Views.PCW));
			}
		}
		catch (Exception e) {
			log.error("Cannot load "+getName()+": "+e.getMessage());
			log.debug(e);
		}
		log.info("Finished load of Consolidations "+getName()+".");
	}

	public void init() throws InitializationException {
		try {
			consolidateBulkSize = getConfigurator().getConsolidateBulksSize();
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
		super.init();
	}

}
