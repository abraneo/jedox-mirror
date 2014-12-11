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
package com.jedox.etl.core.persistence.generic;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.IColumn.ColumnTypes;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.TableProcessor;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.SQLUtil;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.IPersistence;
import com.jedox.etl.core.persistence.generic.module.*;

/**
 * Abstract base class for relational persistence back-ends, sharing all functionality common to specialized relational back-ends.
 * This class is used to provide {{@link com.jedox.etl.core.source.ISource datasources} with the capability to temporary store their data internally for further processing.
 * Also permanent {@link Datastore Datastores} use this back-end adapter.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class GenericPersistor implements IPersistence {

	private static final Log log = LogFactory.getLog(GenericPersistor.class);
	private MessageHandler aggLog = new MessageHandler(log);
	private Connection connection = null;
	protected LinkedList<String> schemas = new LinkedList<String>();
	protected boolean isShutDown = false;
	private boolean plainNames = false;
	private TypeConversionUtil typeConverter = new TypeConversionUtil();
	private MetadataModule metadataModule;
	private InsertModule insertModule;
	private UpdateModule updateModule;
	private DeleteModule deleteModule;
	private int bulkSize = 100;
	private boolean isLogging = false; 
	
	protected abstract Connection setupConnection() throws RuntimeException;

	/**
	 * gets the syntax for the auto-increment primary key generation.
	 * @return the syntax string (back-end dependent)
	 */
	protected abstract String getGeneratedKeySyntax();

	protected abstract Hashtable<String, String> getLookUp();

	/**
	 * hook for the creation and use of triggers. E.g. Oracle needs this for auto-increment primary key generation using sequences and insert triggers.
	 * @param persistentName the persistent name (schema and table)
	 * @param columnName the name of the column
	 */
	protected void createInsertTrigger(String persistentName, String columnName) throws RuntimeException {
		//override this in specific databases if needed. e.g. Oracle needs this
	}
	
	/**
	 * gets the persistence log
	 * @return the persistence log
	 */
	protected Log getLog() {
		return log;
	}

	/**
	 * gets the number of max rows acceptable by Statement#setMaxRows(Integer);
	 * @return number of max rows acceptable
	 */
	protected int getMaxRows() {
		return Integer.MAX_VALUE;
	}

	private void checkModule(PersistenceModule module) throws RuntimeException {
		if (module == null)
			getConnection();
	}

	protected MetadataModule getMetadataModule() throws RuntimeException {
		checkModule(metadataModule);
		return metadataModule;
	}

	protected InsertModule getInsertModule() throws RuntimeException {
		checkModule(insertModule);
		return insertModule;
	}

	protected UpdateModule getUpdateModule() throws RuntimeException {
		checkModule(updateModule);
		return updateModule;
	}

	protected DeleteModule getDeleteModule() throws RuntimeException {
		checkModule(deleteModule);
		return deleteModule;
	}

	public String escapeName(String name) throws RuntimeException {
		return usePlainNames() ? name : NamingUtil.escape(name,getMetadataModule().getQuote());
	}

	protected boolean hasConnection() {
		return connection != null;
	}

	public Connection getConnection() throws RuntimeException {
		try {
			if ((connection == null) || connection.isClosed()) {
				connection = setupConnection();
				metadataModule = new MetadataModule(connection.getMetaData(), getLookUp());
				insertModule = new InsertModule(connection);
				insertModule.setBulkSize(bulkSize);
				insertModule.setLogging(isLogging);
				updateModule = new UpdateModule(connection);
				updateModule.setBulkSize(bulkSize);
				updateModule.setLogging(isLogging);
				deleteModule = new DeleteModule(connection);
				deleteModule.setBulkSize(bulkSize);
				deleteModule.setLogging(isLogging);
			}
		}
		catch (SQLException e) {
			throw new RuntimeException("Lost connection: "+e.getMessage());
		}
		return connection;
	}

	protected void commit() {
		try {
			getConnection().commit();
		} catch (Exception e) {}
	}

	private void closeStatements(Locator locator) {
		try {
			String persistentName = getPersistentName(locator);
			getInsertModule().closeStatement(persistentName);
			getUpdateModule().closeStatement(persistentName);
			getDeleteModule().closeStatement(persistentName);
		}
		catch (Exception e) {
			log.warn(e.getMessage());
		}
	}

	public void commit(Locator locator) {
		closeStatements(locator);
		commit();
	}


	public void copyTable(Locator source, Locator target, Row columnDefinition) throws RuntimeException {
		String sourceName = getPersistentName(source);
		String targetName = getPersistentName(target);
		StringBuffer columns = new StringBuffer();
		for (IColumn c : columnDefinition.getColumns()) {
			if (!c.getName().equals(NamingUtil.internalKeyName()))
				columns.append(escapeName(c.getName())+",");
		}
		columns.deleteCharAt(columns.length()-1);
		try {
			createTable(target, columnDefinition);
			Statement stm = getConnection().createStatement();
			String sql = "INSERT INTO "+targetName+" ("+columns.toString()+") SELECT "+columns.toString()+" FROM "+sourceName;
			log.debug("Starting to copy data from "+source+" to "+target);
			stm.execute(sql);
			log.debug("The number of updated row from \""+ sql +"\" is " + stm.getUpdateCount());
			stm.close();
		}
		catch (SQLException e) {
			throw new RuntimeException("Failed to copy data from "+source+" to "+"target: "+e.getMessage());
		}
	}

	public void renameTable(Locator source, Locator target, Row columnDefinition, boolean recreateEmptySource) throws RuntimeException {
		copyTable(source,target,columnDefinition);
		if (recreateEmptySource)
			delete(source, new Row());
		else
			dropTable(source);
	}

	public void setPlainNames(boolean usePlain) {
		this.plainNames = usePlain;
	}

	/**
	 * determines if we are using plain (non-escaped) names for table and column naming. Not recommended for most use cases!
	 * @return true, we we use plain names
	 */
	protected boolean usePlainNames() {
		return plainNames;
	}

	/**
	 * builds the persistent name for a storage object in the back-end
	 * @param schema the name of the schema to use. if null, the default schema is used.
	 * @param table the name of the table to use
	 * @return the persistent name.
	 */
	protected String buildPersistentName(String schema, String table) {
		return (schema == null || "".equals(schema)) ? table : schema+"."+table;
	}

	public String getPersistentName(Locator locator) throws RuntimeException {
		String tableName = escapeName(locator.getName());
		String schemaName = getSchema(locator);
		return buildPersistentName(schemaName,tableName);
	}

	private List<String> getColumnNames(Row row, boolean includeKey) throws RuntimeException {
		List<String> names = new ArrayList<String>();
		//insert for all columns
		for (IColumn c : row.getColumns()) {
			names.add(escapeName(c.getName()));
		}
		if (!includeKey) {
			//do not insert for auto generated key
			names.remove(escapeName(NamingUtil.internalKeyName()));
		}
		return names;
	}

	/**
	 * creates a SQL-PreparedStatement with the syntax for a given locator and columnDefintion
	 * @param locator the locator addressing the back-end object
	 * @param columnDefinition the columns to use in the insert.
	 * @return the PreparedStatement ready to use for insertion.
	 * @throws SQLException
	 */
	protected PreparedStatement createInsertStatement(Locator locator, Row columnDefinition) throws RuntimeException {
		return getInsertModule().createInsertStatement(getPersistentName(locator), getColumnNames(columnDefinition,false));
	}

	/**
	 * gets a SQL-PreparedStatement with the syntax for a given locator and columnDefintion. If the back-end table does not yet exist, it is created. Existing statements for a given locator are reused.
	 * @param locator the locator addressing the back-end object
	 * @param columnDefinition the columns to use in the insert.
	 * @return the PreparedStatement ready to use for insertion.
	 * @throws SQLException
	 */
	protected PreparedStatement getInsertStatement(Locator locator, Row row) throws RuntimeException {
		PreparedStatement insert = getInsertModule().getStatement(getPersistentName(locator));
		if (insert == null) {
			insert = getTable(locator,row);
		}
		return insert;
	}

	/**
	 * creates a statement for the deletion of values.  Existing statements for a given locator are reused.
	 * @param locator the locator addressing the back-end object
	 * @param row the columns to use as delete criteria
	 * @return the PreparedStatement ready to delete.
	 * @throws RuntimeException
	 */
	protected PreparedStatement getDeleteStatement(Locator locator, Row row) throws RuntimeException {
		PreparedStatement delete = getDeleteModule().getStatement(getPersistentName(locator));
		if (delete == null) {
			delete = getDeleteModule().createStatement(getPersistentName(locator),getColumnNames(row,true));
		}
		return delete;
	}

	/**
	 * creates a statement for the update of values.  Existing statements for a given locator are reused.
	 * @param locator the locator addressing the back-end object
	 * @param keys the columns to use as keys to identify the rows to update
	 * @param toSet the columns, where the values are to set.
	 * @return the preparedStatement ready to use for update
	 * @throws RuntimeException
	 */
	protected PreparedStatement getUpdateStatement(Locator locator, Row keys, Row toSet) throws RuntimeException {
		PreparedStatement update = getUpdateModule().getStatement(getPersistentName(locator));
		if (update == null) {
			List<Boolean> add = new ArrayList<Boolean>();
			for (int i=0; i<toSet.size(); i++) {
				if (toSet.getColumn(i).getColumnType().equals(ColumnTypes.sum))
					add.add(Boolean.TRUE);
				else if (toSet.getColumn(i).getColumnType().equals(ColumnTypes.last))
					add.add(Boolean.FALSE);
				else {
					log.warn("Aggregation '"+toSet.getColumn(i).getColumnType().toString()+"' not supported in legacy persistence layer. Updating in mode 'last'.");
					add.add(Boolean.FALSE);
				}
			}
			update = getUpdateModule().createUpdateStatement(getPersistentName(locator),getColumnNames(keys,true),getColumnNames(toSet,false),add);
		}
		return update;
	}

	public boolean isPersistable(Locator locator, Row row) {
		try {
			createTable(locator, row);
			return true;
		}
		catch (RuntimeException e) {
			return false;
		}
	}

	/**
	 * fills the placeholder variables of a PreparedStatement with the value provided by a column
	 * @param statement the statement to fill
	 * @param column the column to take the value from
	 * @param pos the position of the placeholder variable to fill.
	 * @throws RuntimeException
	 */
	protected void fillStatement(PreparedStatement statement, IColumn column, int pos) throws RuntimeException {
		//cschw: null values cannot be set with setObject returns error. numeric null values are needed for denormalization
		//cschw: furthermore some jdbc driver such as postresql require to have the strict object-types as expected by the mapping
		try {
			Object value = typeConverter.convert(column);
			Integer sqlType = getMetadataModule().getSQLConstant(column.getValueType());
			if (value == null) {
				statement.setNull(pos, sqlType);
			} else
				statement.setObject(pos, value, sqlType);
		}
		catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}
	}
	
	private void handleException (String mode, Exception e, Row row, Locator locator) throws RuntimeException {
		// For Exceptions in insert, update or delete the name of table and columns is logged; for the first occurrence also the values are logged
		String tableName = " ";
		try {
			tableName = getPersistentName(locator);
		} catch (Exception e2) {}
		String message1 = "Failed to "+mode+" values in columns "+NamingUtil.removeIllegalWhiteSpaces(SQLUtil.enumNames(row.getColumnNames()))+" of table "+tableName+": "+e.getMessage();
		boolean isLogged = aggLog.willBeLogged(message1);
    	aggLog.warn(message1);
    	if (!isLogged) {
    		String message2=mode+" failure with value: "+NamingUtil.removeIllegalWhiteSpaces(SQLUtil.enumNames(row.getColumnValues()));
    		aggLog.warn(message2);
    	}	
	}
	
	public void insert(Locator locator, Row row) throws RuntimeException {
        //insert into existing table.
        PreparedStatement insertStatement = getInsertStatement(locator,row);
        if (insertStatement != null) {
            try {
                for (int i=0; i<row.size();i++) {
                	fillStatement(insertStatement,row.getColumn(i),i+1);
                }
                getInsertModule().addBatch(insertStatement);
                //insertStatement.execute();
                //log.debug("Update count (insert count) for the insert statment: " + insertStatement.getUpdateCount());
            } catch (Exception e) {
            	handleException("insert",e,row,locator);
            }
        }
    }
	
	public void delete(Locator locator, Row row) throws RuntimeException {
		
		List<String> names = new ArrayList<String>();
		List<String> values = new ArrayList<String>();
		List<Integer> lengths = new ArrayList<Integer>();
				
		// Columns of the relational table which are not in the row will not be part of the WHERE Statement 
		
		for(IColumn col:row.getColumns()){
			names.add(escapeName(col.getName()));
			values.add(col.getValueAsString());
			lengths.add(1);
		}
	
		String delete = getDeleteModule().createStatement(getPersistentName(locator), names, SQLUtil.quoteValues(values), lengths);
		Statement state;
		try {
			state = connection.createStatement();
			int linesDeleted = state.executeUpdate(delete);
			if (linesDeleted>0)
				log.info("Deleted " + linesDeleted + " records from table " + getPersistentName(locator) + ".");
		} catch (SQLException e) {
			throw new RuntimeException(e.getMessage());
		}
	}
	

	public void deleteLine(Locator locator, Row row) throws RuntimeException {
		//delete from existing table.
		PreparedStatement deleteStatement = getDeleteStatement(locator,row);
		if (deleteStatement != null) {
			try {
				for (int i=0; i<row.size();i++) {
					fillStatement(deleteStatement,row.getColumn(i),i+1);
				}
				getDeleteModule().addBatch(deleteStatement);
				//deleteStatement.executeUpdate();
				//log.debug("Update count (delete count) for the delete statment: " + deleteStatement.getUpdateCount());

			} catch (Exception e) {
            	handleException("delete",e,row,locator);
			}
		}
	}

	public void update(Locator locator, Row keys, Row toSet, Row row) throws RuntimeException {
		//update rows in existing table.
		if (toSet.size() > 0) { //only do something if there are values to set.
			if (keys.size() > 0) { //only update, if at least one key is specified. else do an insert.
				PreparedStatement updateStatement = getUpdateStatement(locator,keys,toSet);
				if (updateStatement != null) {
					try {
						//fill all columns to set
						for (int i=0; i<toSet.size();i++) {
							fillStatement(updateStatement,toSet.getColumn(i),i+1);
						}
						//fill the where condition
						for (int i=0; i<keys.size();i++) {
							fillStatement(updateStatement,keys.getColumn(i),toSet.size()+i+1);
						}
						if (updateStatement.executeUpdate() == 0) {//if no updates found do an insert.
							insert(locator,row);
						}
					} catch (Exception e) {
		            	handleException("update",e,row,locator);
					}
				}
			}
			else
				insert(locator,row);
		}
	}

	public void populate(Locator locator, IProcessor rows, boolean append) throws RuntimeException {
		//create a new table
		PreparedStatement insertStatement;
		if (!append)
			insertStatement = createTable(locator,rows.current());
		else
			insertStatement = getInsertStatement(locator, rows.current());
		MessageHandler aggLog = new MessageHandler(log);
		try {
			if (insertStatement != null) {
				Row row = rows.next();
				while (row != null) {
					String name = null;
					String value = null;
					try {
						for (int i=0; i<row.size();i++) {
							name = row.getColumn(i).getName();
							value = row.getColumn(i).getValueAsString();
							fillStatement(insertStatement,row.getColumn(i),i+1);
						}
						insertStatement.execute();
					} catch (Exception e) {
						aggLog.warn("Failed to insert value "+value+" of column "+name+" into table "+rows.getName()+": "+e.getMessage());
					}
					row = rows.next();
				}
			}
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to populate table: "+e.getMessage());
		}
		commit();
	}

	protected void createTableInternal(Locator locator, Row columnDefinition) throws RuntimeException {
		String persistentName = getPersistentName(locator);
		log.debug("creating internal table: "+persistentName);
		String generatedKey = null;
		StringBuffer create = new StringBuffer("CREATE TABLE "+persistentName+" (");
		for (int i=0;i<columnDefinition.size();i++) {
			IColumn c = columnDefinition.getColumn(i);
			String name = escapeName(c.getName());
			//test for generated key
			if (NamingUtil.internalKeyName().equalsIgnoreCase(c.getName())) {
				create.append(name +" " + getGeneratedKeySyntax() + ",");
				generatedKey = name;
			}
			else
				create.append(name+ " " + getMetadataModule().getDataType(c.getValueType(),c.getScale()) + ",");
		}
		create.deleteCharAt(create.length()-1);
		create.append(")");
		Statement statement;

		try {
			//now create new table
			statement = getConnection().createStatement();
			statement.execute(create.toString());
			statement.close();
			commit();
			log.debug("Created table "+persistentName+" by "+create.toString());
			// Info messages only in case of Loads but not for internal persistence
			// todo: find better general logic
			if (isLogging)
				log.info("Created relational table "+persistentName);
			if (generatedKey != null)
				createInsertTrigger(persistentName, generatedKey);
		} catch (SQLException e) {
			throw new RuntimeException("Failed to create table "+persistentName+": "+e.getMessage());
		}
	}

	public PreparedStatement createTable(Locator locator, Row columnDefinition) throws RuntimeException {
		getConnection();
		dropTable(locator);
		createTableInternal(locator, columnDefinition);
		PreparedStatement insert = createInsertStatement(locator, columnDefinition);
		return insert;
	}

	public PreparedStatement getTable(Locator locator, Row columnDefinition) throws RuntimeException {
		PreparedStatement insert;
		try {
			getTableResult(locator,1);
			insert = createInsertStatement(locator, columnDefinition);
		} catch (Exception e) {
			log.debug("Table "+getPersistentName(locator)+" not found. Creating it.");
			insert = createTable(locator, columnDefinition);
		}
		return insert;
	}

	public IProcessor getQueryResult(Locator locator, String query, int size) throws RuntimeException {
		Statement stm;
		//get Data
		try {
			stm = getConnection().createStatement(java.sql.ResultSet.TYPE_FORWARD_ONLY,java.sql.ResultSet.CONCUR_READ_ONLY);
			int sizemax = getMaxRows();
			if (size <= 0 || size>sizemax) size=sizemax;
			stm.setMaxRows(size);
			stm.setFetchSize(Math.min(1000,size));
			TableProcessor processor =  new TableProcessor(locator.getName(),null,stm.executeQuery(query),true);
			return processor;
		} catch (SQLException e) {
			throw new RuntimeException("Failed to retrieve data from internal table: "+query+", "+e.getMessage());
		}
	}

	public IProcessor getTableResult(Locator locator, int size) throws RuntimeException {
		return getQueryResult(locator, "select * from "+getPersistentName(locator), size);
	}

	private String getSchema(Locator locator) throws RuntimeException {
		if (!"".equals(locator.getPersistentSchema())) {
			String schema = escapeName(locator.getPersistentSchema());
			if (!schemas.contains(schema))
				addSchema(schema,false);
			return schema;
		}
		return null;
	}

	protected void createSchemaInternal(String name) throws SQLException, RuntimeException {
		Statement statement = getConnection().createStatement();
		statement.execute("CREATE SCHEMA "+name);
		statement.close();
	}

	public void dropSchema(Locator locator) throws RuntimeException {
		if (hasConnection()) { //do not open a new connection for schema drop, since we have no reason for cleanup
			String schema = escapeName(locator.getPersistentSchema());
			if (schemas.contains(schema)) {
				dropSchemaInternal(schema);
				schemas.remove(schema);
			}
		}
	}

	protected void dropSchemaInternal(String name) throws RuntimeException {
		Statement statement;
		try {
			statement = getConnection().createStatement();
			statement.execute("DROP SCHEMA "+name);
			statement.close();
		} catch (SQLException e) {
			log.debug("Failed to drop schema "+name+": "+e.getMessage());
		}
	}

	protected void addSchema(String name, boolean dropExisting) throws RuntimeException {
		if (dropExisting) {
			dropSchemaInternal(name);
		}
		try {
			schemas.add(name);
			createSchemaInternal(name);
		} catch (SQLException e) {
			log.debug("Failed to create schema for project: "+name);
		}
	}

	private void dropSchemas() throws RuntimeException {
		for (String schemaName : schemas) {
			dropSchemaInternal(schemaName);
		}
		schemas.clear();
	}

	public void dropTable(Locator locator) throws RuntimeException {
		if (hasConnection()) { //do not open a new connection for a drop operation, but ignore drop when we are not connected to a db.
			String persistentName = getPersistentName(locator);
			log.debug("dropping internal table: "+persistentName);
			try {
				closeStatements(locator);
				Statement statement = getConnection().createStatement();
				statement.execute("DROP TABLE "+persistentName);
				statement.close();
				getConnection().commit();
				if (isLogging)
					log.info("Dropped relational table "+persistentName);
			}
			catch (SQLException e) {
				log.debug(e.getMessage());
			}
		}
	}

	public synchronized void disconnect() {
		if (!isShutDown && hasConnection()) {
			try {
				dropSchemas();
				commit();
				isShutDown = true;
			}
			catch (Exception e) {
				log.warn(e.getMessage());
			};
		}
	}
	
	public void disconnectThread(){
	}
	
	public void setBulkSize(int bulkSize) {
		this.bulkSize = bulkSize;
	}
	
	public void setLogging(boolean isLogging) {
		this.isLogging = isLogging;	
	}

}
