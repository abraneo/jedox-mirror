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
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.ColumnNode;
import com.jedox.etl.core.node.LevelNode;
import com.jedox.etl.core.node.IColumn.ColumnTypes;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeConfigurator extends TreeSourceConfigurator implements ITransformConfigurator {

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

	private Element getTarget() {
		return getXML().getChild("target");
	}

	protected void setAttributes(ColumnManager manager, String levelName, List<?> attributes) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		for (int k=0; k<attributes.size(); k++) {
			Element attribute = (Element) attributes.get(k);
			String ainputName = conf.getInputName(attribute);
			String aname = conf.getColumnName(attribute,ainputName);
			String avalue = conf.getInputValue(attribute);
			String etype = attribute.getAttributeValue("type","TEXT");
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
			ColumnNode acolumn = manager.addAttribute(levelName, aname, ainputName, columnType.toString());
			acolumn.setElementType(etype);
			//value is not null if and only if attribute is constant
			acolumn.setConstantValue(avalue);
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

	protected ColumnManager setLevels() throws ConfigurationException {
		ColumnManager manager = new ColumnManager();
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		List<?> columns = getChildren(getTarget(),"level");
		ElementType eType = getElementType(getTarget().getAttributeValue("type"));
		for (int j=0; j<columns.size(); j++) {
			Element column = (Element) columns.get(j);
			String inputName = conf.getInputName(column);
			String name = conf.getColumnName(column,"level"+(j+1));
			String value = conf.getInputValue(column);
			// String ldefault = column.getAttributeValue("default");
			LevelNode l = manager.addLevel(name,inputName);
			//value is not null if and only if column is constant
			l.setElementType(eType.toString());
			l.setConstantValue(value);
			// l.setFallbackDefault(ldefault);
			//special notation for weight
			Element weight = column.getChild("weight");
			if (weight != null) {
				if (weight.getAttributeValue("nameref") != null)
					l.setWeight(weight.getAttributeValue("nameref"));
				if (weight.getAttributeValue("constant") != null)
					l.setWeight(getWeight(weight.getAttributeValue("constant")));
			}
			//attributes for Level-Nodes
			setAttributes(manager,name, getChildren(column,"attribute","attributes"));
		}
		return manager;
	}

	public Views getFormat() throws ConfigurationException {
		return Views.FH;
	}
	
	public String getDefaultElement() {
		return getTarget().getAttributeValue("default");
	}

	public Boolean getSkipEmptyLevel() {
		return Boolean.parseBoolean(getTarget().getAttributeValue("skipEmpty","false"));
	}
		
	public String getDefaultParentElement() {
		return getTarget().getAttributeValue("parent");
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		util = new TransformConfigUtil(getXML(),getLocator(),getContext());
		manager = setLevels();
		
	}
}
