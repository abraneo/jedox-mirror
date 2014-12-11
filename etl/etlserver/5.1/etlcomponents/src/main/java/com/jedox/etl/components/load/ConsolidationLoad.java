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
package com.jedox.etl.components.load;

import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.tree.Consolidation;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.tree.UniqueConsolidation;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;


public class ConsolidationLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(ConsolidationLoad.class);
	//private MessageHandler aggLog = new MessageHandler(log);
	private ConsolidationMode mode;
	
	private static enum ConsolidationMode {
		UNIQUE, UNIQUEROOT, MULTIPLE
	}

	private class ConsolidationHandler {
		
		private Set<IConsolidation> consolidations = new LinkedHashSet<IConsolidation>();
		private TreeManagerNG uniqueRootTree;
		private IDimension dim;
		private int existingConsolidations = 0;
		private int sourceConsoldiations = 0;

		public ConsolidationHandler(IDimension dim, boolean readExisting) {
			this.dim = dim;
			//long time = System.currentTimeMillis();
			IElement[] elements = dim.getElements(false);
			dim.setCacheTrustExpiry(36000);
			
			//System.err.println("Handler : After read elements: "+(System.currentTimeMillis()-time));			
			if(readExisting){
				switch(mode) {
				case UNIQUEROOT: {
					uniqueRootTree = new TreeManagerNG(dim,false);
					uniqueRootTree.setAutoCommit(false);
					existingConsolidations = uniqueRootTree.getConsolidations().length;
					break;
				}
				case UNIQUE: {
					for (IElement e : elements) {
						for (IElement c : e.getChildren()) {
							consolidations.add(new UniqueConsolidation(e,c,c.getWeight(e)));
							existingConsolidations++;
						}
					}
					break;
				}
				case MULTIPLE: {
					for (IElement e : elements) {
						for (IElement c : e.getChildren()) {
							consolidations.add(new Consolidation(e,c,c.getWeight(e)));
							existingConsolidations++;
						}
					}
					break;
				}
				}
			} 
			//System.err.println("Handler : After consolidations: "+(System.currentTimeMillis()-time));
		}
		
		public int getExistingConsolidationCount() {
			return existingConsolidations;
		}
		
		public int getSourceConsolidationCount() {
			return sourceConsoldiations;
		}
						
		public void add(IElement parent, IElement child, double weight) {
			switch (mode) {
			case MULTIPLE : {
				Consolidation c= new Consolidation(parent, child, weight);
				consolidations.remove(c); //ensures that weight of last consolidation is taken.
				consolidations.add(c);
				break;
			}
			case UNIQUE: {
				Consolidation c= new UniqueConsolidation(parent, child, weight);
				consolidations.remove(c); //ensure that only newest consolidation stays.
				consolidations.add(c);
				break;
			}
			case UNIQUEROOT: {
				uniqueRootTree.addConsolidation(parent, child, weight);
			}
			}
			sourceConsoldiations++;
		}
		
		public void remove (IElement parent, IElement child) {
			Consolidation c= new Consolidation(parent, child, 0);
			consolidations.remove(c);
			// in case the parent does not have children anymore
			Consolidation emptyc= new Consolidation(parent, null, 0);
			consolidations.add(emptyc);
			sourceConsoldiations++;
		}
		
		private boolean isUniquePerRootCons(Map<IElement,Set<IConsolidation>> rootConSets, IElement child) {
			IElement[] exclusive = child.getParents();
			if (exclusive != null && exclusive.length > 1) {
				for (IElement root : rootConSets.keySet()) {
					Set<IConsolidation> c = rootConSets.get(root);
					int count = 0;
					for (IElement e : exclusive) {
						if (e.equals(root) || c.contains(new UniqueConsolidation(null,e,0))) count++;
					}
					if (count > 1) {
						return false;
					}
				}
			}
			return true;
		}
		
		private void setConsolidationsForUniqueRoot() {
			uniqueRootTree.commitConsolidations();
			Map<IElement,IConsolidation> childMap = new HashMap<IElement,IConsolidation>();
			Map<IElement,Set<IConsolidation>> rootConSets = new HashMap<IElement,Set<IConsolidation>>();
			Map<IElement,Set<IConsolidation>> testConSets = new LinkedHashMap<IElement,Set<IConsolidation>>();
			IElement[] rootElements = uniqueRootTree.getRootElements(false);
			for (IElement e : rootElements) { //calculate ordered unique per root sets containing last added parent
				Set<IConsolidation> uniqueConSet = uniqueRootTree.getConsolidationSet(e, true);
				rootConSets.put(e, uniqueConSet);
			}
			uniqueRootTree.clearConsolidations();
			for (IElement e : rootElements) { //first pass - apply unique per root sets to tree - result may still have illegal nodes due to different consolidations in subtrees
				Set<IConsolidation> rootConSet = rootConSets.get(e);
				for (IConsolidation c : rootConSet) {
					uniqueRootTree.addConsolidation(c.getParent(),c.getChild(),c.getWeight());
				}
			}
			uniqueRootTree.commitConsolidations();
			//now we have stripped subtrees of multiple consolidations. so isUniquePerRootCons will also work if migrating a sub-subtree for one node to an other within the root subtree 
			for (IElement e : rootElements) { //filter out illegal nodes
				Set<IConsolidation> rootConSet = rootConSets.get(e);
				testConSets.put(e, rootConSet);
				for (IConsolidation c : rootConSet) {
					IConsolidation cCon = childMap.get(c.getChild());
					//3 conditions to consider for finally adding consolidation to dim:
					//1 no existing unique consolidation (over all subtrees)
					//2 existing unique consolidation, but current consolidation has equal parent and thus is equal
					//3 multiple consolidation but only a single parent is contained in a single subtree
					if (cCon == null || 
						cCon.getParent().equals(c.getParent()) || 
						isUniquePerRootCons(testConSets, c.getChild())) {
						IConsolidation subTreeCon = new Consolidation(lookupByName(c.getParent().getName()),lookupByName(c.getChild().getName()),c.getWeight());
						consolidations.add(subTreeCon);
						if (cCon == null) childMap.put(c.getChild(), c);
					}
				}
			}
			/* TEST CODE FOR PRINT RESULT
			uniqueRootTree.clearConsolidations();
			uniqueRootTree.addConsolidations(consolidations.toArray(new IConsolidation[consolidations.size()]));
			uniqueRootTree.commitConsolidations();
			try {
				ITreeProcessor p = initTreeProcessor(new TreeManagerProcessor(uniqueRootTree),Facets.HIDDEN);
				IProcessor processor = initProcessor(new TreeViewProcessor(p,Views.FH),Facets.HIDDEN);
				CSVWriter writer = new CSVWriter(System.err);
				writer.write(processor);
				System.out.flush();
				writer.close();
			}
			catch (Exception e){}
			*/
			uniqueRootTree.clear();
			uniqueRootTree = null;			
		}
		
		
		public Set<IConsolidation> getConsolidations() {
			if (mode.equals(ConsolidationMode.UNIQUEROOT) && uniqueRootTree != null) {
				setConsolidationsForUniqueRoot();
			}
			return consolidations;
		}
				
		public IElement lookupByName(String name) {
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

	private void buildConsolidations(ConsolidationHandler conHandler, Set<IConsolidation> sourceConsolidations) {
		log.debug("Building consolidations handler");
		String parentName = "";
		String childName = "";		
		int i = 0;
		for (IConsolidation c : sourceConsolidations) {
			i++;
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
						conHandler.add(parent, child, weight);
						break;
					}
					case INSERT:
					case INSERTPARALLEL:
					case ADD: {
						conHandler.add(parent, child, weight);
						break;
					}
					case DELETE: {
						conHandler.remove(parent, child);
						break;
					}
					default:
					}			
				}
			}
			catch (Exception e) {
				log.warn("Failed to consolidate "+parentName+" with "+childName+ "in load "+getName()+": "+e.getMessage());
			}
			if (i % 10000 == 0) log.debug("Internally consolidated: "+i);
		}
	}

	/*private void deleteAllConsolidations(IDimension dim) {
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
				dim.setCacheTrustExpiry(36000); //cschw renew trust time since this operation implicitly ends it.
				changedToNonParent+=subList.size();
				log.debug("Number of parents for which consolidations are removed: " + nextBulkEnd);
			}
		}	
	}*/
	
	//Bulk modus available with the new Palo version in 14.01.2010
	private int consolidateElementsBulk(IDimension dim, ConsolidationHandler conHandler) {
		log.debug("Writing consolidations in bulk");
		Collection<IConsolidation> consolidations = conHandler.getConsolidations();
		if (consolidations.size() > 0) {
			dim.updateConsolidations(consolidations.toArray(new IConsolidation[consolidations.size()]));
		}
		return consolidations.size();
	}

	protected void exportConsolidations(ITreeManager manager) throws RuntimeException {
		IDimension dim=null;
		try {
			log.debug("Retrieving consolidations...");									
			dim = getDimension();
			boolean needExisting = (getMode().equals(Modes.INSERT) || getMode().equals(Modes.ADD) || getMode().equals(Modes.DELETE) || getMode().equals(Modes.INSERTPARALLEL));
			mode = ConsolidationMode.MULTIPLE; 
			switch (getMode()) {
			case INSERT: mode = ConsolidationMode.UNIQUE; break;
			case INSERTPARALLEL: mode = ConsolidationMode.UNIQUEROOT; break;
			default: break;
			}			
			// Read all elements from the dimension and store them in Consolidation Handler
			ConsolidationHandler conHandler = new ConsolidationHandler(dim,needExisting);			
	
			if (!getMode().equals(Modes.ADD) && dim.hasConsolidatedElements()) {
				// Remove all existing consolidations				
				int oldParents = dim.removeAllConsolidations();	
				dim.setCacheTrustExpiry(36000);
				if(getMode().equals(Modes.UPDATE))
					log.info("Removing all consolidations for dimension update");
				log.debug("Number of parents whose consolidations are removed: " + oldParents);
			}
			
			Set<IConsolidation> sourceConsolidations = manager.getConsolidationSet(null, getMode().equals(Modes.INSERT));
		
			// Build up new and existing consolidations in Handler 			
			buildConsolidations(conHandler, sourceConsolidations);	
			
			int finalConsolidations = consolidateElementsBulk(dim, conHandler);			

			if (needExisting) {
				log.info("Consolidations before load: "+conHandler.getExistingConsolidationCount());
			}
			log.info("Consolidations from source: "+conHandler.getSourceConsolidationCount());
			log.info("Consolidations after load: "+finalConsolidations);
		}
		catch (Exception e) {
			log.error("Failed to consolidate elements in load "+getName()+": "+e.getMessage());
		}finally{
			if(dim!=null)
				dim.setCacheTrustExpiry(0);
		}
	}

	public void executeLoad() {
		log.info("Starting load of Consolidations: "+getName());
		try {
			if (getDimension() != null) {
				exportConsolidations(getView().renderTree(Views.PCW).getManager());
			}
		}
		catch (Exception e) {
			log.error("Cannot load "+getName()+": "+e.getMessage());
			log.debug(e);
		}
		log.info("Finished load of Consolidations "+getName()+".");
	}

	public void init() throws InitializationException {
/*		
		try {
			consolidateBulkSize = getConfigurator().getConsolidateBulksSize();
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
*/		
		super.init();
	}

}
