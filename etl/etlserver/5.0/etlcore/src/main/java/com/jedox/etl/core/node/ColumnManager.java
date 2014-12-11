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

package com.jedox.etl.core.node;

import java.util.Hashtable;
import java.util.Iterator;

import com.jedox.etl.core.node.IColumn;

/**
 * Manager Class for {@link IColumn Columns} of different types / roles. 
 * <br>
 * Based on {@link Row} it extends the functionality to manage columns acting in different roles and efficient access / restriction to columns used in these specific roles.
 * Additionally the creation and improved management of specialized Column Classes for Coordinates, Values, Levels, etc. is supported.  
 * <br>
 * The order of the nodes added is preserved. This is e.g. used for organizing the coordinates (for cubes) and levels (for dimensions) in the later load. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ColumnManager extends Row {

	private Hashtable<IColumn.ColumnTypes, Row> columnTypes = new Hashtable<IColumn.ColumnTypes, Row>();
	
	
	public ColumnManager() {
		super();
	}
	
	/**
	 * adds a column to a role list
	 * @param column the column to add
	 * @param type the role list to add to
	 */
	private void addRole(IColumn column, IColumn.ColumnTypes type) {
		Row list = columnTypes.get(type);
		if (list == null) {
			list = new Row();
			columnTypes.put(type, list);
		}
		//remove any node of this type with this name and add the new node on this position.
		list.addColumn(remove(list,getColumn(list,column.getName())),column, column.getName());
	}

	
	/**
	 * Adds a {@link IColumn} to the manager replacing an existing column
	 * @param column the column to add
	 * @param type the explicit type / role of the column, which may differ from the role defined by the column itself.
	 */
	 public void addColumn(IColumn column, IColumn.ColumnTypes type) {
		//add to global row but do not add role
		super.addColumn(remove(this,getColumn(this,column.getName())),column,column.getName());
		//add to roles rows
		addRole(column, type); 
	}
 
 	/**
	 * adds a column with the given name (possibly different to the columns name) replacing an existing column
	 * @param column the column to add to the end of this row 
	 * @param name the name to register this column with
	 */
	public void addColumn(IColumn column, String name) {
		addColumn(remove(this,getColumn(this,name)), column, name);
	}
	
	/**
	 * inserts the column at the given position in this row replacing an existing column
	 * @param pos the position to insert the column
	 * @param column the column to add
	 */
	public void addColumn(int pos, IColumn column) {
		remove(this,getColumn(this,column.getName()));
		addColumn(Math.min(this.size(),pos),column,column.getName());
	}
	
	/**
	 * insert a column at a given position.  
	 */
	protected void addColumn(int pos, IColumn column, String name) {
		super.addColumn(pos, column, name);
		addRole(column, column.getColumnType());
	} 
	
	/**
	 * removes a column with all of its roles
	 */
	protected void removeColumn(IColumn column, String name) {
		super.removeColumn(column, name);
		for (Row list : columnTypes.values()) {
			list.removeColumn(column);
		}
	}
	
	/**
	 * Finds a specific column by name in a given row / role list
	 * @param l the list of columns to search for
	 * @param name the name of the column to be searched for.
	 * @return the column or null
	 */
	private IColumn getColumn(Row l, String name) {
		for (int i=0; i<l.size(); i++) {
			IColumn cn = l.getColumn(i);
			if (cn.getName().equals(name)) return cn;
		}
		return null;
	}
	
	
	/**
	 * Removes a column from a row / role list 
	 * @param l the list of columns
	 * @param cn the column
	 * @return the position of the removed column, the number of columns in the list if the column is null, -1 if the column is not null but not found in the list 
	 */
	private int remove (Row l, IColumn cn) {
		int pos = l.size();
		if (cn != null) {
			pos = l.indexOf(cn);
			l.removeColumn(cn);
		}
		return pos;
	}

	/**
	 * Gets all registered columns of a certain type / role.
	 * @param type the type of the columns to get
	 * @return a Row of Columns or an empty Row
	 */
	public Row getColumnsOfType(IColumn.ColumnTypes type) {
		Row l = columnTypes.get(type);
		if (l == null) l = new Row();
		return l;
	}

	
	/**
	 * Searches a column within the namespace of a certain type.
	 * @param type the role / namespace of the column
	 * @param name the name of the column
	 * @return
	 */
	public IColumn getColumnOfType(IColumn.ColumnTypes type, String name) {
		Row columns = getColumnsOfType(type);
		return getColumn(columns, name);
	}
	
	/**
	 * Removes all columns of the given type from this manager
	 * @param the types of the columns
	 */
	private void clearColumnsOfType(IColumn.ColumnTypes type) {
			Row l = columnTypes.get(type);
			if (l != null) l.clear();
	}
	
	/**
	 * Removes all columns of all types from this manager
	 *
	 */
	public void clear() {
			Iterator<IColumn.ColumnTypes> i = columnTypes.keySet().iterator();
			while (i.hasNext()) {
				clearColumnsOfType(i.next());
			}
			super.clear();
	}
	
	/**
	 * Creates and adds a {@link LevelNode} to this manager
	 * @param name the name of the level
	 * @param inputName the name of the input column
	 * @return the LevelNode
	 */
	public LevelNode addLevel(String name, String inputName) {
		LevelNode n = new LevelNode(name); 
		Column c = new Column(inputName);
		n.setInput(c);
		addColumn(n);
		return n;
	}
	
	/**
	 * Adds an attribute to an existing {@link LevelNode}
	 * @param levelName the name of the LevelNode
	 * @param attributeName the name of the attribute 
	 * @param inputName the name of the input Column for this attribute
	 * @return the Attribute as ColumnNode
	 */
	public ColumnNode addAttribute(String levelName, String attributeName, String inputName, String attributeType) {
		LevelNode n = (LevelNode) getColumnsOfType(IColumn.ColumnTypes.level).getColumn(levelName);
		Column column = new Column(inputName);
		ColumnNode a = n.setAttribute(attributeName, column);
		a.setColumnType(IColumn.ColumnTypes.valueOf(attributeType));
		addColumn(a,IColumn.ColumnTypes.valueOf(attributeType));
		return a;
	}
	
	/**
	 * Creates and adds a {@link ValueNode} to this manager. Before loading values into a cube, they have to be normalized. Normalization works via an additional coordinate.
	 * @param target the name of the target {@link CoordinateNode} for value normalization. The target gets the name of the ValueNode as input, when this ValueNode is active in the output. 
	 * @param name the name of the ValueNode
	 * @param inputName the name of the input Column for this ValueNode
	 * @return the ValueNode
	 */
	public ValueNode addValue(String target, String name, String inputName) {
		ValueNode n = new ValueNode(name,target);
		Column c = new Column(inputName);
		n.setInput(c);
		addColumn(n);
		return n;
	}
	
	/**
	 * Creates and adds a {@link CoordinateNode} to this manager. 
	 * @param name the name of the CoordinateNode
	 * @param inputName the name of the input Column for this CoordinateNode
	 * @return the CoordinateNode
	 */
	public CoordinateNode addCoordinate(String name, String inputName) {
		CoordinateNode n = new CoordinateNode(name);
		Column c = new Column(inputName);
		n.setInput(c);
		addColumn(n);
		return n;
	}
	
	/**
	 * Creates and adds an {@link AnnexNode} to this manager
	 * @param name the name of the AnnexNode
	 * @param inputName the name of the input Column for this AnnexNode
	 * @return the AnnexNode
	 */
	public AnnexNode addAnnex(String name, String inputName) {
		AnnexNode n = new AnnexNode(name);
		Column c = new Column(inputName);
		n.setInput(c);
		addColumn(n);
		return n;
	}
	
	/**
	 * Adds existing {@link IColumn Columns} as {@link CoordinateNode CoordinateNodes} to this manager
	 * @param row the Row hosting the existing columns
	 */
	public void addCoordinates(Row row) {
		for (IColumn col : row.getColumns()) {
			CoordinateNode c = addCoordinate(col.getName(),col.getName());
			c.setColumnType(col.getColumnType());
			c.setValueType(col.getValueType());
			// c.setFallbackDefault(col.getDefaultValue());
		}
	}
}
