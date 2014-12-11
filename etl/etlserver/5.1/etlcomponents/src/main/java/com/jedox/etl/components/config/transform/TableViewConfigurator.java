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

import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.FilterConfigurator;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.filter.RowFilter;

public class TableViewConfigurator extends TableSourceConfigurator implements
		ITransformConfigurator {

	private Row row = new Row();
	private TransformConfigUtil util;

	public Row getRow() throws RuntimeException {
		/*
		if (manager == null) {
			try {
				manager = util.getColumns();
				setAliasMap(AliasMap.build(manager.getRow().getOutputDescription()));
			}
			catch (ConfigurationException e) {
				throw new RuntimeException(e);
			}
		}
		*/
		return row;
	}

	public RowFilter getFilter() throws ConfigurationException {
		FilterConfigurator fc = new FilterConfigurator(getContext(), getParameter());
		return fc.getFilter(getXML().getChild("filter"));
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		return util.getFunctions();
	}

	public String getSorter() throws ConfigurationException {
		Element sorter = getXML().getChild("sorter");
		StringBuffer orderString = new StringBuffer();
		if (sorter != null) {
			orderString.append(" order by");
			List<?> inputs = sorter.getChildren("input");
			for (int i=0; i<inputs.size(); i++) {
				Element input = (Element) inputs.get(i);
				if (input != null)  {
					String order = input.getAttributeValue("order","asc");
					String alias = input.getAttributeValue("nameref");
					orderString.append(" "+escapeName(alias)+" "+order);
					if (i<inputs.size()-1) orderString.append(",");
				}
			}
		}
		return orderString.toString();
	}

	public void configure() throws ConfigurationException {
		try {
			setName(getXML().getAttributeValue("name"));
			if(getXML().getChild("sorter") ==  null  &&  getXML().getChild("filter") ==  null  &&  getXML().getChild("start") ==  null  && getXML().getChild("end") ==  null ){
				throw new ConfigurationException( "The content of element 'transform' is not complete. One of '{ filter, sorter, start, end}' is expected.");
			}
			util = new TransformConfigUtil(getXML(),getLocator(),getContext());
			//manager = util.getColumns();
			setAliasMap(new AliasMap());
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to configure transform "+getName()+": "+e.getMessage());
		}
	}

}
