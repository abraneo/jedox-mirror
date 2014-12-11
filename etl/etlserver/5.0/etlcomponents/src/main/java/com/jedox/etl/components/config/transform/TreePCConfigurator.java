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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.transform;

import java.util.List;

import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.source.FilterConfigurator;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.config.transform.ColumnConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.ColumnNode;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn.ColumnTypes;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreePCConfigurator extends TreeSourceConfigurator implements
		ITransformConfigurator {

	private ColumnManager manager;
	private TransformConfigUtil util;

	public ColumnManager getColumnManager() {
		return manager;
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		// TODO Auto-generated method stub
		return util.getFunctions();
	}

	public RowFilter getFilter() throws ConfigurationException {
		FilterConfigurator fc = new FilterConfigurator(getContext(), getParameter());
		return fc.getFilter(getXML().getChild("filter"));
	}

	protected Element getTarget() {
		return getXML().getChild("target");
	}

	protected CoordinateNode setAttribute(ColumnManager manager, Element element) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(element);
		String name = conf.getColumnName(element,inputName);
		String value = conf.getInputValue(element);
		// String ldefault = element.getAttributeValue("default");
		String etype = element.getAttributeValue("type","TEXT");
		ColumnTypes columnType;
		if (etype.equalsIgnoreCase("TEXT")) {
			etype = ElementType.ELEMENT_STRING.toString();
			columnType = ColumnTypes.attribute;
		}
		else if (etype.equalsIgnoreCase("NUMERIC")) {
			etype = ElementType.ELEMENT_NUMERIC.toString();
			columnType = ColumnTypes.attribute;
		}
		else { //is an alias
			etype = ElementType.ELEMENT_STRING.toString();
			columnType = ColumnTypes.alias;
		}
		CoordinateNode c = manager.addCoordinate(name, inputName);
		//value is not null if and only if column is constant
		c.setConstantValue(value);
		// c.setFallbackDefault(ldefault);
		c.setElementType(etype);
		//also add as attribute
		ColumnNode a = new ColumnNode(name);
		a.mimic(c);
		a.setColumnType(columnType);
		manager.addColumn(a);
		return c;
	}

	protected CoordinateNode setCoordinate(ColumnManager manager, Element element, String name) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(element);
		String value = conf.getInputValue(element);
		// String ldefault = element.getAttributeValue("default");
		CoordinateNode c = manager.addCoordinate(name, inputName);
		//value is not null if and only if column is constant
		c.setElementType(element.getAttributeValue("type"));
		c.setConstantValue(value);
		// c.setFallbackDefault(ldefault);
		return c;
	}

	protected CoordinateNode setWeight(ColumnManager manager, Element weight) throws ConfigurationException {
		if (weight != null)
			return setCoordinate(manager,weight, NamingUtil.getElementnameWeight());
		CoordinateNode c = new CoordinateNode(NamingUtil.getElementnameWeight());
		c.setConstantValue(1);
		manager.addColumn(c);
		return c;
	}

	protected CoordinateNode setType(ColumnManager manager, Element type) throws ConfigurationException {
		if (type != null)
			return setCoordinate(manager,type, NamingUtil.getElementnameType());
		CoordinateNode c = new CoordinateNode(NamingUtil.getElementnameType());
		c.setConstantValue("N");
		manager.addColumn(c);
		return c;
	}

	protected ColumnManager setColumns() throws ConfigurationException {
		ColumnManager manager = new ColumnManager();
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
		manager = setColumns();
	}

}
