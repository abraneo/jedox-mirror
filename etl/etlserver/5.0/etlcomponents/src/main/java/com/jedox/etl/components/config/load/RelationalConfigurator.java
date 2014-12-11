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
package com.jedox.etl.components.config.load;

import org.jdom.Element;
import java.util.HashMap;
import java.util.Map;
import java.util.List;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.load.LoadConfigurator;
import com.jedox.etl.core.persistence.IPersistence.UpdateModes;
import com.jedox.etl.core.persistence.PersistorDefinition.AggregateModes;

public class RelationalConfigurator extends LoadConfigurator {

	public String getTableName() throws ConfigurationException {
		Element table = getXML().getChild("table");
		if (table != null) {
			String name = table.getAttributeValue("name",getName()).trim();
			return name;  
		}
		return getName();
	}
	
	public String getSchemaName() throws ConfigurationException {
		Element table = getXML().getChild("table");
		if (table != null) {
			String schema = table.getAttributeValue("schema","").trim();
			return schema;
		}
		//return getLocator().getRootName();
		return "";
	}
	
	public Map<String,UpdateModes> getUpateStrategy() throws ConfigurationException {
		HashMap<String,UpdateModes> result = new HashMap<String,UpdateModes>();
		Element update = getXML().getChild("table");
		if (update != null) {
			List<?> columns = update.getChildren("column");
			for (int i=0; i<columns.size(); i++) {
				Element c = (Element) columns.get(i);
				String name = c.getAttributeValue("nameref");
				String m = c.getAttributeValue("mode");
				UpdateModes mode = UpdateModes.none;
				if (m != null) {
					if (m.equalsIgnoreCase("numeric")) m = UpdateModes.sum.toString();
					if (m.equalsIgnoreCase("text")) m = UpdateModes.last.toString();
					mode = UpdateModes.valueOf(m);
				}
				result.put(name, mode);
			}
		}
		return result;
	}
	
	public boolean usePlainNames() throws ConfigurationException {
		Element table = getXML().getChild("table");
		if (table != null) {
			return table.getAttributeValue("plain","false").equalsIgnoreCase("true");
		}
		return false;
	}
	
	private boolean doAggregate() throws ConfigurationException {
		boolean doAggregate = false;
		Element table = getXML().getChild("table");
		if (table != null) {
			doAggregate = !table.getAttributeValue("aggregate", "false").equalsIgnoreCase("false");
		}
		return doAggregate;
	}
	
	public String getAggregationMode() throws ConfigurationException {
		Element table = getXML().getChild("table");
		String aggregateMode = AggregateModes.none.toString();
		if (doAggregate() && table != null) {
			String agg = table.getAttributeValue("aggregate");
			if ("row".equals(agg))
				aggregateMode = agg;
			else
				aggregateMode = AggregateModes.bulk.toString();
		}
		return aggregateMode;
	}
	
	public boolean doCreateKeyColumn() throws ConfigurationException {
		boolean doCreateKeyColumn = false;
		Element table = getXML().getChild("table");
		if (table != null) {
			doCreateKeyColumn = table.getAttributeValue("createKeyColumn", "false").equalsIgnoreCase("true");
		}
		return doCreateKeyColumn;
	}
	
	public String getPrimaryKey() throws ConfigurationException {
		Element table = getXML().getChild("table");
		if (table != null) {
			return table.getAttributeValue("primaryKey");
		}
		return null;
	}
	
	public String getPrimaryKeyGeneration() throws ConfigurationException {
		Element table = getXML().getChild("table");
		if (table != null) {
			return table.getAttributeValue("primaryKeyGeneration");
		}
		return null;
	}
	
}
