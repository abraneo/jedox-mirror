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
package com.jedox.etl.components.config.transform;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.List;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.config.transform.ColumnConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.AttributeNode;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.LevelNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeConfigurator extends TreeSourceConfigurator implements ITransformConfigurator {

	private Row row;
	private TransformConfigUtil util;
	private LinkedHashMap<String,Attribute> attributesMap = new LinkedHashMap<String,Attribute>();

	public Row getRow() {
		return row;
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		// TODO Auto-generated method stub
		return util.getFunctions();
	}

	private Element getTarget() {
		return getXML().getChild("target");
	}

	protected void setAttributes(Row row, String levelName, List<?> attributes) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		for (int k=0; k<attributes.size(); k++) {
			Element attribute = (Element) attributes.get(k);
			String ainputName = conf.getInputName(attribute);
			String aname = conf.getColumnName(attribute,ainputName);
			if(row.getColumn(levelName,LevelNode.class).getAttribute(aname)!=null){
				throw new ConfigurationException("Attribute " + aname + " has more than one input assigned at " + levelName);
			}
			String avalue = conf.getInputValue(attribute);
			Attribute definition = attributesMap.get(aname);
			if(definition==null){
				throw new ConfigurationException("Attribute " + aname + " is not defined in the attributes list.");
			}

			AttributeNode acolumn = ColumnNodeFactory.getInstance().createAttributeNode(definition, new Column(ainputName));
			row.getColumn(levelName,LevelNode.class).setAttribute(acolumn);
			//value is not null if and only if attribute is constant
			if (avalue != null) acolumn.setValue(avalue);
		}
	}

	private Double getWeight(String weight) throws ConfigurationException {
		try {
			return Double.parseDouble(weight);
		}
		catch (NumberFormatException e) {
			throw new ConfigurationException("Constant weight in transform "+getName()+" is not numeric.");
		}
	}
	
	public ElementType getGlobalElementType() {
		return getTarget().getAttributeValue("type","numeric").equalsIgnoreCase("numeric") ? ElementType.ELEMENT_NUMERIC : ElementType.ELEMENT_STRING;
	}

	protected Row setLevels() throws ConfigurationException {
		Row row = new Row();
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		List<?> columns = getChildren(getTarget(),"level");
		for (int j=0; j<columns.size(); j++) {
			Element column = (Element) columns.get(j);
			String inputName = conf.getInputName(column);
			String name = conf.getColumnName(column,"level"+(j+1));
			String value = conf.getInputValue(column);
			if (inputName.equals(NamingUtil.skipColumn()))			
				continue;					
			// String ldefault = column.getAttributeValue("default");
			LevelNode l = ColumnNodeFactory.getInstance().createLevelNode(name, new Column(inputName));
			l.setElementType(getGlobalElementType());
			//value is not null if and only if column is constant
			if (value != null) l.setValue(value);
			// l.setFallbackDefault(ldefault);
			//special notation for weight
			Element weight = column.getChild("weight");
			if (weight != null) {
				if (weight.getAttributeValue("nameref") != null)
					l.setWeight(weight.getAttributeValue("nameref"));
				if (weight.getAttributeValue("constant") != null)
					l.setWeight(getWeight(weight.getAttributeValue("constant")));
			}
			row.addColumn(l);
			//attributes for Level-Nodes
			setAttributes(row,name, getChildren(column,"attribute","attributes"));
		}
		return row;
	}

	public Views getFormat() throws ConfigurationException {
		return Views.FH;
	}
	
	public String getDefaultElement() {
		return getTarget().getAttributeValue("default");
	}

	public Boolean getSkipEmptyLevel() {
		return Boolean.parseBoolean(getTarget().getAttributeValue("skipEmpty","true"));
	}
		
	public String getDefaultParentElement() {
		return getTarget().getAttributeValue("parent");
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		util = new TransformConfigUtil(getXML(),getLocator(),getContext());
		fillAttributeseMap();
		row = setLevels();
		
	}
	
	public Collection<Attribute> getAttributes(){
		return attributesMap.values();
	}
	

	private void fillAttributeseMap() throws ConfigurationException {
		
		List<?> columns = getChildren(getTarget().getChild("attributes"),"attribute");
		for (int j=0; j<columns.size(); j++) {
			Element attcolumn = (Element) columns.get(j);
			ElementType elementType = ElementType.ELEMENT_STRING;		
			if (attcolumn.getAttributeValue("type","string").equalsIgnoreCase("numeric")) {
				elementType = ElementType.ELEMENT_NUMERIC;
			}
			String name = attcolumn.getAttributeValue("name");
			if (attributesMap.containsKey(name))
				throw new ConfigurationException("Attribute \"" + name + "\" exists more than once.");
			
			Attribute definition = new Attribute(name,elementType);
			definition.setMode(AttributeModes.ATTRIBUTE);
			attributesMap.put(name, definition);
		}	
	}
}
