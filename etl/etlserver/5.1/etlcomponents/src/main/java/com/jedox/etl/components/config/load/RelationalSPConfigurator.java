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
package com.jedox.etl.components.config.load;

import java.util.List;

import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.load.LoadConfigurator;
import com.jedox.etl.core.config.transform.ColumnConfigurator;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.Row;

public class RelationalSPConfigurator extends LoadConfigurator {
	
	boolean needSource = false;

	public String getStoredProcedureName() throws ConfigurationException {
		return getXML().getChild("sp").getTextTrim();
	}
	
	public String getSchemaName() throws ConfigurationException {
		Element schema = getXML().getChild("schema");
		if (schema != null && !schema.getTextTrim().isEmpty()) {
			return schema.getTextTrim();
		}
		return null;
	}
	
	private void setCoordinate(Row manager, Element column) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(column);
		String name = conf.getColumnName(column,inputName);
		String value = conf.getInputValue(column);
		if(manager.getColumn(name)!= null)
			throw new ConfigurationException("Coordinate \"" + name + "\" exists more than once.");
		CoordinateNode c = ColumnNodeFactory.getInstance().createCoordinateNode(name,new Column(inputName));
		if (value != null)
			c.setValue(value);
		else
			needSource=true;
		manager.addColumn(c);

	}
	
	public Row getCoordinates() throws ConfigurationException {
		Row manager = new Row();
		//process coordinate columns
		List<Element> coordinate = getChildren(getXML(),"coordinate");
		for (Element column : coordinate)
			setCoordinate(manager,column);
		return manager;
	}
	
	public boolean needSource(){
		return needSource;
	}
	
	public boolean hasSource(){
		return (getXML().getChild("source")!=null);
	}
}
