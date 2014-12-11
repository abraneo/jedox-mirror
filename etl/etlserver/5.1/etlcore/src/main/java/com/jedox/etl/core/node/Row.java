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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.IFunction;

/**
 * A simple manager class for forming a row of columns serving as input or output of components.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Row implements Cloneable {
	private static final Log log = LogFactory.getLog(Row.class);
	private List<IColumn> columns = new LinkedList<IColumn>();
	private Hashtable<String, IColumn> lookup = new Hashtable<String, IColumn>();
	private String name = "";

	
	public Row() {
	}

	
	/**
	 * gets the name of this row
	 * @return the name of this row
	 *
	*/
	public String getName() {
		return name;
	}


	/**
	 * sets the name of this row
	 * @param name the name of this row to set
	 */
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * sets aliases from an external AliasMap for the columns present in this row. The columns then can be addressed by both their name and by their alias name.
	 * @param map
	 * @throws RuntimeException
	 */
	public void setAliases(IAliasMap map) throws RuntimeException {
		//check overall uniquness of names 
		boolean isApplicable = true;
		if (!map.isEmpty()) {
			List<String> names = new ArrayList<String>();
			for (String key : getColumnNames()) {
				names.add(key);
			}
			for (String key: map.getAliases()) {
				AliasMapElement m = map.getElement(key);
				int index = m.getColumn()-1;
				if (index < names.size()) {
					names.add(index, key);
					names.remove(index+1);
				}
			}
			Set<String> nameSet = new HashSet<String>();
			nameSet.addAll(names);
			if (nameSet.size() < names.size()) {
				isApplicable = false;
			}
		}	
		//process external aliases
		for (String key: map.getAliases()) {
			AliasMapElement m = map.getElement(key).clone();
			int index = m.getColumn()-1;
			if (index < size()) {
				IColumn c = getColumn(index);
				//check if we can apply alias name without breaking unique naming constraint
				IColumn existingColumn = getColumn(m.getName());
				if (existingColumn == null || c == existingColumn || isApplicable) {
					lookup.remove(c.getName());
					m.setOrigin(c.getName());
					c.setAliasElement(m); //this sets potentially a new name!
					lookup.put(c.getName(), c);
				}
				else {
					log.warn("Cannot apply name "+name+" to column at index "+index+" because this name is already in use.");
				}
			}
			else
				log.info("Column index "+(index+1)+ " out of range. Maximum index is "+size());
		}
	}

	/**
	 * adds a column to the end of this row
	 * @param column the column to add
	 */
	public void addColumn(IColumn column) {
		addColumn(columns.size(), column, column.getName());
	}

	/**
	 * internally adds a column at the given position with the given name as lookup. If there is already a column present with this name, the column is rejected.
	 * @param pos the position to insert the column in the row
	 * @param column the column to add
	 * @param name the name to add the column with
	 */
	protected void addColumn(int pos, IColumn column, String name) {
		//ensure that there are no columns with the same name.
		if (getColumn(name) == null) {
			//add column
			columns.add(pos,column);
			lookup.put(name,column);
		}
	}

	/**
	 * adds a column with the given name (possibly different to the columns name)
	 * @param column the column to add to the end of this row
	 * @param name the name to register this column with
	 */
	public void addColumn(IColumn column, String name) {
		addColumn(columns.size(), column, name);
	}

	/**
	 * inserts the column at the given position in this row
	 * @param pos the position to insert the column
	 * @param column the column to add
	 */
	public void addColumn(int pos, IColumn column) {
		addColumn(pos,column,column.getName());
	}

	/**
	 * adds multiple columns at once from another row.
	 * @param row the row holding the columns to add. order is preserved.
	 */
	public void addColumns(Row row) {
		addColumns(row.getColumns());
	}
	
	public <T extends IColumn> void addColumns(List<T> columns) {
		for (T newColumn : columns) {
			addColumn(newColumn);
		}
	}

	/**
	 * internally removes a column from the row
	 * @param column the column to remove
	 * @param name the name to remove it from the registry
	 */
	protected void removeColumn(IColumn column, String name) {
		columns.remove(column);
		lookup.remove(name);
	}

	/**
	 * removes a column from this row
	 * @param column the column to remove
	 */
	public void removeColumn(IColumn column) {
		removeColumn(column,column.getName());
	}

	/**
	 * removes a column from this row
	 * @param name the name, under which the column to be removed is registered
	 */
	public void removeColumn(String name) {
		IColumn column = getColumn(name);
		if (column != null)
			removeColumn(column,name);
	}

	/**
	 * gets a column by its position in the row
	 * @param id the position of the column in the row
	 * @return the column at this position
	 */
	public IColumn getColumn(int id) {
		return columns.get(id);
	}

	/**
	 * gets a column by name
	 * @param name the name, under which the column is registered
	 * @return the column
	 */
	public IColumn getColumn(String name) {
		return lookup.get(name);
	}

	/**
	 * gets all columns of this row
	 * @return a vector holding the columns
	 */
	public List<IColumn> getColumns() {
		return columns;
	}

	/**
	 * clears this row removing all columns
	 */
	public void clear() {
		columns.clear();
		lookup.clear();
	}

	/**
	 * determines whether this row contains a column registered with a given name
	 * @param name the name the column is registered with
	 * @return true, if there is a column registered with the given name
	 */
	public boolean containsColumn(String name) {
		return getColumn(name) != null;
	}

	/**
	 * gets the number of columns registered in this row.
	 * @return the number of columns present.
	 */
	public int size() {
		return columns.size();
	}

	/**
	 * gets the positional index of a specific column
	 * @param column the column the get the index for
	 * @return the index / position of the column in the row.
	 */
	public int indexOf(IColumn column) {
		return columns.indexOf(column);
	}

	public Row clone() {
		Row clone = new Row();
		clone.setName(getName());
		for (IColumn source : columns) {
			IColumn target;
			try {
				if (source instanceof IFunction) { //special treatment of functions, since we cannot clone them straightforward. we just take them as input for a new coordinatenode. However this case SHOULD NOT HAPPEN, since functions should never be in a row directly.
					target = ColumnNodeFactory.getInstance().createCoordinateNode(source.getName(), source);
				} else {
					target = source.getClass().newInstance();
					target.mimic(source);
					//set constant value if defined.
					if (!source.isEmpty()) target.setValue(source.getValue());
				}
				clone.addColumn(target);

			} catch (Exception e) {
				log.error("Error cloning column "+source.getName()+": "+e.getMessage());
			}
		}
		return clone;
	}

	public List<String> getColumnNames() {
		List<String> result = new ArrayList<String>();
		for (IColumn c : getColumns()) {
			result.add(c.getName());
		}
		return result;
	}

	public List<String> getColumnValues() {
		List<String> result = new ArrayList<String>();
		for (IColumn c : getColumns()) {
			result.add(c.getValueAsString());
		}
		return result;
	}
	
	public <T extends IColumn> List<T> getColumns(Class<T> clazz) {
		List<T> result = new ArrayList<T>();
		for (IColumn c : getColumns()) {
			if (clazz.isAssignableFrom(c.getClass())) result.add(clazz.cast(c));
		}
		return result;
	}
	
	public <T extends IColumn> T getColumn(String name, Class<T> clazz) {
		IColumn c = getColumn(name);
		if (c != null && clazz.isAssignableFrom(c.getClass())) return clazz.cast(c);
		return null;
	}
	
	public <T extends IColumn> T getColumn(int pos, Class<T> clazz) {
		IColumn c = getColumn(pos);
		if (c != null && clazz.isAssignableFrom(c.getClass())) return clazz.cast(c);
		return null;
	}


}
