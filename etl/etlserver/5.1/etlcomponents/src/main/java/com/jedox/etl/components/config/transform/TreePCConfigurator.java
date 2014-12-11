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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.transform;

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
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreePCConfigurator extends TreeSourceConfigurator implements
		ITransformConfigurator {

	private Row row;
	private TransformConfigUtil util;

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

	protected Element getTarget() {
		return getXML().getChild("target");
	}
	
	protected AttributeNode setAttribute(Row row, Element element) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(element);
		String name = conf.getColumnName(element,inputName);
		String value = conf.getInputValue(element);
		// String ldefault = element.getAttributeValue("default");
		
		ElementType elementType = element.getAttributeValue("type","string").equalsIgnoreCase("numeric") ? ElementType.ELEMENT_NUMERIC : ElementType.ELEMENT_STRING;;
		AttributeModes mode = AttributeModes.ATTRIBUTE;
		//define as attribute
		Attribute definition = new Attribute(name,elementType);
		definition.setMode(mode);
		AttributeNode acolumn = ColumnNodeFactory.getInstance().createAttributeNode(definition, new Column(inputName));
		if (value != null) acolumn.setValue(value);
		row.addColumn(acolumn);
		return acolumn;
	}

	protected CoordinateNode setCoordinate(Row manager, Element element, String name) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(element);
		String value = conf.getInputValue(element);
		if(element.getName().equals("type") && conf.isInputConstant(element) && (!value.equalsIgnoreCase("N")&& !value.equalsIgnoreCase("S")))
			throw new ConfigurationException("Type can only have values N or S.");
		// String ldefault = element.getAttributeValue("default");
		CoordinateNode c = ColumnNodeFactory.getInstance().createCoordinateNode(name, new Column(inputName));
		//value is not null if and only if column is constant
		if (value != null) c.setValue(value);
		manager.addColumn(c);
		// c.setFallbackDefault(ldefault);
		return c;
	}

	protected CoordinateNode setWeight(Row manager, Element weight) throws ConfigurationException {
		if (weight != null)
			return setCoordinate(manager,weight, NamingUtil.getElementnameWeight());
		CoordinateNode c = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameWeight(),null);
		c.setValue(1);
		manager.addColumn(c);
		return c;
	}

	protected CoordinateNode setType(Row manager, Element type) throws ConfigurationException {
		if (type != null)
			return setCoordinate(manager,type, NamingUtil.getElementnameType());
		CoordinateNode c = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameType(),null);
		c.setValue("N");
		manager.addColumn(c);
		return c;
	}

	protected Row setColumns() throws ConfigurationException {
		Row manager = new Row();
		setCoordinate(manager,getTarget().getChild("parent"),NamingUtil.getElementnameParent());
		setCoordinate(manager,getTarget().getChild("child"),NamingUtil.getElementnameChild());
		setWeight(manager, getTarget().getChild("weight"));
		setType(manager,getTarget().getChild("type"));
		List<Element> attributes = getChildren(getTarget(),"attribute");
		for (Element a : attributes)
			setAttribute(manager,a);
		return manager;
	}

	public Views getFormat() throws ConfigurationException {
		return Views.PCWA;
	}
	
	public String getDefaultElement() {
		return getTarget().getAttributeValue("default");
	}

	public String getDefaultParentElement() {
		return getTarget().getAttributeValue("parent");
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		util = new TransformConfigUtil(getXML(),getLocator(),getContext());
		row = setColumns();
	}

}
