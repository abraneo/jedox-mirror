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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.tree.ITreeExporter;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.palojlib.interfaces.*;
import com.jedox.palojlib.interfaces.ICube.CubeType;

public class ElementLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(ElementLoad.class);
	private int existingElementsNum;
	private ArrayList<Integer> toMove = new ArrayList<Integer>();
	private int elementBulkSize;

	private IDimension addDimension(IDatabase d) {
		IDimension dim = null;
		try {
			log.info("Creating dimension "+getDimensionName());
			dim = d.addDimension(getDimensionName());
		}
		catch (Exception e) {
			log.warn("Failed to add dimension "+getDimensionName()+": "+e.getMessage());
		}
		return dim;
	}

	private IDimension recreateDimension(IDimension dim) throws RuntimeException {
		IDatabase d = getDatabase();
		if (dim != null) {
			try {
				ArrayList<ICube> cubes2delete = new ArrayList<ICube>();
				ICube[] cubes = d.getCubes();
				for (ICube c : cubes) {
					if ((c!=null) && (c.getDimensionByName(getDimensionName()) != null) && (!c.getType().equals(CubeType.CUBE_ATTRIBUTE) &&  (!c.getType().equals(CubeType.CUBE_SYSTEM) &&  (!c.getType().equals(CubeType.CUBE_USERINFO))))) {
						cubes2delete.add(c);
					}
				}
				for (ICube c: cubes2delete) {
					try {
						log.info("Removing cube "+c.getName());
						d.removeCube(c);
					}
					catch (Exception e) {
						log.warn("Failed to remove cube "+c.getName()+": ",e);
					};
				}
//				d.save();
//				d.reload();
				log.info("Deleting dimension "+dim.getName());
				d.removeDimension(getDimension());
			}
			catch (Exception e) {
				log.warn("Failed to delete dimension: ",e);
			}
		}
		return addDimension(d);
	}

	/**
	 * internal Initialization
	 *
	 */
	private IDimension initInternal() throws InitializationException, RuntimeException {
//		do databasechecks.
		IDatabase d;
		try {
			d = getDatabase();
		} catch (RuntimeException e) {	
			log.info("Creating Jedox database "+getDatabaseName());
			d = getConnection().getDatabase(true,true); // create database if necessary
		}
		IDimension dim = d.getDimensionByName(getDimensionName());;
		switch (getMode()) {
			case CREATE: {
				//recreate dimension
				dim = recreateDimension(dim);
				break;
			}
			case ADD : {
				//create dimension if not present
				if (dim == null)
					dim = addDimension(d);
				break;
			}
			case UPDATE: {
				//create dimension if not present
				if (dim == null)
					dim = addDimension(d);
				break;
			}
			case INSERT: {
				if (dim == null)
					dim = addDimension(d);
				break;
			}
			case DELETE: {
				//create dimension if not present
				if (dim == null)
					dim = addDimension(d);
				break;
			}
			default: {
				log.error("Unsupported mode in load "+getName());
			}
		}
		if (dim == null) {
			throw new InitializationException("Dimension "+getDimensionName()+" does not exist in database "+getDatabaseName());
		}
		return dim;
	}


	public ElementLoad() {
		setConfigurator(new DimensionConfigurator());
	}

	public ElementLoad(IConfigurator configurator) throws InitializationException {
		setConfigurator(configurator);
		init();
	}

	private HashSet<IElement> getElementsForDeletion(IDimension dim, ITreeExporter exporter) throws RuntimeException {
		HashSet<IElement> toRemove = new HashSet<IElement>();		
		IElement[] elements=null;
		while (exporter.hasNext()){
			elements = exporter.getNextBulk();		
			for (IElement e : elements) {
				IElement dimElement = dim.getElementByName(e.getName(), false);
				if (dimElement != null)
					toRemove.add(dimElement);
			}			
		}	
		return toRemove;		
	}
	
	private void addElements(IDimension dim, ITreeExporter exporter, boolean isInsertMode, HashSet<IElement> toRemove) throws RuntimeException {		
		IElement[] elements=null;
		existingElementsNum = dim.getElements(false).length;

		while (exporter.hasNext()){
			dim.setCacheTrustExpiry(36000); //ensure that in update mode cache is not renewed, since this breaks toRemove IElement equality.
			elements = exporter.getNextBulk();
			IElement[] newElements;
			if(existingElementsNum!=0){
				ArrayList<IElement> toAdd = new ArrayList<IElement>();
				toMove.clear();
				for (int i=0; i<elements.length; i++) {
					IElement e = elements[i];
					IElement dimElement=dim.getElementByName(e.getName(), false);
					if (dimElement==null) {
						toAdd.add(e);
						if(isInsertMode)
							toMove.add(i);
					}
					else {
						if (toRemove!=null)
							toRemove.remove(dimElement);						
					}
				}
				newElements = toAdd.toArray(new IElement[toAdd.size()]);
			} 
			else {
				// no need to move or to build a new toAdd list
				newElements = elements;				
			}	
			//bulk add
			log.debug("Loading "+newElements+" new elements");				
			dim.appendElements(newElements);							
			
			// for moving 
			if (existingElementsNum!=0 && isInsertMode){ //only applies for insert mode. no toRemove set needed here. will renew cRache			
				ArrayList<IElement> serverAdded = new ArrayList<IElement>();
				boolean duplicatesExist=false;
				for(int i = 0;i<newElements.length;i++) {
					IElement element = dim.getElementByName(newElements[i].getName(), false);
					if (element==null) {
						// Handling of elements with the same name as existing elements on ICU secondary comparison level (e.g. A2 and A²)are in newElements array but are not created on sever differ in ICU
						// They are in newElements array but are not created on the Server. They have to be removed from the toMove array, too
						log.warn("Element with name "+newElements[i].getName()+" is duplicat on ICU secondary comparison level.");
						duplicatesExist = true;
						break;
						
					}	
					else {
						serverAdded.add(element);
					}		
				}	
				
				if(duplicatesExist)
					log.warn("Duplicat elements on ICU secondary comparison level exist, therefor no sorting could be applied.");
				else if(serverAdded.size()>0)
					dim.moveElements(serverAdded.toArray(new IElement[0]), toMove.toArray(new Integer[0]));
			}
		}			
		
		int addedElements = dim.getDimensionInfo().getNumberOfElements() - existingElementsNum;
		if (addedElements>0)
			log.info("New elements loaded: " + addedElements);
		else
			log.info("No new elements are loaded");
		log.debug("Finished load of elements.");
	}
	
	protected void removeElements(IDimension dim, HashSet<IElement> toRemove) {
		int removedElements = toRemove.size();
		if (removedElements>0) {
			dim.removeElements(toRemove.toArray(new IElement[toRemove.size()]));		
			log.info("Elements deleted: " + removedElements);
		}	
	}

	protected void exportElements(ITreeExporter exporter) throws InitializationException, RuntimeException {
		initInternal();
		IDimension dim = getDimension();
		exporter.setBulkSize(elementBulkSize);
		exporter.setWithAttributes(false);
		
		switch (getMode()) {
			case CREATE: {
				addElements(dim, exporter, false, null);
				break;
			}
			case ADD : {
				addElements(dim, exporter, false, null);
				break;
			}
			case UPDATE: {
				HashSet<IElement> toRemove = new HashSet<IElement>();
				toRemove.addAll(Arrays.asList(dim.getElements(false)));
				addElements(dim, exporter, false, toRemove);
				removeElements(dim, toRemove);
				break;
			}
			case INSERT: {
				addElements(dim, exporter, true, null);
				break;
			}
			case DELETE: {
				HashSet<IElement> toRemove = getElementsForDeletion(dim,exporter);
				removeElements(dim, toRemove);
				break;
			}
			default: {
				log.error("Unsupported mode in load "+getName());
			}
		}
	}

	// currently not use
	// For non-treebased sources a tree is created with TreeManagerNG (not optimised with FlatTreeExporter)
	public void executeLoad() {
		log.info("Starting load of Elements: "+getDimensionName());
		try {
			ITreeManager manager = getView().renderTree(Views.EA).getManager();
			exportElements(manager.getExporter());
		}
		catch (Exception e) {
			log.error("Cannot load Elements: "+getDimensionName()+": "+e.getMessage());
			log.debug(e);
		}
		log.info("Finished load of Elements "+getDimensionName()+".");
	}

	public void init() throws InitializationException {
		try {
			super.init();
			existingElementsNum = 0;
			try {
				elementBulkSize = getConfigurator().getElementBulkSize();
			} catch (ConfigurationException e) {
				throw new InitializationException(e.getMessage());
			}
			
			// elementType = getConfigurator().getElementsType();
			//this is for future extensions for restriction on operations on a certain part of the hierarchy
			//dimensionRoot = null;
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
