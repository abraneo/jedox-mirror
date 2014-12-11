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
package com.jedox.etl.core.node;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.util.Recoder;

/**
 * A simple manager class for forming a row of columns serving as input or output of components.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Row implements Cloneable {
	private List<IColumn> columns = new LinkedList<IColumn>();
	private Hashtable<String, IColumn> lookup = new Hashtable<String, IColumn>();
	private Hashtable<Integer,String> withoutAliasNames;
	private Log log;
	private String name;
	private boolean optimizeMemory;

	
	public Row() {
		withoutAliasNames = new Hashtable<Integer,String>();
		name = "default";
		optimizeMemory = false;
	}
	
	public Row(boolean optimizeMemory) {
		this.optimizeMemory = optimizeMemory;
		if (!optimizeMemory) {
			withoutAliasNames = new Hashtable<Integer,String>();
			name = "default";
		}
	}
	
	protected Log getLog() {
		if (log == null) {
			log = new MessageHandler(LogFactory.getLog(Row.class));
		}
		return log;
	}
	
	/**
	 * gets the name of this row
	 * @return the name of this row
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
		//process external aliases
		for (String key: map.getAliases()) {
			AliasMapElement m = map.getElement(key);
			int index = m.getColumn()-1;
			String name = m.getName();
			String defaultValue = m.getDefaultValue();
			if (index < size()) {
				IColumn c = getColumn(index);
				//check if we can apply alias name without breaking unique naming constraint
				IColumn existingColumn = getColumn(name);
				if (existingColumn == null || c == existingColumn) {
					if (defaultValue != null)
						c.setDefaultValue(defaultValue);
					if (!name.equals(m.getInternalDefaultName())) {//empty alias. just take default value and leave name as is.
						c.setName(name);
						lookup.put(key, c);
					}
				}
				else {
					getLog().warn("Cannot apply name "+name+" to column at index "+index+" because this name is already in use.");
				}
			}
			else
				getLog().warn("Column index "+(index+1)+ " out of range. Maximum index is "+size());
				//throw new RuntimeException("Index "+(index+1)+" of source column out of range. Maximum index is "+size())
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
			if (!optimizeMemory) 
				withoutAliasNames.put( withoutAliasNames.size(),name);
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
		for (IColumn newColumn : row.getColumns()) {
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
		if (!optimizeMemory) withoutAliasNames.clear();
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

	/**
	 * recodes the values of the columns using the given Recoder
	 * @param recoder the recoder to use
	 * @return this row now holding the values in a different encoding
	 */
	public Row recode(Recoder recoder) {
		for (IColumn s : getColumns()) {
			s.setValue(recoder.recode(s.getValueAsString()));
		}
		return this;
	}

	public Row clone() {
		Row clone = new Row();
		clone.withoutAliasNames = withoutAliasNames;
		clone.optimizeMemory = optimizeMemory;
		clone.setName(getName());
		for (IColumn source : columns) {
			Column target = new Column(source.getName());
			target.mimic(source);
			clone.addColumn(target);
		}
		return clone;
	}

	/**
	 * gets the original name of column before an aliasMap was applied.
	 * @param currentPos the position in the row
	 * @return the original Name.
	 */
	public String getOriginalName(int currentPos) {
		String origin = null;
		if (!optimizeMemory) {
			origin =  withoutAliasNames.get(currentPos);
		}
		if(origin==null) return "";
		return origin;
	}
	
	public void setOriginalName(int currentPos, String origin) {
		if (!optimizeMemory)
			withoutAliasNames.put(currentPos, origin);
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


}
