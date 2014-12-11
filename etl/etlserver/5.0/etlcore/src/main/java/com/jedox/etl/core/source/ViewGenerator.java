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
package com.jedox.etl.core.source;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
/**
 * Generates a tree representation (internally used by tree based sources) by interpreting a column based input as a specific view of a tree.
 * Note: Only information available in the flat input can be used for building the tree. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ViewGenerator {
	
	private ITreeManager manager;
	private static final Log log = LogFactory.getLog(ViewGenerator.class);
	
	private double convertToDouble(Row row, int pos) {
		if (pos < row.size()) {
			try {
				return Double.parseDouble(row.getColumn(pos).getValueAsString()); 
			}
			catch (Exception e) {}
		}
		return 1;
	}
	
	public ViewGenerator(ITreeManager manager) {
		this.manager = manager;
	}

	protected void generateFHWFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		Row row = processor.next();
		while (row != null) {
			IElement parent = null;
			for (int i = 0; i<row.size(); i+=2) {
				String name = row.getColumn(i).getValueAsString();
				if (!name.trim().isEmpty()) {
					//name = name.trim();
					Double weight = 1.0;
					if (i > 1) 
						weight = convertToDouble(row,i-1);
					//see if there is 
					IElement tn = manager.provideElement(name, row.getColumn(i).getElementType());
					if (parent != null) manager.addConsolidation(parent,tn,weight);
					parent = tn;
				}
			}
			row = processor.next();
		}
	}
	
	protected void generateFHFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		Row row = processor.next();
		while (row != null) {
			IElement parent = null;
			for (int i = 0; i<row.size(); i++) {
				String name = row.getColumn(i).getValueAsString();
				if (!name.trim().isEmpty()) {
					//name = name.trim();
					//see if there is 
					IElement tn = manager.provideElement(name, row.getColumn(i).getElementType());
					if (parent != null) manager.addConsolidation(parent,tn,1);
					parent = tn;
				}
			}
			row = processor.next();
		}
	}
	
	protected void generatePCWFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		Row row = processor.next();
		List<Attribute> attributes = new ArrayList<Attribute>();
		if (row != null) { //add attribute definitions once
			for (int i=4; i<row.size(); i++) {
				attributes.add(manager.addAttribute(row.getColumn(i).getName(), row.getColumn(i).getElementType(ElementType.ELEMENT_STRING)));
			}
		}
		while (row != null) {
			String parentName = row.getColumn(0).getValueAsString();
			String childName = row.getColumn(1).getValueAsString();
			IElement parent = null;
			if (!parentName.trim().isEmpty()) {				
				parent = manager.getElement(parentName);
				if (parent==null) {	
					parent = manager.provideElement(parentName, ElementType.ELEMENT_NUMERIC);
				}
			}
			if (!childName.trim().isEmpty()) {
				double weight = convertToDouble(row,2);
				IElement child = manager.provideElement(childName, getElementType(row.getColumn(3).getValueAsString()));
				manager.addConsolidation(parent,child,weight);
				for (int i=4; i<row.size(); i++) {
					manager.addAttributeValue(attributes.get(i-4).getName(), child.getName(), row.getColumn(i).getValue());
				}
			}
			row = processor.next();
		}
	}
	
	protected void generateEAFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		Row row = processor.next();
		List<Attribute> attributes = new ArrayList<Attribute>();
		if (row != null) { //add attribute definitions once
			for (int i=1; i<row.size(); i++) {
				attributes.add(manager.addAttribute(row.getColumn(i).getName(), row.getColumn(i).getElementType(ElementType.ELEMENT_STRING)));
			}
		}
		while (row != null) {
			String elementName = row.getColumn(0).getValueAsString();
			if (!elementName.trim().isEmpty()) {
				//create with condolidation on root level
				IElement element = manager.provideElement(elementName,row.getColumn(0).getElementType());
				for (int i=1; i<row.size(); i++) {
					manager.addAttributeValue(attributes.get(i-1).getName(), element.getName(), row.getColumn(i).getValue());
				}
			}
			row = processor.next();
		}
	}
	

	protected void generateLEWAFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		
		HashMap<Integer,IElement> parents = new HashMap<Integer,IElement>();  		
		Row row = processor.next();
		List<Attribute> attributes = new ArrayList<Attribute>();
		if (row != null) { //add attribute definitions from header line
			for (int i=4; i<row.size(); i++) {
				attributes.add(manager.addAttribute(row.getColumn(i).getName(), ElementType.ELEMENT_STRING));
			}
		}		
		while (row != null) {
			String elementName = row.getColumn(1).getValueAsString();
			int level=0;
			try {
				level = Integer.parseInt(row.getColumn(0).getValueAsString()); 
			} catch (Exception e) {}
			if (level<=0) {
				log.warn("Invalid level "+row.getColumn(0).getValueAsString()+". The element "+elementName+" is ignored.");
			}
			else {
				String type = row.getColumn(3).getValueAsString();
				IElement element = manager.provideElement(elementName,getElementType(type));
				parents.put(level, element);			
				if (level>1) {
					IElement parent=parents.get(level-1);
					if (parent==null)
						log.warn("No parent found for "+elementName);
					manager.addConsolidation(parent,element,convertToDouble(row,2));
				}
				for (int i=4; i<row.size(); i++) {
					manager.addAttributeValue(attributes.get(i-4).getName(), element.getName(), row.getColumn(i).getValue());
				}
			}	
			row = processor.next();
		}
	}	
	
	
	
	
	private ElementType getElementType(String type) {
		if ("N".equalsIgnoreCase(type)) return ElementType.ELEMENT_NUMERIC;
		if ("S".equalsIgnoreCase(type)) return ElementType.ELEMENT_STRING;
		if ("C".equalsIgnoreCase(type)) return ElementType.ELEMENT_CONSOLIDATED;
		return ElementType.ELEMENT_NUMERIC;
	}	
	
	protected void generateNCFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		IElement parent = null;
		Row row = processor.next();
		List<Attribute> attributes = new ArrayList<Attribute>();
		if (row != null) { //add attribute definitions once
			for (int i=3; i<row.size(); i++) {
				attributes.add(manager.addAttribute(row.getColumn(i).getName(), row.getColumn(i).getElementType(ElementType.ELEMENT_STRING)));
			}
		}
		if (row == null || row.size() >= 2) {
			while (row != null) {
				String type = row.getColumn(0).getValueAsString();
				String name = row.getColumn(1).getValueAsString();
				if (!name.trim().isEmpty()) {
					IElement te = null;
					double weight = convertToDouble(row,2);
					if ("N".equalsIgnoreCase(type) || "S".equalsIgnoreCase(type)) {
						te = manager.provideElement(name,("N".equalsIgnoreCase(type)) ? row.getColumn(1).getElementType() : ElementType.ELEMENT_STRING);
						for (int i=3; i<row.size(); i++) {
							manager.addAttributeValue(attributes.get(i-3).getName(), te.getName(), row.getColumn(i).getValue());
						}
					}
					else if ("C".equalsIgnoreCase(type)) {
						//add on root level
						te = manager.provideElement(name,row.getColumn(1).getElementType());
						parent = te;
						for (int i=3; i<row.size(); i++) {
							manager.addAttributeValue(attributes.get(i-3).getName(), te.getName(), row.getColumn(i).getValue());
						}
					}
					else if (type==null || type.trim().isEmpty()) {
						// Empty type indicates child of last C-node
						te = manager.getElement(name);
						if (te == null) {
							//this should not be, when the format is correct. but we accept it gracefully :-)
							te = manager.provideElement(name, row.getColumn(1).getElementType());
							manager.addConsolidation(parent,te,weight);
						} else {
							manager.addConsolidation(parent,te,weight);
						}
					 } 
					else {
						log.warn("The type "+type+" for node "+name+" is not supported in Tree format NC. It is ignored.");
					}
				}
				row = processor.next();
			}
		}
		else 
			log.error("Not a valid palo dimension format");
	}
	
	/**
	 * creates an additional element in the tree and consolidates it under a parent element if given. 
	 * Element can be used for default or non-assigned values
	 */
	public void addDefaultElement(String defaultElement, String defaultParentElement ) {
		if (defaultElement != null) {
			if (manager.getElement(defaultElement)!=null)
				log.warn("Default Element "+defaultElement+" is already existing in Tree.");
			ITreeElement element = manager.provideElement(defaultElement,ElementType.ELEMENT_NUMERIC);
			if (defaultParentElement != null) {
				ITreeElement parent = manager.getElement(defaultParentElement);
				if (parent==null)
					log.warn("Element "+defaultParentElement+" is not existing. The default element "+defaultElement+" will not get consolidated.");
				else {
					manager.addConsolidation(parent, element, 1);
				}
			}	
		}
	}
	
	/**
	 * generates the tree from a flat (column based) representation by interpreting it as view on a tree.
	 * @param processor the processor holding the flat data structure
	 * @param view the view format as which the data should be interpreted
	 * @return the tree representation
	 * @throws RuntimeException
	 */
	public ITreeManager generate(IProcessor processor, Views view, String defaultElement, String defaultParentElement) throws RuntimeException {
		boolean autocommit = manager.isAutoCommit();
		manager.setAutoCommit(false);
		if (view.equals(Views.NONE)) 
			throw new RuntimeException("A format has to be specified for the generation of a tree.");
		else {
			switch (view) {
				case FH: generateFHFormat(manager, processor); break;
				case FHW: generateFHWFormat(manager, processor); break;
				case NCW: generateNCFormat(manager, processor); break;
				case NCWA: generateNCFormat(manager, processor); break;
				case PC: generatePCWFormat(manager, processor); break;
				case EA: generateEAFormat(manager, processor); break;
				case PCW: generatePCWFormat(manager, processor); break;
				case PCWA: generatePCWFormat(manager, processor); break;
				case LEWTA: generateLEWAFormat(manager, processor); break;
				default: log.error("Generation from format "+view.toString()+" not supported yet.");
			}
			addDefaultElement(defaultElement, defaultParentElement);
			manager.commitConsolidations();
			manager.commitAttributeValues();
		}
		manager.setAutoCommit(autocommit);
		return manager;
	}
	
}
