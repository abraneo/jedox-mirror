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


import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.palojlib.interfaces.*;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class ElementLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(ElementLoad.class);
	// private ElementType elementType;
	private String dimensionRoot;
	private int addedElements;

	private class Elements {
		public ArrayList<String> names = new ArrayList<String>();
		public ArrayList<ElementType> types = new ArrayList<ElementType>();
	}

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
		com.jedox.palojlib.interfaces.IConnection connection =  (com.jedox.palojlib.interfaces.IConnection)getConnection().open();
		IDatabase d =  connection.getDatabaseByName(getDatabaseName());
		if (d == null) {
			log.info("Creating Palo database "+getDatabaseName());
			d = connection.addDatabase(getDatabaseName()); // create database if necessary
		}
		IDimension dim = getDatabase().getDimensionByName(getDimensionName());
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

	/**
	 * gets the element type: numeric or String
	 * @return
	 */
	private ElementType getElementType(ElementType nodeElementType) {
		//override types with globally set type if set.
		// ElementType t = (elementType == null) ? nodeElementType : elementType;
		switch (nodeElementType) {
		case ELEMENT_STRING: return ElementType.ELEMENT_STRING;
		case ELEMENT_NUMERIC: return ElementType.ELEMENT_NUMERIC;
		default: return ElementType.ELEMENT_NUMERIC;
		}
	}

	private IElement[] addElements(IDimension dim, ITreeManager manager, HashMap<String, IElement> present) throws RuntimeException {
		IElement[] elements = manager.getElementsIgnoreCase(false);
		Elements toAdd = new Elements();
		//remove all names already present as elements
		for (int i=0; i<elements.length; i++) {
			IElement e = elements[i];
			String name = e.getName();
			if (!present.keySet().contains(getLookupName(name))) {
				toAdd.names.add(e.getName());
				toAdd.types.add(e.getType());
			}
		}
		//bulk add
		log.debug("Starting load of elements.");
		ElementType[] types = new ElementType[toAdd.types.size()];
		for (int i=0; i<toAdd.types.size(); i++) {
			types[i] = getElementType(toAdd.types.get(i));
		}
		dim.addElements(toAdd.names.toArray(new String[toAdd.names.size()]), types);
		addedElements += toAdd.names.size();
		if (addedElements>0)
			log.info("New Elements loaded: " + addedElements);
		else
			log.info("No new elements are loaded");
		log.debug("Finished load of elements.");
		return elements;
	}
	
	protected void removeElements(IDimension dim, HashMap<String, IElement> toRemove) {
		int removedElements = toRemove.size();
		dim.removeElements(toRemove.values().toArray(new IElement[toRemove.size()]));
		if (removedElements>0)
			log.info("Elements deleted: " + removedElements);
	}

	protected void exportElements(ITreeManager manager) throws InitializationException, RuntimeException {
		initInternal();
		IDimension dim = getDimension();
		HashMap<String, IElement> present = getElementsInDimension(dim, null);
		switch (getMode()) {
			case CREATE: {
				dim.removeElements(present.values().toArray(new IElement[present.size()]));
				addElements(dim,manager,present);
				break;
			}
			case ADD : {
				addElements(dim,manager,present);
				break;
			}
			case UPDATE: {
				IElement[] added = addElements(dim,manager,present);
				HashMap<String, IElement> toRemove =  (dimensionRoot != null) ?  getElementsInDimension(dim, dimensionRoot) : present;
				if (!toRemove.isEmpty()) {
					//remove all Elements, that where not added.
					for (IElement e : added) {
						toRemove.remove(getLookupName(e.getName()));
					}
					removeElements(dim, toRemove);
				}	
				break;
			}
			case INSERT: {
				addElements(dim,manager, present);
				break;
			}
			case DELETE: {
				IElement[] elements = manager.getElementsIgnoreCase(false);
				HashMap<String, IElement> toRemove = new HashMap<String, IElement>();
				HashMap<String, IElement> toConsider =  (dimensionRoot != null) ?  getElementsInDimension(dim, dimensionRoot) : present;
				for (IElement e : elements) {
					IElement element = toConsider.get(getLookupName(e.getName()));
					if (element != null)
						toRemove.put(element.getName(),element);
				}
				//remove all these Elements
				removeElements(dim, toRemove);
				break;
			}
			default: {
				log.error("Unsupported mode in load "+getName());
			}
		}
	}


	public void execute() {
		if (isExecutable()) {
			log.info("Starting load of Elements: "+getDimensionName());
			try {
				ITreeManager manager = getView().renderTree(Views.EA);
				exportElements(manager);
			}
			catch (Exception e) {
				log.error("Cannot load Elements: "+getDimensionName()+": "+e.getMessage());
				log.debug(e);
			}
			log.info("Finished load of Elements "+getDimensionName()+".");
		}
	}

	public void init() throws InitializationException {
		try {
			super.init();
			addedElements = 0;
			// elementType = getConfigurator().getElementsType();
			//this is for future extensions for restriction on operations on a certain part of the hierarchy
			dimensionRoot = null;
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
