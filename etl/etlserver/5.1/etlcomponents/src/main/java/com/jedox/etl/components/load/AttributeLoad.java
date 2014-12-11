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
import java.util.HashMap;
import java.util.HashSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.node.tree.ITreeExporter;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.palojlib.interfaces.*;
import com.jedox.palojlib.interfaces.IElement.ElementType;


public class AttributeLoad extends AbstractDimensionLoad {

	private static final Log log = LogFactory.getLog(AttributeLoad.class);
	private int addedAttributes = 0;
	private int attributeBulkSize;
	private HashMap<String,IAttribute> attributesInDim = null;


	public AttributeLoad() {
		setConfigurator(new DimensionConfigurator());
	}

	public AttributeLoad(IConfigurator configurator) throws InitializationException {
		setConfigurator(configurator);
		init();
	}

	private void addAttributeValues(IDimension dim, ITreeExporter exporter, boolean skipEmpty) throws RuntimeException {
		IElement[] treeElements;
		while (exporter.hasNext()){
			treeElements = exporter.getNextBulk();		
			//iterate over all elements and attributes
			for (Attribute a : exporter.getAttributes()) {
				String attributeName = a.getName();
				ArrayList<String> attributeValues = new ArrayList<String>();
				IAttribute attribute = attributesInDim.get(attributeName);
				ArrayList<IElement> elementsBulk = new ArrayList<IElement>();
				if (attribute != null) {
					//get the non null values of this attribute for each element present in the dimension
					for (int j=0; j < treeElements.length; j++) {
						IElement e = treeElements[j];
						String elementName = e.getName();
						Object attributeValueObject = null;
						try {
							attributeValueObject = TypeConversionUtil.convertAttribute(attribute, e);
						} catch (NumberFormatException nfe) {
							log.warn("Failed to insert non-numerical attribute value \"" + e.getAttributeValue(a.getName()) + "\" for the numerical element \""+elementName + "\" and its numeric attribute \"" + attribute.getName() + "\"");
							continue;
						}
						if (attributeValueObject != null || !skipEmpty ) {
							String attributeValue = (attributeValueObject != null) ? attributeValueObject.toString() : "";
							if (attribute.getType() == ElementType.ELEMENT_NUMERIC) {
								if (skipEmpty && (attributeValue.equals("0") || attributeValue.replace(",", ".").equals("0.0"))) { 
									continue;							
								} 
							}
							IElement element = dim.getElementByName(e.getName(), false);
							if (element != null) {
								//do not assign numeric attribute values to consolidated elements because splashing is disabled for now!
								if (!(element.getType() == ElementType.ELEMENT_CONSOLIDATED && attribute.getType() == ElementType.ELEMENT_NUMERIC)) {
									elementsBulk.add(element);
									attributeValues.add(attributeValue);
									//	if(j==treeElements.length-1 || attributeValues.size() >= attributesBulkSize){// if it is the last element in the dimension or it exceeded the bulk size
									//		exportSingleAttribute(dim,elementsBulk, attributeValues, attribute);
									//	}
								}
								else {
									if (!(attributeValue.equals("0") || attributeValue.replace(",", ".").equals("0.0"))) { //null or empty attribute values are converted to "0.0" in convertToNumeric
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
			}
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
		IAttribute alias = attributesInDim.get("Alias");
		if (alias == null) {
			alias =  dim.addAttribute("Alias",ElementType.ELEMENT_STRING);
			attributesInDim.put("Alias",alias);
		}
		return alias;
	}

	// add attributes from source processor to dimension
	private void addAttributes(IDimension dim, ITreeExporter exporter) throws RuntimeException {
		Attribute[] attributes = exporter.getAttributes();
		ArrayList<IAttribute> aliases = new ArrayList<IAttribute>();
		if(attributesInDim==null){
			buildAttributesMap(dim);
		}
		for (Attribute a : attributes) {
			if (attributesInDim.get(a.getName()) == null) {
				switch (a.getType()) {
					case ELEMENT_NUMERIC: attributesInDim.put(a.getName(),dim.addAttribute(a.getName(), ElementType.ELEMENT_NUMERIC)); break;
					default: attributesInDim.put(a.getName(),dim.addAttribute(a.getName(), ElementType.ELEMENT_STRING));
				}
				if (a.getMode().equals(AttributeModes.ALIAS)) {
					aliases.add(attributesInDim.get(a.getName()));
				}
			}
		}
		if(aliases.size()>0){
			IAttribute alias = getAlias(dim);
			for(IAttribute aliasChild:aliases)
				dim.addAttributeConsolidation(alias, aliasChild);
		}
	}

	private void buildAttributesMap(IDimension dim) {
		attributesInDim = new HashMap<String, IAttribute>();
		IAttribute[] atts = dim.getAttributes();
		for(IAttribute att:atts)
			attributesInDim.put(att.getName(), att);
	}


	private void deleteAttributeValues(IDimension dim, ITreeExporter exporter) throws RuntimeException {
		// get set of elements from dimension
		HashSet<IElement> elementsSet = new HashSet<IElement>();
		if(attributesInDim==null){
			buildAttributesMap(dim);
		}
		
		while (exporter.hasNext()){
			IElement[] treeElements = exporter.getNextBulk();
			for (IElement treeElement : treeElements) {
				IElement dimElement = dim.getElementByName(treeElement.getName(), false);
				if (dimElement!=null)
					elementsSet.add(dimElement);
			}
			//iterate over all attributes
			int deletedAttributes = 0;
			for (IAttribute a : exporter.getAttributes()) {
				IAttribute attribute = attributesInDim.get(a.getName());
				if(attribute!=null){
					// remove attribute values
					dim.removeAttributeValues(attribute,elementsSet.toArray(new IElement[elementsSet.size()]));
					deletedAttributes++;
				}
				else {
					log.warn("Attribute "+a.getName()+" of source not existing in dimension "+dim.getName()+" and therefor values will not be deleted.");
				}
			}
			if(deletedAttributes!=0)
				log.info("Deleted values of "+ deletedAttributes +" attributes for "+elementsSet.size()+" elements");			
		}
	}	

	// remove attributes of source processor from dimension
	private void deleteAttributes(IDimension dim, ITreeExporter exporter) {
		ArrayList<IAttribute> todelete = new ArrayList<IAttribute>();
		if(attributesInDim==null){
			buildAttributesMap(dim);
		}
		for (IAttribute a : exporter.getAttributes()) {
			IAttribute dimAttr = attributesInDim.get(a.getName());
			if (dimAttr != null){
				todelete.add(dimAttr);
				attributesInDim.remove(a.getName());
			}
		}
		dim.removeAttributes(todelete.toArray(new IAttribute[todelete.size()]));
	}

	// remove all attributes of dimension
	private void deleteAllAttributes(IDimension dim) {
		IAttribute[] attributes = dim.getAttributes();
		if (attributes.length>0) {
			dim.removeAttributes(attributes);
		}
		attributesInDim = new HashMap<String, IAttribute>();
	}

	protected void exportAttributes(ITreeExporter exporter) throws RuntimeException {
		if(exporter.getAttributes()==null || exporter.getAttributes().length==0){
			log.info("No attributes are defined");
			return;
		}

		IDimension dim = getDimension();
		dim.getElements(false);
		dim.setCacheTrustExpiry(3600);
		exporter.setBulkSize(attributeBulkSize);
		exporter.setWithAttributes(true);		
		try {
			switch (getMode()) {
			case CREATE: {
				deleteAllAttributes(dim); // also attributes not in source will get deleted
				addAttributes(dim, exporter);
				addAttributeValues(dim, exporter, false);
				break;
			}
			case ADD: {
				addAttributes(dim, exporter);
				addAttributeValues(dim, exporter, true);
				break;
			}
			case INSERT: {
				addAttributes(dim, exporter);
				addAttributeValues(dim, exporter, false);
				break;
			}
			case UPDATE: {
				deleteAttributes(dim, exporter);  // all attributes of source will get deleted
				addAttributes(dim,exporter);
				addAttributeValues(dim, exporter, false);
				break;
			}
			case DELETE: {
				deleteAttributeValues(dim, exporter);
				break;
			}
			default: {
				log.error("Unsupported mode in load "+getName());
			}
			}
		} catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}finally{
			if(dim!=null)
				dim.resetCache();
		}
	}

	// currently not use
	// For non-treebased sources a tree is created with TreeManagerNG (not optimised with FlatTreeExporter)	
	public void executeLoad() {
		log.info("Starting load of Attributes: "+getName());
		try {
			if (getDimension() != null) {
				ITreeManager manager = getView().renderTree(Views.EA).getManager();				
				exportAttributes(manager.getExporter());
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
			attributeBulkSize = getConfigurator().getAttributeBulkSize();
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
		super.init();
	}

}
