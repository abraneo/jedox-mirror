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
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.palojlib.interfaces.*;
import com.jedox.palojlib.interfaces.IElement.ElementType;


public class AttributeLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(AttributeLoad.class);
	private int addedAttributes = 0;
	private int attributesBulkSize;
	private TypeConversionUtil typeConversion = new TypeConversionUtil();


	public AttributeLoad() {
		setConfigurator(new DimensionConfigurator());
	}

	public AttributeLoad(IConfigurator configurator) throws InitializationException {
		setConfigurator(configurator);
		init();
	}

	private void addAttributeValues(IDimension dim, ITreeManager manager, boolean skipEmpty) throws RuntimeException {
		HashMap<String,IElement> lookup = getElementsInDimension(dim,null);
		
		IElement[] treeElements = manager.getElements(true);
		//iterate over all elements and attributes
		for (IAttribute a : manager.getAttributes()) {
			String attributeName = a.getName();
			ArrayList<String> attributeValues = new ArrayList<String>();
			IAttribute attribute = dim.getAttributeByName(attributeName);
			ArrayList<IElement> elementsBulk = new ArrayList<IElement>();
			if (attribute != null) {
				//get the non null values of this attribute for each element present in the dimension
				for (int j=0; j < treeElements.length; j++) {
					IElement e = treeElements[j];
					String elementName = e.getName();
					Object attributeValueObject = e.getAttributeValue(attributeName);
					if (attributeValueObject != null || !skipEmpty ) {
						String attributeValue = (attributeValueObject != null) ? attributeValueObject.toString() : "";
						if (attribute.getType() == ElementType.ELEMENT_NUMERIC) {
							attributeValue = typeConversion.convertToNumeric(attributeValue);
							if (!typeConversion.isNumeric(attributeValue)) {
								log.warn("Element "+elementName+" has non-numerical attribute value "+attributeValue+" for numerical attribute "+attribute.getName());
							}
						}
						IElement element = lookup.get(getLookupName(e.getName()));
						if (element != null) {
							//do not assign numeric attribute values to consolidated elements because splashing is disabled for now!
							if (!(element.getType() == ElementType.ELEMENT_CONSOLIDATED && attribute.getType() == ElementType.ELEMENT_NUMERIC)) {
								elementsBulk.add(element);
								attributeValues.add(attributeValue);
								if(j==treeElements.length-1 || attributeValues.size() >= attributesBulkSize){// if it is the last element in the dimension or it exceeded the bulk size
									exportSingleAttribute(dim,elementsBulk, attributeValues, attribute);
								}
							}
							else {
								if (!attributeValue.isEmpty() && !attributeValue.equals("0") ) {
									log.debug("Element "+element.getName()+" is consolidated and thus does not accept numerical attribute "+attribute.getName());
								}
							}
						}
					}
				}
			}

			if(elementsBulk.size() != 0){
				// if the last element is null, the list will still contain some elements for export
				exportSingleAttribute(dim,elementsBulk, attributeValues, attribute);
			}

			if (addedAttributes == 0)
				log.info("For attribute " + attributeName + ", no new attributes are loaded");
			addedAttributes = 0;
			//export this attribute
			//attribute.setValues(elements.toArray(new Element[elements.size()]), attributeValues.toArray(new String[attributeValues.size()]));
			//addedAttributes += attributeValues.size();
		}
	}

	private void exportSingleAttribute(IDimension dim,ArrayList<IElement> elements,ArrayList<String> attributeValues,IAttribute attribute) {
		//export this attribute
		dim.addAttributeValues(attribute, elements.toArray(new IElement[elements.size()]), attributeValues.toArray(new String[attributeValues.size()]));
		addedAttributes += attributeValues.size();
		log.info("For attribute " + attribute.getName() + ", values loaded: "+addedAttributes);
		elements.clear();
		attributeValues.clear();
	}

	private IAttribute getAlias(IDimension dim) {
		IAttribute alias = dim.getAttributeByName("Alias");
		if (alias == null) {
			dim.addAttributes(new String[]{"Alias"},new ElementType[]{ElementType.ELEMENT_STRING});
			alias = dim.getAttributeByName("Alias");
		}
		return alias;
	}

	// add attributes from source processor to dimension
	private void addAttributes(IDimension dim, ITreeManager manager) throws RuntimeException {
		Attribute[] attributes = manager.getAttributes();
		ArrayList<IAttribute> aliases = new ArrayList<IAttribute>();
		for (Attribute a : attributes) {
				if (dim.getAttributeByName(a.getName()) == null) {
					switch (a.getType()) {
					case ELEMENT_NUMERIC: dim.addAttributes(new String[]{a.getName()}, new ElementType[]{ElementType.ELEMENT_NUMERIC}); break;
					default: dim.addAttributes(new String[]{a.getName()}, new ElementType[]{ElementType.ELEMENT_STRING});
					}
					if (a.getMode().equals(AttributeModes.ALIAS)) {
						aliases.add(dim.getAttributeByName(a.getName()));
					}
				}
		}
		if(aliases.size()>0){
			IAttribute alias = getAlias(dim);
			for(IAttribute aliasChild:aliases)
				dim.addAttributeConsolidation(alias, aliasChild);
		}
	}

	// remove attributes of source processor from dimension
	private void deleteAttributes(IDimension dim, ITreeManager manager) {
		ArrayList<IAttribute> todelete = new ArrayList<IAttribute>();
		for (IAttribute a : manager.getAttributes()) {
			IAttribute dimAttr = dim.getAttributeByName(a.getName());
			if (dimAttr != null)
				todelete.add(dimAttr);
		}
		dim.removeAttributes(todelete.toArray(new IAttribute[todelete.size()]));
	}

	// remove all attributes of dimension
	private void deleteAllAttributes(IDimension dim) {
		IAttribute[] attributes = dim.getAttributes();
		if (attributes.length>0) {
			dim.removeAttributes(attributes);
		}
	}
			
	protected void exportAttributes(ITreeManager manager) throws RuntimeException {
		IDimension dim = getDimension();
		switch (getMode()) {
			case CREATE: {
				deleteAllAttributes(dim);
				addAttributes(dim, manager);
				addAttributeValues(dim, manager, false);
				break;
			}
			case ADD: {
				addAttributes(dim, manager);
				addAttributeValues(dim, manager, true);
				break;
			}
			case INSERT: {
				addAttributes(dim, manager);
				addAttributeValues(dim, manager, false);
				break;
			}
			case UPDATE: {
				deleteAttributes(dim, manager);
				addAttributes(dim,manager);
				addAttributeValues(dim, manager, false);
				break;
			}
			case DELETE: {
				deleteAttributes(dim, manager);
				break;
			}
			default: {
				log.error("Unsupported mode in load "+getName());
			}
		}
	}

	@Override
	public void execute() {
		log.info("Starting load of Attributes: "+getName());
		try {
			if (isExecutable() && getDimension() != null) {
				exportAttributes(getView().renderTree(Views.EA));
			}
		}
		catch (Exception e) {
			log.error("Cannot load "+getName()+": "+e.getMessage());
			log.debug(e);
		}
		log.info("Finished load of Attributes "+getName()+".");
	}


	public void init() throws InitializationException {
		try {
			attributesBulkSize = getConfigurator().getAttributeBulksSize();
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
		super.init();
	}

}
