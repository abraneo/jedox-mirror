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
package com.jedox.etl.components.connection;

import java.io.File;
import java.io.StringWriter;
import java.sql.DatabaseMetaData;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.TableProcessor;
import com.jedox.etl.core.util.CSVWriter;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.NamingUtil;

public class RelationalConnection extends Connection implements IRelationalConnection {

	public enum MetadataSelectors {
		catalog, schema, table, column,procedure,procedureParameter
	}
	
	public enum MetadataFilters {
		catalogPattern, schemaPattern, tablePattern, tableTypes, columnPattern, procedurePattern
	}

	protected java.sql.Connection connection;
	private String quoteString = null;
	private static final Log log = LogFactory.getLog(RelationalConnection.class);

	protected String getDataDir() {
		return Settings.getInstance().getDataDir();
	}

	public String getDatabase() {
		String database = getDatabaseString();
		if (getHost() == null && FileUtil.isRelativ(database)) {
			String dir = getDataDir();
			database = dir + File.separator + database;
			database = database.replace("/", File.separator);
			database = database.replace("\\", File.separator);
		}
		return database;
	}

	protected String getDatabaseString() {
		return super.getDatabase();
	}

	protected String getConnectionString(String parameter) {
		String connectionString = getProtocol();
		if (getHost() != null) connectionString += "//"+getHost();
		if (getPort() != null) connectionString += ":"+getPort();
		if (getHost() != null) connectionString += "/";
		if (getDatabase() != null) connectionString += getDatabase();
		if (parameter.length() > 0) {
			connectionString += ";"+parameter;
		}
		return connectionString;
	}


	private String getConnectionString() {
		//is connection URL set explicitly?
		if (getURL() == null) {
			Properties properties = getConnectionParameters();
			Iterator<Object> i = properties.keySet().iterator();
			StringBuffer p = new StringBuffer();
			while (i.hasNext()) {
				String n = (String) i.next();
				String v = properties.getProperty(n);
				if (!n.startsWith(NamingUtil.internalPrefix())) p.append(n+"="+v+";");
			}
			String parameterString = p.toString();
			return getConnectionString(parameterString);
		}
		return getURL();
	}

	public String getConnectionUrl() throws RuntimeException {
		return getConnectionString();
	}

	protected java.sql.Connection connect2Relational() throws RuntimeException{
		String connectionString = getConnectionUrl();
		try {
			log.debug("Opening connection "+getName());
			//load driver class into memory.
			Class.forName(getDriver());
			java.sql.Connection connection = null;

			//if the connection has integrated authentication (case SQL server)
			String integratedSecurity = (String) getConnectionParameters().get("integratedSecurity");
			if(integratedSecurity!= null && integratedSecurity.equals("true")){
				log.info("Using windows authentication for connection " + getName() + ", make sure to add the required \"sqljdbc_auth.dll\" to your system path." );
				connection = DriverManager.getConnection(connectionString);
			}
			else{
				connection = DriverManager.getConnection(connectionString,getUser(), getPassword());
			}
			
			setTranactionIsolationLevel(connection);
			
			setAutoCommit(connection);
			
			log.debug("Connection "+getName()+" is open: "+connectionString);
			return connection;
		} catch (ClassNotFoundException cnfe) {
			throw new RuntimeException("Class not found: "+getDriver()+": "+cnfe.getMessage());
		} catch (SQLException sqle) {
			throw new RuntimeException("Failed to open Relational connection "+getName()+" with URL "+connectionString+". Please check your connection specification. "+sqle.getMessage());
		}
	}

	private void setAutoCommit(java.sql.Connection connection) throws RuntimeException, SQLException {
		
		// setting auto commit to connection if needed
		String autoCommit = "";
		try {
			autoCommit = (String) getParameter("autoCommit","");
		} catch (ConfigurationException e1) {}
		
		if(!autoCommit.isEmpty()){
			boolean i = false;

			if(!autoCommit.equals("true") && !autoCommit.equals("true"))
				throw new RuntimeException("Autocommit in components.xml can only have values \"true\" or \"false\".");
			i = Boolean.parseBoolean(autoCommit);

			log.info("Setting autoCommit for connection " + getName() + " to \"" +  i +"\" ." );				
			connection.setAutoCommit(i);
		}else{
			try{
				connection.setAutoCommit(false);
			}catch(Exception e){
				// some drivers does not support this, like in excel
				log.debug(e.getMessage());
			}
		}
	}

	private void setTranactionIsolationLevel(java.sql.Connection connection) throws RuntimeException, SQLException {
		
		// setting transaction Isolation Level to connection if needed
		String transactionIsolationLevel = "";
		try {
			transactionIsolationLevel = (String) getParameter("transactionIsolationLevel","");
		} catch (ConfigurationException e1) {}	
		
		if(!transactionIsolationLevel.isEmpty()){
			int i = 0;
			try{
				i = Integer.parseInt(transactionIsolationLevel);
			}catch(Exception e){
				throw new RuntimeException("Transaction Isolation Level in components.xml can only have values 0,1,2,4 and 8.");
			}
			if(i!=0 && i!=1 && i!=2 && i!= 4 & i!= 8){
				throw new RuntimeException("Transaction Isolation Level in components.xml can only have values 0,1,2,4 and 8.");
			}
			log.info("Setting transactionIsolationLevel for connection " + getName() + " to \"" +  i +"\"." );				
			connection.setTransactionIsolation(i);
		}
	}

	public synchronized java.sql.Connection open() throws RuntimeException {
		try {
			if ((connection == null) || (connection.isClosed())) {
				connection = connect2Relational();
				/*try{
					connection.setAutoCommit(false);
				}catch(Exception e){
					// some drivers does not support this, like in excel
					log.debug(e.getMessage());
				}*/
			}
		}
		catch (SQLException e) {
			throw new RuntimeException(e);
		}
		return connection;
	}

	public synchronized void commit() throws RuntimeException {
		try {
			if ((connection != null) && (!connection.isClosed())) {
				connection.commit();
			}
		}
		catch (SQLException e) {
			throw new RuntimeException(e.getMessage());
		}
	}

	public synchronized void close() {
		log.debug("Closing connection "+getName());
		try {
			if (!isKept() && (connection != null)) {
				connection.close();
				connection = null;
			}
		}
		catch (Exception e) {};
	}

	public String getServerName() {
		return getDBType().split(":")[0];
	}

	public StringWriter getCatalogs() throws RuntimeException {
		log.info("Getting Catalogs from connection: "+getName());
		StringWriter out = new StringWriter();
		CSVWriter writer = new CSVWriter(out);
		writer.setQuote("");
		try {
			DatabaseMetaData meta = open().getMetaData();
			writer.write(meta.getCatalogs());
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get catalogs: "+e.getMessage());
		}
	}

	public StringWriter getSchemas() throws RuntimeException {
		log.info("Getting Schemas from connection: "+getName());
		StringWriter out = new StringWriter();
		CSVWriter writer = new CSVWriter(out);
		writer.setQuote("");
		try {
			DatabaseMetaData meta = open().getMetaData();
			writer.write(meta.getSchemas());
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get schemas: "+e.getMessage());
		}
	}

	public StringWriter getTables(String catalogPattern, String schemaPattern, String tablePattern, String types) throws RuntimeException{
		log.info("getting tables from connection: "+getName());
		if ("*".equals(catalogPattern)) catalogPattern = null;
		if ("*".equals(schemaPattern)) schemaPattern = null;
		if ("*".equals(tablePattern)) tablePattern = null;
		if ("*".equals(types)) types = null;
		String[] allTypes = {"TABLE", "VIEW", "SYSTEM TABLE", "GLOBAL TEMPORARY", "LOCAL TEMPORARY", "ALIAS", "SYNONYM"};
		String[] typesSelected = null;
		if (types == null) {
			typesSelected = allTypes;
		}
		else {
			typesSelected = types.split(",");
		}
		StringWriter out = new StringWriter();
		CSVWriter writer = new CSVWriter(out);
		writer.setQuote("");
		try {
			DatabaseMetaData meta = open().getMetaData();
			writer.write(meta.getTables(catalogPattern, schemaPattern, tablePattern, typesSelected));
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get tables for catalog "+catalogPattern+" schema "+schemaPattern+": "+e.getMessage());
		}
	}

	public StringWriter getColumns(String catalogPattern, String schemaPattern, String tablePattern, String columnPattern) throws RuntimeException{
		log.info("Getting Columns from connection: "+getName()+" and table: "+tablePattern);
		if ("*".equals(catalogPattern)) catalogPattern = null;
		if ("*".equals(schemaPattern)) schemaPattern = null;
		if ("*".equals(tablePattern)) tablePattern = null;
		if ("*".equals(columnPattern)) columnPattern = null;
		StringWriter out = new StringWriter();
		CSVWriter writer = new CSVWriter(out);
		writer.setQuote("");
		try {
			DatabaseMetaData meta = open().getMetaData();
			writer.write(meta.getColumns(catalogPattern, schemaPattern, tablePattern, columnPattern));
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get columns for catalog "+catalogPattern+" schema "+schemaPattern+" table "+tablePattern+": "+e.getMessage());
		}
	}

	public final IProcessor getProcessor(String query, Boolean onlyHeader, int size) throws RuntimeException {
		return getProcessor(query,null,onlyHeader,true,size);
	}
	
	protected ResultSet getResultSet(Statement stmt, String query) throws SQLException{
		stmt.execute(query);
		boolean found = false;
		ResultSet rs = stmt.getResultSet();
		if(rs!=null)
			found = true;
		
		while(!found && !((stmt.getMoreResults() == false) && (stmt.getUpdateCount() == -1))){
			ResultSet rsTemp = stmt.getResultSet();
			if(rsTemp!=null){
				rs = rsTemp;
				found = true;
			}
		}
		
		return rs;
	}

	public IProcessor getProcessor(String query, IAliasMap aliasMap, Boolean onlyHeader, Boolean ignoreInternalKey, int size) throws RuntimeException {
		try {
			Statement stmt = createStatement(size);			
			IProcessor processor = new TableProcessor(getName(),aliasMap,getResultSet(stmt, query),ignoreInternalKey);
			processor.setInfo(true, "extract");
			return processor;
		}
		catch (SQLException e) {
			throw new RuntimeException(e.getMessage());
		}
	}
	
	protected Statement createBufferedStatementInternal() throws SQLException, RuntimeException {
		return open().createStatement(java.sql.ResultSet.TYPE_FORWARD_ONLY,java.sql.ResultSet.CONCUR_READ_ONLY);
	}

	protected Statement createStatement(int size) throws SQLException, RuntimeException {
		Statement statement;
		switch (getFetchMode()) {
		case BUFFERED: {
			statement = createBufferedStatementInternal();
			//some databases (such as derby) complain, when fetchsize is greate setMaxRows...
			try{
				int s = (size <= 0) ? Integer.MAX_VALUE : size;
				statement.setFetchSize(Math.min(500,s));
			}catch(Exception e){
			    //using driver like  "MicoSoft Access Driver" or drivers from DSN drivers list
				log.info("Setting fetch size for connection "+getName() + " is not possible.");
			}
			break;
		}
		default: statement = open().createStatement();
		}
		statement.setMaxRows(size);
		return statement;
	}

	public String getDriver() {
		return super.getDriver();
	}

	public String getUser() {
		return super.getUser();
	}

	public String getPassword() {
		return super.getPassword();
	}

	public String getIdentifierQuote() throws RuntimeException {
		if (quoteString == null) {
			try {
				quoteString = open().getMetaData().getIdentifierQuoteString();
			} catch (SQLException e) {
				throw new RuntimeException(e);
			}
		}
		return quoteString;
	}

	public boolean autoCreateSchemata() {
		return getConnectionParameters().getProperty(NamingUtil.internal("createSchemata"), "false").equalsIgnoreCase("true");
	}

	public boolean isSingleton() {
		return getParameter().getProperty(NamingUtil.internal("singleton"), "false").equalsIgnoreCase("true");
	}

	public MetadataCriteria[] getMetadataCriterias() {
		String[] tableFilters = {"catalogPattern","schemaPattern","tablePattern","tableTypes"};
		String[] columnFilters = {"catalogPattern","schemaPattern","tablePattern","columnPattern"};
		String[] procedureFilters = {"catalogPattern","schemaPattern","procedurePattern"};
		String[] procedureParameterFilters = {"catalogPattern","schemaPattern","procedurePattern","columnPattern"};
		
		ArrayList<MetadataCriteria> criterias = new ArrayList<MetadataCriteria>();		
		for (MetadataSelectors s : MetadataSelectors.values()) {
			MetadataCriteria c = new MetadataCriteria(s.toString());
			switch (s) {
			case table: { c.setFilters(tableFilters); break; }
			case column: { c.setFilters(columnFilters);break; }
			case procedure: {c.setFilters(procedureFilters);break;}
			case procedureParameter: {c.setFilters(procedureParameterFilters);}
			}
			criterias.add(c);
		}
		return criterias.toArray(new MetadataCriteria[criterias.size()]);
	}	
	
	public String getMetadata(Properties properties) throws RuntimeException {
		String selector = properties.getProperty("selector");
		String catalogPattern = properties.getProperty("catalogPattern");
		String schemaPattern = properties.getProperty("schemaPattern");
		String tablePattern = properties.getProperty("tablePattern");
		String tableTypes = properties.getProperty("tableTypes");
		String columnPattern = properties.getProperty("columnPattern");
		String procedurePattern = properties.getProperty("procedurePattern");
	
		MetadataSelectors sel;
		try {
			sel = MetadataSelectors.valueOf(selector);
		}
		catch (Exception e) {
			throw new RuntimeException("Property 'selector' must be one of: "+getMetadataSelectorValues());
		}
		switch (sel) {
		case catalog: return getCatalogs().toString();
		case schema: return getSchemas().toString();
		case table: return getTables(catalogPattern,schemaPattern,tablePattern,tableTypes).toString();
		case column: return getColumns(catalogPattern,schemaPattern,tablePattern,columnPattern).toString();
		case procedure: return getProcedures(catalogPattern,schemaPattern,procedurePattern).toString();
		case procedureParameter: return getProcedureParameters(catalogPattern,schemaPattern,procedurePattern,columnPattern).toString();
		default: return null;
		}
	}

	public StringWriter getProcedureParameters(String catalogPattern, String schemaPattern, String procedurePattern, String columnPattern) throws RuntimeException {
		log.info("Getting procedure parameters from connection: "+getName()+" and procedure pattern: "+procedurePattern);
		if ("*".equals(catalogPattern)) catalogPattern = null;
		if ("*".equals(schemaPattern)) schemaPattern = null;
		if ("*".equals(procedurePattern)) procedurePattern = null;
		if ("*".equals(columnPattern)) columnPattern = null;
		StringWriter out = new StringWriter();
		CSVWriter writer = new CSVWriter(out);
		writer.setQuote("");
		try {
			DatabaseMetaData meta = open().getMetaData();
			writer.write(meta.getProcedureColumns(catalogPattern, schemaPattern, procedurePattern,columnPattern));
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get procedure parameters for catalog "+catalogPattern+" schema "+schemaPattern+" procedure "+procedurePattern+": "+e.getMessage());
		}
	}

	public StringWriter getProcedures(String catalogPattern, String schemaPattern, String procedurePattern) throws RuntimeException{
		log.info("Getting procedures from connection: "+getName()+" and procedure pattern: "+procedurePattern);
		if ("*".equals(catalogPattern)) catalogPattern = null;
		if ("*".equals(schemaPattern)) schemaPattern = null;
		if ("*".equals(procedurePattern)) procedurePattern = null;
		StringWriter out = new StringWriter();
		CSVWriter writer = new CSVWriter(out);
		writer.setQuote("");
		try {
			DatabaseMetaData meta = open().getMetaData();
			writer.write(meta.getProcedures(catalogPattern, schemaPattern, procedurePattern));
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get procedures for catalog "+catalogPattern+" schema "+schemaPattern+" procedure "+procedurePattern+": "+e.getMessage());
		}
	}

	public boolean isSchemaSupported () {
		return true;
	}
}
