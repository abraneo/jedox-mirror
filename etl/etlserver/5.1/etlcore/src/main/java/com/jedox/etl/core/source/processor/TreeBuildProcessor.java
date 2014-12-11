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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source.processor;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.AttributeNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.LevelNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
/**
 * Generates a tree representation (internally used by tree based sources) by interpreting a column based input as a specific view of a tree.
 * Note: Only information available in the flat input can be used for building the tree. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TreeBuildProcessor extends IdProcessor implements ITreeProcessor {
	
	private ITreeManager manager;
	private ElementType globalElementType = ElementType.ELEMENT_NUMERIC;
	private boolean skipEmptyLevel = false;
	private Views view;
	private String name;
	private String defaultElement;
	private String defaultParentElement;
	private List<AttributeNode> attributes;	
	private static final Log log = LogFactory.getLog(TreeBuildProcessor.class);
	
	private double convertToDouble(Row row, int pos) {
		if (pos < row.size()) {
			try {
				return Double.parseDouble(row.getColumn(pos).getValueAsString()); 
			}
			catch (Exception e) {}
		}
		return 1;
	}
	
	private ElementType getAttributeType(IColumn column) {
		if (column instanceof AttributeNode)
			return ((AttributeNode)column).getDefinition().getType();
		else 
			return ElementType.ELEMENT_STRING;
	}	

	
	public TreeBuildProcessor(IProcessor sourceProcessor, Views view) {
		super(sourceProcessor);
		this.name = sourceProcessor.getName();
		this.view = view;
	}
	
	protected void init() throws RuntimeException {
		this.manager = new TreeManagerNG(name);
		// add all attribute defintions even if source is empty)
		if (attributes!=null) {
			for (AttributeNode c : attributes) { 
				Attribute attribute = manager.addAttribute(c.getName(), c.getDefinition().getType());
				attribute.setMode(c.getDefinition().getMode());
			}		
		}	
	}	
	
	public void setGlobalElementType(ElementType globalElementType) {
		this.globalElementType = globalElementType;
	}
	
	public void setSkipEmptyLevel(boolean skipEmptyLevel) {
		this.skipEmptyLevel = skipEmptyLevel;
	}
	
	public void setDefaultElement(String defaultElement) {
		this.defaultElement = defaultElement;
	}
	
	public void setDefaultParentElement(String defaultParentElement) {
		this.defaultParentElement = defaultParentElement;
	}
	
	private String getValues(Row row) {
		StringBuffer names = new StringBuffer();
		for (int j=0; j<row.size();j++) {
			IColumn c = row.getColumn(j);
			names.append(c.getValueAsString()+",");
		}
		names.deleteCharAt(names.length()-1);
		return names.toString();
	}
	
	public void setAttributes(List<AttributeNode> attributes) throws RuntimeException {
		this.attributes = attributes;
	}	
	
	private void addAttributeValue(ITreeManager manager, String attribute, String element, Object value) {
		if (manager.getAttributeByName(attribute) != null) {
			manager.addAttributeValue(attribute, element, value);
		}
	}
		
	protected void generateFHWAFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		MessageHandler handler = new MessageHandler(log);
		Row row = processor.next();
		//int countRows = 0;
		while (row != null) {
			try {
				ITreeElement parent = null;
				for (int i=0; i<row.getColumns(LevelNode.class).size(); i++) {
					LevelNode cn = row.getColumns(LevelNode.class).get(i);
					//take value from the result set, where the column points to.
					String name = cn.getValueAsString();
					if (name.trim().isEmpty()) {
						if (!skipEmptyLevel) {
							// Check if after empty node a lower level has a filled node
							for(int j=i+1;j<row.getColumns(LevelNode.class).size(); j++){
								LevelNode ln = row.getColumns(LevelNode.class).get(j);
								if((ln.getValue()) != null && !ln.getValue().toString().trim().isEmpty() ){
									handler.warn("In transform "+processor.getName()+ " value " + ln.getValueAsString()+ " of "+ln.getName() + " has empty parent and is therefor ignored. Use a default value in the source to fix this.");
									break;
								}
							}
							break;
						}	
					} 
					// Ignore node if equal name as parent node
					else if (parent != null && name.equals(parent.getName())) {
						log.debug("Ignore consolidation of node "+name+" to parent with same name");
					}
					else {
						ITreeElement tn = manager.provideElement(name, globalElementType);
						manager.addConsolidation(parent,tn,cn.getWeight());
						Row attributes = cn.getAttributes();
						for (AttributeNode c : attributes.getColumns(AttributeNode.class)) {
							//check if there is a definition.
							IAttribute a = manager.getAttributeByName(c.getName());
							if (a == null) 
								throw new RuntimeException("Internal Error: Attribute "+c.getName()+" not existing in transform "+processor.getName());
							manager.addAttributeValue(a.getName(), tn.getName(), c.getValue());
						}
						parent = tn;
					}
				}
			}
			catch (Exception e) {
				handler.error("Failed to build tree from data "+getValues(row)+" in transform "+processor.getName()+": "+e.getMessage());
			}
			row = processor.next();
		}
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
					IElement tn = manager.provideElement(name, ElementType.ELEMENT_NUMERIC);
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
					IElement tn = manager.provideElement(name, ElementType.ELEMENT_NUMERIC);
					if (parent != null) manager.addConsolidation(parent,tn,1);
					parent = tn;
				}
			}
			row = processor.next();
		}
	}
	
	protected void generatePCWFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		Row row = processor.next();
		while (row != null) {
			String parentName = row.getColumn(0).getValueAsString();
			String childName = row.getColumn(1).getValueAsString();
			String type = row.getColumn(3).getValueAsString();
			IElement parent = null;
			if (!parentName.trim().isEmpty()) {
				parent = manager.getElement(parentName);
				if (parent==null) {	// if parent element is not defined as child yet the default element type Numeric is assigned
					parent = manager.provideElement(parentName,  ElementType.ELEMENT_NUMERIC);
				}
			}
			if (!childName.trim().isEmpty()) {
				double weight = convertToDouble(row,2);
				IElement child = manager.provideElement(childName,getElementType(type));
				manager.addConsolidation(parent,child,weight);
				for (int i=3; i<row.size(); i++) {
					addAttributeValue(manager,row.getColumn(i).getName(), child.getName(), row.getColumn(i).getValue());
				}
			}
			row = processor.next();
		}
	}
	
	protected void generateEAFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		Row row = processor.next();
		List<Attribute> attributesList = new ArrayList<Attribute>();
		if (row != null && attributes == null) { //add attribute definitions once
			for (int i=1; i<row.size(); i++) {
				attributesList.add(manager.addAttribute(row.getColumn(i).getName(), getAttributeType(row.getColumn(i))));
			}
		}
		while (row != null) {
			String elementName = row.getColumn(0).getValueAsString();
			if (!elementName.trim().isEmpty()) {
				//create with condolidation on root level
				IElement element = manager.provideElement(elementName,ElementType.ELEMENT_NUMERIC);
				for (int i=1; i<row.size(); i++) {
					addAttributeValue(manager,row.getColumn(i).getName(), element.getName(), row.getColumn(i).getValue());
				}
			}
			row = processor.next();
		}
	}
	

	protected void generateLEWAFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		
		HashMap<Integer,IElement> parents = new HashMap<Integer,IElement>();  		
		Row row = processor.next();	
		if (row != null && attributes == null) { //add attribute definitions once
			for (int i=4; i<row.size(); i++) {
				manager.addAttribute(row.getColumn(i).getName(), getAttributeType(row.getColumn(i)));
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
					addAttributeValue(manager,row.getColumn(i).getName(), element.getName(), row.getColumn(i).getValue());
				}
			}	
			row = processor.next();
		}
	}	
	
	
	private ElementType getElementType(String type) throws RuntimeException {
		if (type.trim().isEmpty() || "N".equalsIgnoreCase(type)) return ElementType.ELEMENT_NUMERIC;
		if ("S".equalsIgnoreCase(type)) return ElementType.ELEMENT_STRING;
		if ("C".equalsIgnoreCase(type)) return ElementType.ELEMENT_CONSOLIDATED;
		throw new RuntimeException("type " +  type + " is not a valid element type, possible values are N,S and C");
	}
	
	protected void generateNCFormat(ITreeManager manager, IProcessor processor) throws RuntimeException {
		IElement parent = null;
		Row row = processor.next();
		if (row != null && attributes == null) { //add attribute definitions once
			for (int i=3; i<row.size(); i++) {
				manager.addAttribute(row.getColumn(i).getName(), getAttributeType(row.getColumn(i)));
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
						te = manager.provideElement(name,getElementType(type));
						for (int i=3; i<row.size(); i++) {
							addAttributeValue(manager,row.getColumn(i).getName(), te.getName(), row.getColumn(i).getValue());
						}
					}
					else if ("C".equalsIgnoreCase(type)) {
						//add on root level
						te = manager.provideElement(name,ElementType.ELEMENT_CONSOLIDATED);
						parent = te;
						for (int i=3; i<row.size(); i++) {
							addAttributeValue(manager,row.getColumn(i).getName(), te.getName(), row.getColumn(i).getValue());
						}
					}
					else if (type==null || type.trim().isEmpty()) {
						// Empty type indicates child of last C-node
						te = manager.getElement(name);
						if (te == null) {
							//this should not be, when the format is correct. but we accept it gracefully :-)
							te = manager.provideElement(name, ElementType.ELEMENT_NUMERIC);
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
	
	public int getRowsAccepted() {
		return manager.getElements(false).length;
	}
	
	/**
	 * generates the tree from a flat (column based) representation by interpreting it as view on a tree.
	 * @param processor the processor holding the flat data structure
	 * @param view the view format as which the data should be interpreted
	 * @throws RuntimeException
	 */
	public void run() throws RuntimeException {
		boolean autocommit = manager.isAutoCommit();
		manager.setAutoCommit(false);
		if (view.equals(Views.NONE)) 
			throw new RuntimeException("A format has to be specified for the generation of a tree.");
		else {
			long start = System.nanoTime();
			switch (view) {
				case FH: generateFHFormat(manager, getSourceProcessor()); break;
				case FHW: generateFHWFormat(manager, getSourceProcessor()); break;
				case FHWA: generateFHWAFormat(manager, getSourceProcessor()); break;
				case NCW: generateNCFormat(manager, getSourceProcessor()); break;
				case NCWA: generateNCFormat(manager, getSourceProcessor()); break;
				case PC: generatePCWFormat(manager, getSourceProcessor()); break;
				case EA: generateEAFormat(manager, getSourceProcessor()); break;
				case PCW: generatePCWFormat(manager, getSourceProcessor()); break;
				case PCWA: generatePCWFormat(manager, getSourceProcessor()); break;
				case LEW: generateLEWAFormat(manager, getSourceProcessor()); break;				
				case LEWTA: generateLEWAFormat(manager, getSourceProcessor()); break;				
				default: log.error("Generation from format "+view.toString()+" not supported yet.");
			}
			addDefaultElement(defaultElement, defaultParentElement);
			manager.commitConsolidations();
			manager.commitAttributeValues();
			long buildDuration = System.nanoTime() - start;
			addProcessingTime(buildDuration);
		}
		manager.setAutoCommit(autocommit);
	}

	@Override
	public ITreeManager getManager() {
		return manager;
	}
	
}
