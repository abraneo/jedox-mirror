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
package com.jedox.etl.core.util;

import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.util.List;

import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.IPersistence.UpdateModes;

public class PersistenceUtil {
	
	/**
	 * builds a column definition from a ResultSetMetaData
	 * @param meta the meta data object
	 * @return the column definition
	 */
	public static Row getColumnDefinition(ResultSetMetaData meta) {
		Row columnDefinition = new Row();
		try {
			for (int i=1;i<=meta.getColumnCount();i++) {
				Column c = new Column(meta.getColumnName(i));
				c.setValueType(meta.getColumnTypeName(i));
				columnDefinition.addColumn(c);
			}
		} catch (SQLException e) {}
		return columnDefinition;
	}


	public static Row getColumnDefinition(String[] columns) {
		return getColumnDefinition(new AliasMap(), columns);
	}

	public static Row getColumnDefinition(List<String> columnNames) {
		return getColumnDefinition(columnNames.toArray(new String[columnNames.size()]));
	}
	
	
	/**
	 * builds a column definition from an array of column names
	 * @param columns array of column names
	 * @param map an AliasMap to use the names from as overlay to the names of the string.
	 * @return the column definition
	 */
	public static Row getColumnDefinition(IAliasMap map, String[] columns) {
		Row columnDefinition = new Row();
		for (int i=0;i<columns.length;i++) {
			Column c = new Column(map.getAlias(i+1, columns[i]));
			columnDefinition.addColumn(c);
		}
		return columnDefinition;
	}
	
/*  Note: Building column definition from comma separated string problematic if column names contain comma 	
/*	
	public static Row getColumnDefinition(IAliasMap map, String meta) {
		String[] columns = meta.split(",");
		return getColumnDefinition(map, columns);
	}
*/	
	
	public static UpdateModes getDataPersistenceMode(Modes mode){
		switch (mode) {
		case CREATE : return UpdateModes.sum;
		case ADD : return UpdateModes.sum;
		case INSERT : return UpdateModes.last;
		case UPDATE : return UpdateModes.sum;
		case DELETE : return UpdateModes.first;
		default: return UpdateModes.first;
		}
	}

	
	
/*	
	public static Row getAttributeDefinition(String meta, String typeMeta) {
		return getAttributeDefinition(new AliasMap(), meta, typeMeta);
	}

	public static Row getAttributeDefinition(IAliasMap map, String meta, String typeMeta) {
		Row row = getColumnDefinition(map, meta);
		String[] elementTypes = typeMeta.split(",");

		ColumnManager attributes = new ColumnManager();
		int i=0;
		for (IColumn column : row.getColumns()) {
			ColumnNode a = new ColumnNode(column.getName());
			a.mimic(column);
			a.setColumnType(IColumn.ColumnTypes.attribute);
			a.setElementType(elementTypes[i]);
			attributes.addColumn(a);			
			i++; 
		}
		return attributes;
		
	}
*/
	
	
}
