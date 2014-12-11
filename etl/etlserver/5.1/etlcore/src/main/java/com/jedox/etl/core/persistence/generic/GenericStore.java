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
package com.jedox.etl.core.persistence.generic;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.IPersistence;
import com.jedox.etl.core.persistence.IStore;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.persistence.PersistorDefinition.AggregateModes;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.util.NamingUtil;

public class GenericStore implements IStore {

	private IPersistence persistence;
	private Locator temporaryLocator;
	private PersistorDefinition definition;
	private long rowCountInitial = 0;	
	private Modes mode;
	private static final Log log = LogFactory.getLog(GenericStore.class);

	public GenericStore(PersistorDefinition definition) throws RuntimeException {
		this.definition = definition;
		setMode(definition.getMode()); //CSCHW: we use a local mode here, since we may change it internally
		if (isRowCountInfo()) {
			rowCountInitial = getCurrentRowCount();			
			log.debug("Initial number of rows in datastore: " + rowCountInitial);
		}
		setWriteable();
		checkCompatibility();
	}

	private void checkCompatibility() {
		if (definition.getPrimaryKey()!=null || definition.getPrimaryKeyGeneration()!=null)
			log.warn("Settings for Primary Key not supported.");
		if (definition.getAggregateMode().equals(AggregateModes.row))
			log.warn("Aggregate Mode Row not supported.");
	}	

	private boolean isRowCountInfo() {
		return definition.getLocator().getManager().equals(ITypes.Loads);
	}
	
	private PersistorDefinition getDefinition() {
		return definition;
	}


	private Locator getTemporaryLocator() throws RuntimeException {
		if (temporaryLocator == null) {
			temporaryLocator = getDefinition().getLocator().clone().reduce().add(getDefinition().getLocator().getName()+NamingUtil.internal("temporary"));
			temporaryLocator.setPersistentTable(getPersistence().getPersistentName(getDefinition().getLocator())+NamingUtil.internal("temporary"));
		}
		return temporaryLocator;
	}

	private Row getInternalRow() throws RuntimeException {
		Row internalRow = new Row();
		if (!getDefinition().getColumnDefinition().containsColumn(NamingUtil.internalKeyName())) {
			RelationalNode key = ColumnNodeFactory.getInstance().createRelationalNode(NamingUtil.internalKeyName(),null);
			internalRow.addColumn(key);
		}
		internalRow.addColumns(getDefinition().getColumnDefinition());
		return internalRow;
	}

	private Row getExternalRow() throws RuntimeException {
		Row externalRow = new Row();
		if (getDefinition().doCreateKeyColumn() && !getDefinition().getColumnDefinition().containsColumn(NamingUtil.internalKeyName())) {
			RelationalNode key = ColumnNodeFactory.getInstance().createRelationalNode(NamingUtil.internalKeyName(),null);
			externalRow.addColumn(key);
		}
		externalRow.addColumns(getDefinition().getColumnDefinition());
		return externalRow;
	}
	
	protected IProcessor initProcessor(IProcessor processor) throws RuntimeException {
		 return getDefinition().getConnection().initProcessor(processor, Facets.HIDDEN);
	}
	
	private void checkDataCompatibility() throws RuntimeException {
		try {
			Row tableDefinition =  initProcessor(getPersistence().getQueryResult(getDefinition().getLocator(), "select * from "+getPersistence().getPersistentName(getDefinition().getLocator()), 1)).current();
			Row dataDefinition = getDefinition().getColumnDefinition();
			List<IColumn> dataColumns = new ArrayList<IColumn>();
			dataColumns.addAll(dataDefinition.getColumns());
			log.debug("Adapting column types for persistence in table "+getDefinition().getLocator().getPersistentTable());
			for (IColumn c : dataColumns) {
				if (!tableDefinition.containsColumn(c.getName())) {
					String msg="Data column '"+c.getName()+"' is not present in existing table '"+getDefinition().getLocator().getPersistentTable()+"'.";
					if (getDefinition().isLogging())
						log.warn(msg);
					throw new RuntimeException(msg);
					// dataDefinition.removeColumn(c);
				} else {
					c.setValueType(tableDefinition.getColumn(c.getName()).getValueType());
					log.debug("Column "+c.getName()+" exists, type is set to "+c.getValueType().getName());
				}
			}
			if (!getMode().equals(Modes.DELETE)) {
				for (IColumn c : tableDefinition.getColumns()) {
					if (!dataDefinition.containsColumn(c.getName())) {
						log.info("Existing column '"+c.getName()+"' in table '"+getDefinition().getLocator().getPersistentTable()+"' is not present as data column. Null values will be written on insert instead.");
					}
				}
			}
		}
		catch (RuntimeException e ) {
			log.debug("Table to check for compatibility not found / not yet created.");
		}
	}

	/**
	 * sets this Datastore writeable. Needs {@link #setDefinition(PersistorDefinition)} to be set.
	 * @throws RuntimeException
	 */
	private void setWriteable() throws RuntimeException {
		try {
			switch (getMode()) {
			case CREATE: { //create new backend table
				getPersistence().createTable(getDefinition().getLocator(), getExternalRow());
				break;
			}
			case TEMPORARY: { //create new backend table
				//drop if not already in use
				getPersistence().dropTable(getDefinition().getLocator());
				//get it - create it new if drop succeeded
				getPersistence().getTable(getDefinition().getLocator(), getExternalRow());
				//getPersistence().createTable(getDefinition().getLocator(), getExternalRow());
				break;
			}
			case UPDATE: {//create backend table if needed and delete all data from existing table
				checkDataCompatibility();
				getPersistence().getTable(getDefinition().getLocator(), getExternalRow());
				getPersistence().delete(getDefinition().getLocator(), new Row());
				break;
			}
			case DELETE: {
				checkDataCompatibility();
				break;
			}
			default: { //ensure that there is a table
				checkDataCompatibility();
				getPersistence().getTable(getDefinition().getLocator(), getExternalRow());
			}
			}
			//check if we can change mode to FILL, which is faster, if no aggregation is specified.
			if (!getDefinition().doAggregate()) {
				Modes mode = getMode();
				if (mode.equals(Modes.CREATE) || mode.equals(Modes.ADD) || mode.equals(Modes.INSERT) || mode.equals(Modes.UPDATE)) {
					setMode(Modes.FILL);
				}
			}
			//create and fill temporary store if necessary
			switch (getMode()) {
			case FILL: break;
			case TEMPORARY: break;
			case DELETE: break;
			default: if (AggregateModes.bulk.equals(getDefinition().getAggregateMode())) {
				//check if source table already has an auto-generated key. rename table, if so
				if (initProcessor(getPersistence().getTableResult(getDefinition().getLocator(), 1)).current().containsColumn(NamingUtil.internalKeyName()))
					getPersistence().renameTable(getDefinition().getLocator(), getTemporaryLocator(), getInternalRow(), true);
				else //copy table content to temporary table having auto-generated primary key
					getPersistence().copyTable(getDefinition().getLocator(), getTemporaryLocator(), getInternalRow());
			}
			}
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	
	private Row getLogicalKeys() {
		return (getDefinition().getKeys().size() == 0) ? getDefinition().getColumnDefinition() : getDefinition().getKeys();
	}

	public void write() throws RuntimeException {
		switch (getMode()) {
		case DELETE: getPersistence().delete(getDefinition().getLocator(), getLogicalKeys()); break;
		case FILL: getPersistence().insert(getDefinition().getLocator(), getDefinition().getColumnDefinition()); break;
		case TEMPORARY: getPersistence().insert(getDefinition().getLocator(), getDefinition().getColumnDefinition()); break;
		default: {
			if (AggregateModes.bulk.equals(getDefinition().getAggregateMode())) //use temporary table approach for aggregation
				getPersistence().insert(getTemporaryLocator(), getDefinition().getColumnDefinition());
			else if (AggregateModes.row.equals(getDefinition().getAggregateMode())) { //do a per line adding via direct update
				getPersistence().update(getDefinition().getLocator(), getDefinition().getKeys(), getDefinition().getToSet(),getDefinition().getColumnDefinition());
			}
			else {	
				//assume data is already aggregated and do a simple insert in original table
				getPersistence().insert(getDefinition().getLocator(), getDefinition().getColumnDefinition());
			}
		}
		}
	}

	private String getPrefix(RelationalNode column) {
		switch (column.getRole()) {
		case last: return "p.";
		case first: return "k.";
		default: return "a.";
		}
	}

	private String getColumnName(IColumn column) throws RuntimeException {
		return getPersistence().escapeName(column.getName())+",";
	}

	private String getSelectName(RelationalNode column) throws RuntimeException {
		String aggSelectName = getPrefix(column) + getPersistence().escapeName(NamingUtil.internal(column.getName()))+",";
		switch(column.getRole()) {
		case sum: return aggSelectName;
		case min: return aggSelectName;
		case max: return aggSelectName;
		case count: return aggSelectName;
		case avg: return aggSelectName;
		default: return getPrefix(column) + getPersistence().escapeName(column.getName())+",";
		}
	}

	private String getAggregateName(RelationalNode column) throws RuntimeException {
		String aggName = column.getRole().toString()+"("+getPersistence().escapeName(column.getName())+") as "+getPersistence().escapeName(NamingUtil.internal(column.getName()))+",";
		switch (column.getRole()) {
		case sum: return aggName;
		case min: return aggName;
		case max: return aggName;
		case count: return aggName;
		case avg: return aggName;
		case key: return getPersistence().escapeName(column.getName())+",";
		default: return "";
		}
	}

	private String getGroupByName(RelationalNode column) throws RuntimeException {
		switch (column.getRole()) {
		case key: return getPersistence().escapeName(column.getName())+",";
		default: return "";
		}
	}

	private void copyDataAggregated(Locator source, Locator target) throws RuntimeException {
		String sourceName = getPersistence().getPersistentName(source);
		String targetName = getPersistence().getPersistentName(target);
		String keyName = getPersistence().escapeName(NamingUtil.internalKeyName());
		String maxID = getPersistence().escapeName(NamingUtil.internal("maxID"));
		String minID = getPersistence().escapeName(NamingUtil.internal("minID"));
		StringBuffer columns = new StringBuffer(" ");
		Row row = getDefinition().getColumnDefinition();
		for (RelationalNode c : row.getColumns(RelationalNode.class)) {
			columns.append(getColumnName(c));
		}
		StringBuffer selects = new StringBuffer(" ");
		for (RelationalNode c : row.getColumns(RelationalNode.class)) {
			selects.append(getSelectName(c));
		}
		StringBuffer aggregates = new StringBuffer(" ");
		for (RelationalNode c : row.getColumns(RelationalNode.class)) {
			aggregates.append(getAggregateName(c));
		}
		StringBuffer groups = new StringBuffer(" ");
		for (RelationalNode c : row.getColumns(RelationalNode.class)) {
			groups.append(getGroupByName(c));
		}
		//separate columns to keep values and column to put / overwrite values
		Row puts = new Row();
		Row keeps = new Row();
		for (RelationalNode c : row.getColumns(RelationalNode.class)) {
			switch (c.getRole()) {
			case last: puts.addColumn(c); break;
			case first: keeps.addColumn(c); break;
			default:
			}
		}
		//build 2 separate joins for keeps (minID) and puts (maxID)
		StringBuffer putJoin = new StringBuffer(" ");
		StringBuffer keepJoin = new StringBuffer(" ");
		if (puts.size() > 0) {
			putJoin.append("inner join "+sourceName+" p on a."+maxID+" = p."+keyName);
			aggregates.append("max("+keyName+") as "+maxID+",");
		}
		if (keeps.size() > 0) {
			keepJoin.append("inner join "+sourceName+" k on a."+minID+" = k."+keyName);
			aggregates.append("min("+keyName+") as "+minID+",");
		}
		columns.setCharAt(columns.length()-1,' ');
		selects.setCharAt(selects.length()-1,' ');
		aggregates.setCharAt(aggregates.length()-1,' ');
		groups.setCharAt(groups.length()-1,' ');
		try {
			//TODO: cschw: MOVE query execution to generic persistence.
			Statement stm = getPersistence().getConnection().createStatement();
			String sql = "INSERT INTO "+targetName+" ("+columns.toString()+") SELECT "+selects.toString()+" FROM (SELECT "+aggregates.toString()+" FROM "+sourceName+" GROUP BY "+groups.toString()+") a "+putJoin.toString()+keepJoin.toString();
			stm.execute(sql);
			log.debug("The number of updated row from \""+ sql +"\" is " + stm.getUpdateCount());
			stm.close();
		}
		catch (SQLException e) {
			throw new RuntimeException("Failed to copy aggregated data from "+source+" to "+target+": "+e.getMessage());
		}
	}



	protected IPersistence getPersistence() throws RuntimeException {
		if (persistence == null) {
			persistence = PersistenceManager.getPersistence(getDefinition().getConnection());
			/*if(getDefinition().getChildrenMaps()!=null && !getDefinition().getChildrenMaps().isEmpty())
				persistence.setChildrenMaps(getDefinition().getChildrenMaps());*/
			persistence.setPlainNames(getDefinition().isPlain());
			persistence.setBulkSize(getDefinition().getBulkSize());
			persistence.setLogging(getDefinition().isLogging());
		}
		return persistence;
	}

	public void commit() throws RuntimeException {

		try {
			switch (getMode()) {
			case FILL: break;
			case DELETE: break;
			case TEMPORARY: break;
			default: if (AggregateModes.bulk.equals(getDefinition().getAggregateMode())) {
				//copy back aggregated from temporary store
				getPersistence().commit(getTemporaryLocator());
				getPersistence().delete(getDefinition().getLocator(), new Row());
				copyDataAggregated(getTemporaryLocator(),getDefinition().getLocator());
				getPersistence().dropTable(getTemporaryLocator());
			}
			}
			getPersistence().commit(getDefinition().getLocator());
			if (isRowCountInfo())
				log.info("Number of rows in table " + persistence.getPersistentName(getDefinition().getLocator()) + " changed from " + rowCountInitial + " to " +  getCurrentRowCount());
			else
				log.debug("Number of rows in datastore: "+getCurrentRowCount());
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	@Override
	public long getCurrentRowCount() {
		String tableName = "";
		try{
			tableName = getPersistence().getPersistentName(getDefinition().getLocator());
			PreparedStatement statement = getPersistence().getConnection().prepareStatement("select count(*) from "+tableName);
			ResultSet result = statement.executeQuery();
			result.next();
			long res = result.getLong(1);
			result.close();
			statement.close();
			getPersistence().getConnection().commit();
			return res;
		}catch(Exception e){
			log.debug("Could not count rows of table "+tableName+": "+e.getMessage());
			return 0;
		}
	}

	@Override
	public void close() throws RuntimeException {
		switch (getMode()) {
		case TEMPORARY: 
			getPersistence().dropTable(getDefinition().getLocator());
			getPersistence().disconnectThread();
		default: break;
		}
	}

	public void setMode(Modes mode) {
		this.mode = mode;
	}

	public Modes getMode() {
		return mode;
	}

}
