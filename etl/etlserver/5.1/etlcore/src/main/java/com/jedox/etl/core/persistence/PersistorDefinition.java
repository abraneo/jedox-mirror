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
package com.jedox.etl.core.persistence;

import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode;
import com.jedox.etl.core.node.RelationalNode.UpdateModes;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import java.util.HashMap;

/**
 * Definition class for {@link Datastore datastores}, which holds all information to configure writeable Datastores
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class PersistorDefinition {
	
	public enum AggregateModes {
		none, bulk, row
	}

	private IRelationalConnection connection;
	private Locator locator;
	private Modes mode;
	private HashMap<String,UpdateModes> roles = new HashMap<String,UpdateModes>();
	private HashMap<String,Class<?>> types = new HashMap<String,Class<?>>();
	private AggregateModes aggregateMode = AggregateModes.none; 
	private Row input = new Row();
	private Row definition;
	private Row keys;
	private Row toSet;
	private Row toKeep;
	private String primaryKey;
	private String primaryKeyGeneration;
	//cschw: the following properties are not used in hibernate interface
	private boolean isPlain = false;
	private boolean createKeyColumn = true;
	private ISource directSource;
	private int bulkSize = 100;
	private boolean isLogging = false;
	
	//private HashMap<String, Map<String, IElement[]>> childrenMaps = new HashMap<String, Map<String,IElement[]>>();
	
	public PersistorDefinition() {}
	
	public PersistorDefinition(Locator locator, IRelationalConnection connection) {
		this.locator = locator;
		this.connection = connection;
		this.createKeyColumn = false; //only applies for generic persistence
		setMode(Modes.TEMPORARY);
	}
	

	/**
	 * gets the connection for the Datastore to use
	 * @return the connection
	 */
	public IRelationalConnection getConnection() {
		return connection;
	}

	/**
	 * sets the connection for the Datastore to use
	 * @param connection the connection
	 */
	public void setConnection(IRelationalConnection connection) {
		this.connection = connection;
	}

	/**
	 * gets the locator of the Datastore.
	 * @return the locator of the Datastore
	 */
	public Locator getLocator() {
		return locator;
	}

	/**
	 * gets the locator of the Datastore. The locator should have set a {{@link Locator#setPersistentSchema(String)}
	 * @param locator the locator to use to identify and persist the datastore
	 */
	public void setLocator(Locator locator) {
		this.locator = locator;
	}

	/**
	 * gets the Write-Mode the Datastore should conform. This write mode performs as if writing in an olap cube to enable the synchronization of OLAP and relational storage.
	 * @return the write mode
	 */
	public Modes getMode() {
		return mode;
	}

	/**
	 * sets the Write-mode the Datastore should conform. This write mode performs as if writing in an olap cube to enable the synchronization of OLAP and relational storage.
	 * @param mode the write mode
	 */
	public void setMode(Modes mode) {
		this.mode = mode;
	}

	/**
	 * gets the mapping of column names to their roles in writing to the datastore.
	 * @return the map of roles
	 */
	public HashMap<String, UpdateModes> getRoles() {
		return roles;
	}

	/**
	 * sets the mapping of column names to their roles in writing to the datastore.
	 * @param roles the map of roles
	 */
	public void setRoles(HashMap<String, UpdateModes> roles) {
		this.roles = roles;
	}

	/**
	 * set the role of a column in writing to the datastore
	 * @param name the name of the column
	 * @param role the role of this column to take
	 */
	public void setRole(String name, UpdateModes role) {
		getRoles().put(name, role);
	}

	/**
	 * gets the role of a column in writing to the datastore
	 * @param name the name of the column to get its role
	 * @return the role
	 */
	public UpdateModes getRole(String name) {
		return getRoles().get(name);
	}

	/**
	 * set the data type of a column in writing to the datastore
	 * @param name the name of the column
	 * @param type the data type of this column to take
	 */
	public void setType(String name, Class<?> type) {
		types.put(name, type);
	}

	/**
	 * gets the data type of a column in writing to the datastore
	 * @param name the name of the column to get its role
	 * @return the type
	 */
	public Class<?> getType(String name) {
		return types.get(name);
	}

	/**
	 * determines if in naming of schema, table and columns the internal names should be taken as is (true) or if they should be escaped to avoid conflicts with reserved words and forbidden characters in the back-end.
	 * @return true, if no escaping, false (default, recommended) otherwise
	 */
	public boolean isPlain() {
		return isPlain;
	}

	/**
	 * sets if in naming of schema, table and columns the internal names should be taken as is (true) or if they should be escaped to avoid conflicts with reserved words and forbidden characters in the back-end.
	 * @return true, if no escaping, false (default, recommended) otherwise
	 */
	public void setPlain(boolean isPlain) {
		this.isPlain = isPlain;
	}

	/**
	 * determines if the datastore should perform aggregation, which takes more time but ensures the general sync of olap-cube data and the datastore.
	 * @return true (default), if sync is to be guaranteed. false if data is just written in the store possible destroying the sync but faster.
	 */
	public boolean doAggregate() {
		//check if there is the required data for aggregation
		if (AggregateModes.none.equals(aggregateMode) || getKeys().size() == 0 || (getToSet().size() == 0 && getToKeep().size() == 0)) {
			return false;
		}
		return true;
	}

	/**
	 * determines if the datastore should contain an autogenerated primary key
	 * @return true, if so
	 */
	public boolean doCreateKeyColumn() {
		return createKeyColumn;
	}

	/**
	 * sets the flag for the datastore to contain an auto-generated primary key
	 * @param createKeyColumn the flag to set.
	 */
	public void setCreateKeyColumn(boolean createKeyColumn) {
		this.createKeyColumn = createKeyColumn;
	}

	/**
	 * gets the column definition (with their set column roles) as to used by the datastore
	 * @return the column definition
	 */
	public Row getColumnDefinition() {
		if (definition == null) {
			definition = new Row();
			for (int i=0; i<input.size(); i++) {
				IColumn c = input.getColumn(i);
				RelationalNode d = ColumnNodeFactory.getInstance().createRelationalNode(c.getName(), c);
				UpdateModes role = getRole(c.getName());
				Class<?> dataType = getType(c.getName());
				if (dataType != null)
					d.setValueType(dataType);
				if (role != null) {
					//convert update modes to column modes
					d.setRole(role);
				}
				definition.addColumn(d);
			}
		}
		return definition;
	}

	/**
	 * sets the input row from an underlying processor writing in this datastore.
	 * @param input the input row from the processor
	 */
	public void setInput(Row input) {
		this.input = input;
	}

	/**
	 * gets all columns acting as keys in writing to the datastore
	 * @return the key columns
	 */
	public Row getKeys() {
		if (keys == null) {
			keys = new Row();
			for (RelationalNode c : getColumnDefinition().getColumns(RelationalNode.class)) {
				UpdateModes role = c.getRole();
				switch (role) {
					case key: keys.addColumn(c); break;
					default: break;
				}
			}
		}
		return keys;
	}

	/**
	 * gets all columns, which are to be updated when writing in the datastore in mode update
	 * @return all data columns to update
	 */
	public Row getToSet() {
		if (toSet == null) {
			toSet = new Row();
			for (RelationalNode c : getColumnDefinition().getColumns(RelationalNode.class)) {
				UpdateModes role = c.getRole();
				switch (role) {
					case sum: toSet.addColumn(c);
					case min: toSet.addColumn(c);
					case max: toSet.addColumn(c);
					case last: toSet.addColumn(c);
					case count: toSet.addColumn(c);
					case avg: toSet.addColumn(c);
					default: break;
				}
			}
		}
		return toSet;
	}

	/**
	 * gets all columns, which are to be updated when writing in the datastore in mode update
	 * @return all data columns to update
	 */
	public Row getToKeep() {
		if (toKeep == null) {
			toKeep = new Row();
			for (RelationalNode c : getColumnDefinition().getColumns(RelationalNode.class)) {
				UpdateModes role = c.getRole();
				switch (role) {
					case first: toKeep.addColumn(c); break;
					default: break;
				}
			}
		}
		return toKeep;
	}


	/**
	 * gets the id of the datastore.
	 * @return the datastore id, either taken from {@link #setId(String)} or {@link #setLocator(Locator)}
	 */
	public String getId() {
		return getLocator().toString();
	}

	public void setPrimaryKey(String primaryKey) {
		this.primaryKey = primaryKey;
	}

	public String getPrimaryKey() {
		return primaryKey;
	}

	public void setPrimaryKeyGeneration(String primaryKeyGeneration) {
		this.primaryKeyGeneration = primaryKeyGeneration;
	}

	public String getPrimaryKeyGeneration() {
		return primaryKeyGeneration;
	}
	
	public void setAggregateMode(String aggregateMode) {
		this.aggregateMode = AggregateModes.valueOf(aggregateMode);
	}
	
	public AggregateModes getAggregateMode() {
		return aggregateMode;
	}

	/**
	 * @param childrenMaps the childrenMaps to set
	 */
	/*public void setChildrenMaps(HashMap<String, Map<String, IElement[]>> childrenMaps) {
		this.childrenMaps = childrenMaps;
	}*/

	/**
	 * @return the childrenMaps
	 */
	/*public HashMap<String, Map<String, IElement[]>> getChildrenMaps() {
		return childrenMaps;
	}*/

	/**
	 * gets the Source for direct access for Drillthrough
	 * @return the locator of the Datastore
	 */
	public ISource getDirectSource() {
		return directSource;
	}

	/**
	 * sets the Source for a direct access for Drillthrough. If set the Datastore will not get persisted
	 * @param locator the locator to use to identify and persist the datastore
	 */
	public void setDirectSource(ISource directSource) {
		this.directSource = directSource;
	}
	
	public String getPersistentName() {
		return getLocator().getPersistentName();
	}

	public void setBulkSize(int bulkSize) {
		this.bulkSize = bulkSize;
	}

	public int getBulkSize() {
		return bulkSize;
	}

	public void setLogging (boolean isLogging) {
		this.isLogging = isLogging;
	}

	public boolean isLogging() {
		return isLogging;
	}	
	
}
