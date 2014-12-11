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
package com.jedox.etl.core.persistence;

import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.extract.IRelationalExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.generic.GenericStore;
import com.jedox.etl.core.persistence.hibernate.HibernateStore;
import com.jedox.etl.core.persistence.hibernate.IPersistable;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.SQLUtil;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.Arrays;
import javax.persistence.*;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Datastores provide a permanent relational storage for data, which is intended for further use e.g. for Drillthrough operations on cubes. They provide means for synchronization with external data back-ends (such as OLAP-Cubes), when they are manipulated by the ETL-Server.
 * Additionally the output from arbitrary {@link com.jedox.etl.core.source.ISource datasources} may be persisted in Datastores for later data retrieval.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

@Entity
public class Datastore implements IStore, IPersistable {
	
	private class DatastoreProcessor extends Processor {
		
		private IProcessor sourceProcessor;
		private IRelationalConnection connection;
		
		public DatastoreProcessor(IProcessor sourceProcessor, IRelationalConnection connection) {
			this.sourceProcessor = sourceProcessor;
			this.connection = connection;
		}

		@Override
		protected boolean fillRow(Row row) throws Exception {
			return sourceProcessor.next() != null;
		}

		@Override
		protected Row getRow() {
			return sourceProcessor.current();
		}
		
		public void close() {
			connection.close();
		}
		
	}
	
	
	@Transient
	private static final Log log = LogFactory.getLog(Datastore.class);
	@Transient
	private static final String tablealias = "s";

	private static enum BackendType {
		HBM, GEN, NONE
	}	

	public static enum ConnectorTypes {
		TEMPORARY, PERSISTED_INTERN, PERSISTED_EXTERN, DIRECT
	}	
	
	
	@Id @GeneratedValue(strategy=GenerationType.TABLE)
	@Column(name="store_id")
    private Long id;
	private String connector;
	private String locator;
	private String context;
	private String schemaName;
	private String tableName;
	private Boolean plain;
	@Transient
	private Locator locatorObject;
	@Transient
	private String queryTable;
	@Transient
	private PersistorDefinition definition;
	@Transient
	private ReentrantReadWriteLock lock = new ReentrantReadWriteLock();
	@Transient
	private IStore backend = null;
	@Transient
	private BackendType hbmDefault = BackendType.valueOf(Settings.getInstance().getContext(Settings.PersistenceCtx).getProperty("backend", "hbm").toUpperCase());


	public Datastore() {
	}

	protected Datastore(PersistorDefinition definition) throws RuntimeException {
		setDefinition(definition);
	}

	/**
	 * gets the internal id of this Datastore
	 * @return the internal unique id
	 */
	public Long getId() {
		return id;
	}

	/**
	 * sets the internal unique id of this Datastore
	 * @param id
	 */
	public void setId(Long id) {
		this.id = id;
	}

	/**
	 * gets the Locator string of the connection to use.
	 * @return the connector
	 */
	public String getConnector() {
		return connector;
	}

	/**
	 * sets the Locator String of the connection to use. This String (as Locator) must reference a valid external connection available to the ETL-Server. Set NULL to use the global internal {#link com.jedox.etl.core.connection.DrillthroughConnection Drillthrough Connection}
	 * @param con the string representation of the locator of a external connection or NULL for internal Drillthrough Connection.
	 */
	public void setConnector(String con) {
		this.connector = con;
	}

	/**
	 * gets the string representation of the locator of this Datastore.
	 * @return the Datastore locator string
	 */
	public String getLocator() {
		return locator;
	}

	/**
	 * sets the string representation of the locator of this Datastore serving as external id.
	 * @param loc the locator string
	 */
	public void setLocator(String loc) {
		this.locator = loc;
	}

	/**
	 * gets the name of the context an external connection is to used in. Has no effect when using internal Drillthrough Connection. see {@link #setConnector(String)}
	 * @return the name of the context of the external connection.
	 */
	public String getContext() {
		return context;
	}

	/**
	 * sets the name of the context of an external connection. Has no effect when using internal Drillthrough Connection. see {@link #setConnector(String)}
	 * @param context
	 */
	public void setContext(String context) {
		this.context = context;
	}

	/**
	 * gets the name of the relational database schema, where this Datastore is stored in.
	 * @return the schema name.
	 */
	public String getSchemaName() {
		return schemaName;
	}

	/**
	 * sets the name of the relational database schema, where this Datastore is stored in.
	 * @param schema the schema name
	 */
	public void setSchemaName(String schema) {
		this.schemaName = schema;
	}

	/**
	 * determines if the names used for schema, table and columns are non-escaped.
	 * @return true, if non-escaped.
	 */
	public Boolean isPlain() {
		return plain;
	}

	/**
	 * sets the escape-mode used for the names of schema, table and columns. If true, no escaping is used and names are exactly as internally specified.
	 * Note: This may lead to errors when using reserved words and characters. You should only set this "true" if you know exactly what you are doing. You have been warned...
	 * @param isPlain true to turn escaping of names off.
	 */
	public void setPlain(Boolean isPlain) {
		this.plain = isPlain;
	}

	
	/** Changes of the current Datastore parameters are checked against a given definition. Necessary to avoid errors if datastore values are overwritten
	 * An Info-Message is raised if parameters would change, could be enhanced to more generic message if needed
	 * @param definition the datastore definition with the new values to be checked
	 * @throws RuntimeException
	 */
	protected void checkChanges (PersistorDefinition definition) throws RuntimeException {
		if (definition!=null) {
			Locator locator = definition.getLocator();
			if (getSchemaName()!=null && !getSchemaName().equals(locator.getPersistentSchema()))
				log.info("The persistence of "+locator.toString()+" will change from schema "+getSchemaName()+" to "+locator.getPersistentSchema());
			if (getTableName()!=null && !getTableName().equals(locator.getPersistentTable()))
				log.info("The persistence of "+locator.toString()+" will change from table "+getTableName()+" to "+locator.getPersistentTable());
				//add check on connection object here, more generic sol
		}	
	}	
	
	/**
	 * Initializes this Datastore by its Definition object. A Datastore can only set writable, when provided with such a definition object.
	 * @param definition the Datastore definition object.
	 */
	protected void setDefinition(PersistorDefinition definition) throws RuntimeException {
		try { //lock this Datastore when setting a definition
			lock.writeLock().lock();
			this.definition = definition;
			setLocator(definition.getLocator().toString());				
			this.locatorObject = definition.getLocator();
			String oldContext = this.context;
			if (definition.getDirectSource() != null) {
				setConnector(definition.getDirectSource().getLocator().toString());
				if (definition.getDirectSource().getContext() != null) //project connection
					setContext(definition.getDirectSource().getContext().getBaseContextName());					
			}
			else {	
				IRelationalConnection connection = (IRelationalConnection) definition.getConnection();
				setConnector(connection.getLocator().toString());
				if (connection.getContext() != null) {//project connection. Internal connection has no context. 
					setContext(connection.getContext().getBaseContextName());
				}else{
					setContext(null);
				}
				setSchemaName(locatorObject.getPersistentSchema());
				setTableName(locatorObject.getPersistentTable());
			}
			if( !((context==null || oldContext==null) ? context == oldContext : context.equals(oldContext)))
				this.queryTable = null; // this have to be build again since the context changed
			setPlain(definition.isPlain());				
		}
		finally {
			lock.writeLock().unlock();
		}
	}

	/**
	 * gets the PersistorDefintion of this Datastore.
	 * @return the PersistorDefinition
	 * @throws RuntimeException, if no definition is set.
	 */
	protected PersistorDefinition getDefinition() throws RuntimeException {
		if (definition == null)
			throw new RuntimeException("Datastore is not writeable since no PersistorDefinition is provided.");
		return definition;
	}

	protected IRelationalConnection getConnectionObject() throws RuntimeException {
		try {
			if (getConnector() == null) { //no connector was ever set.
				throw new RuntimeException("Datastore does not exist.");
			}
			if (!getConnector().isEmpty() && getContext() != null) { //project connection
				IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(getConnector()), getContext());
				if (component instanceof IRelationalExtract)
					component = ((IRelationalExtract) component).getConnection();						
				if (component instanceof IRelationalConnection) {
					IRelationalConnection global = (IRelationalConnection) component;
					ConnectionManager m = new ConnectionManager();
					IRelationalConnection temporary = (IRelationalConnection) m.add(global.getConfigurator().getXML());
					temporary.keep(false); //do not keep open, we want to manually close after usage
					return temporary;
				}
				else
					throw new RuntimeException("Connector "+component.getName()+" in Datastore "+getLocator().toString()+" is not a relational connection.");
			}
			else { //use temporary instance of internal connection since h2 does not support multiple connections on same thread; do NOT set connectionObject
				IRelationalConnection global = ConfigManager.getInstance().getGlobalConnection(getConnector());
				ConnectionManager m = new ConnectionManager();
				IRelationalConnection temporary = (IRelationalConnection) m.add(global.getConfigurator().getXML());
				temporary.keep(false); //do not keep open, we want to manually close after usage
				return temporary;
			}
		}
		catch (ConfigurationException e) {
			throw new RuntimeException(e);
		}
		catch (CreationException e) {
			throw new RuntimeException(e);
		}
	}

	/**
	 * gets the Locator of this Datastore. From the serialized from the Locator object is built via {@link #getLocator()}, {@link #getContext()}, {@link #getSchemaName()}
	 * @return the locator object.
	 */
	protected Locator getLocatorObject() {
		if (locatorObject == null) { //try to build from persistent properties
			locatorObject = Locator.parse(getLocator());
			locatorObject.setContext(getContext());
			locatorObject.setPersistentSchema(getSchemaName());
			locatorObject.setPersistentTable(getTableName());
		}
		return locatorObject;
	}
	
	public String getPersistentName() {
		return getLocatorObject().getPersistentName();
	}

	/**
	 * gets a Processor on this Datastore, which conforms to the given query
	 * @param query the query. the table name used in any from clauses has to be the value specified by {@link NamingUtil#internalDatastoreName()} and will be substituted by the correct value for this datastore.
	 * @param lines the number of lines to return
	 * @return the processor holding the data
	 * @throws RuntimeException
	 */
	public IProcessor getProcessor(String query, int lines) throws RuntimeException {
		try {
			lock.readLock().lock();
			log.debug("Locking schema "+getSchemaName());
			query = query.replace(NamingUtil.internalDatastoreName(), getQueryTable());
			log.debug("Execute SQL-Query: "+query);
			IRelationalConnection connection = getConnectionObject();
			IProcessor result = connection.getProcessor(query, false, lines);
			if (result != null) 
				return new DatastoreProcessor(result,connection);
			else
				throw new RuntimeException("Query is not valid for datastore "+getLocator()+": "+query);
			}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
		finally {
			log.debug("Unlocking schema:"+getSchemaName());			
			lock.readLock().unlock();
		}
	}

	private String[] getNamesFromRow() throws RuntimeException {
		//get names from datastore
		Row row = getDataColumns();
		String[] names = new String[row.size()];
		//coordinates
		for (int i=0; i<names.length-1; i++) {
			names[i] = row.getColumn(i).getName();
		}
		//value -- info nodes are between coordinates and values and thus are ignored.
		names[names.length-1] = row.getColumn(row.size()-1).getName();
		return names;
	}
	
	private List<String> adaptColumnNames(String[] names) throws RuntimeException {
		
		if (getConnectorType().equals(ConnectorTypes.DIRECT )){ 			
			IRelationalExtract directDrillthroughExtract = null;;
			try {
				directDrillthroughExtract = (IRelationalExtract)ConfigManager.getInstance().getComponent(Locator.parse(getConnector()), getContext());
			} catch (ConfigurationException e) {
				throw new RuntimeException(e.getMessage());
			}
			IAliasMap map = directDrillthroughExtract.getAliasMap();
			if (!map.isEmpty()){					
				// Rename filter name to column if it differs only in upper/lower case
				Set<String> origins = map.getAliasesOrigin(); 
				for (int i=0; i<names.length; i++) {
						boolean found=false;
						for(String key:map.getAliases()){
							if(map.getElement(key).getName().equalsIgnoreCase(names[i])){
								names[i] = map.getElement(key).getOrigin();
								found=true;
								break;
							}
						}
						if (!found) {
							for(String origin:origins){
								if(origin.equalsIgnoreCase(names[i])){
									names[i] = origin;
									found=true;
									break;
								}
							}	
						}
						if (!found)
							throw new RuntimeException("Column "+names[i]+" not found in Relational Extract "+Locator.parse(getConnector()).getName());						
					}
				return null;
			} else {
				log.info("Drillthrough query performance can be increased by using an AliasMap in RelationalExtract " + Locator.parse(getConnector()).getName());				
			}
		}		
		// For Persisted Drillthrough and Direct Drillthrough without Alias Map		
		//gets select names (all but internal key)
		IRelationalConnection connection = getConnectionObject();
		Row selects = connection.getProcessor("select "+tablealias+".* from "+getQueryTable(), true, 1).current();
		connection.close();
				
		// Rename filter name to column if it differs only in upper/lower case
		List<String> columnNames = selects.getColumnNames(); 
		for (int i=0; i<names.length; i++) {
			if (!selects.containsColumn(names[i])) {
				for (String columnName : columnNames) {
					if (names[i].equalsIgnoreCase(columnName)) {						
						log.debug("Column name "+names[i]+" changed to "+columnName+" for request on table "+getQueryTable());
						names[i]=columnName;
						continue;
					}	
				}
			}	
		}
		if(columnNames.get(0).equals(NamingUtil.internalKeyName()))
			return columnNames.subList(1, columnNames.size());
		else
			return columnNames;
	}
	
	
	/**
	 * gets a Processor on this Datastore delivering all (non-internal) columns and rows complying to equality criteria. Each given column name must have a value given for an exact match.
	 * @param names the names of the columns to apply the equality criteria.
	 * @param values the values of the equality criteria.
	 * @param lengths the number of values for each dimension in values array.
	 * @param lines the maximum number of lines to return
	 * @return the Processor holding the data.
	 * @throws RuntimeException
	 * @throws ConfigurationException 
	 */
	public IProcessor getFilteredProcessor(String[] names, String[] values, int[] lengths,  int lines) throws RuntimeException {
		log.info("Reading datastore "+locator+" of type " +getConnectorType() +
				 (connector.equals("Drillthrough") ? "" : " with Connector: "+connector) +
				 (schemaName==null ? "" : " Schema: "+schemaName) + 
				 (tableName==null ? "" :  " Table: "+tableName)); 		
		//get names from row, if not specified.
		if (names == null)
			names = getNamesFromRow();	
		if(values==null)
			values= new String[]{};
		
		List<String> selects = adaptColumnNames(names);
		
		List<Integer> len = new ArrayList<Integer>();
		String lengthMessage="Number of values requested:";
		for (int l : lengths) {
			len.add(l);
			lengthMessage=lengthMessage.concat(" "+l);
		}
		log.info(lengthMessage);
		
		List<String> prefixSelects = new ArrayList<String>();	
		if(getConnectorType().equals(ConnectorTypes.DIRECT)){
			prefixSelects.add("*");
		}else{
			prefixSelects = SQLUtil.prefixNames(selects,tablealias,getQuote());
		}
		
		List<String> prefixNames = SQLUtil.prefixNames(Arrays.asList(names),tablealias,getQuote());
		String query = SQLUtil.buildQuery(
				NamingUtil.internalDatastoreName(),
				SQLUtil.enumNames(prefixSelects),
				SQLUtil.getWhereCondition(prefixNames, SQLUtil.quoteValues(Arrays.asList(values)), len),
				"","");
		if (lock.isWriteLocked())
			log.info("Datastore "+getLocator()+" id:"+getId()+" is write locked.");
		return getProcessor(query,lines);
	}

	public String getQueryTable() throws RuntimeException {
		try {
			if (queryTable == null) { //try to build from persistent properties
				if (!getConnector().isEmpty() && getContext() != null) { //project connection
					IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(getConnector()), getContext());
					if (component instanceof IRelationalExtract) {
						String sqlquery=((IRelationalExtract)component).getSQLQuery().trim();
						if (sqlquery.endsWith(";"))
							sqlquery=sqlquery.substring(0, sqlquery.length()-1);
						//starting with "(" will be used in API as a check to know when direct drillthrough is used 
						queryTable =  "( "+sqlquery+" )"+" "+tablealias;
					}	
					else if (component instanceof IRelationalConnection) {
						IRelationalConnection connection = getConnectionObject();
						queryTable = getLocatorObject().getPersistentName(connection.getIdentifierQuote())+" "+tablealias;
						connection.close();
					}
					else
						throw new RuntimeException("Connector "+component.getName()+" in Datastore "+getLocator().toString()+" is not a relational connection.");
				}
				else {
					IRelationalConnection connection = getConnectionObject();
					queryTable = getLocatorObject().getPersistentName(connection.getIdentifierQuote())+" "+tablealias;
					connection.close();
				}
				
			}
		}
		catch (ConfigurationException e) {
			throw new RuntimeException(e);
		}
		return queryTable;
	}

	/**
	 * gets all externally filled data columns. Omits all internally created columns such as the primary auto-generated key.
	 * @return the Row containing the data columns
	 * @throws RuntimeException
	 */
	public Row getDataColumns() throws RuntimeException {
		IRelationalConnection connection = getConnectionObject();
		Row result = connection.getProcessor("select "+tablealias+".* from "+getQueryTable(), true, 1).current();
		result.removeColumn(NamingUtil.internalKeyName());
		connection.close();
		return result;
	}
	
	/**
	 * @param tableName the tableName to set
	 */
	public void setTableName(String tableName) {
		this.tableName = tableName;
	}

	/**
	 * @return the tableName
	 */
	public String getTableName() {
		return tableName;
	}

	/**
	 * 
	 */

	public String escapeName(String name) throws RuntimeException {
		IRelationalConnection connection = getConnectionObject();
		String result = SQLUtil.quoteName(name, connection.getIdentifierQuote());
		connection.close();
		return result;
	}

	private String getQuote() throws RuntimeException {
		return escapeName("x").substring(0, 1);
	}

	/**
	 * sets this Datastore writeable. Needs {@link #setDefinition(PersistorDefinition)} to be set.
	 * @throws RuntimeException
	 */
	public void setWriteable() throws RuntimeException {
		try {
			lock.writeLock().lock(); //lock datastore until commit
			PersistorDefinition definition = getDefinition();
			
			BackendType backendType = BackendType.valueOf(definition.getConnection().getParameter().getProperty("#backend", hbmDefault.toString()).toUpperCase());
			if (backendType.equals(BackendType.NONE)) {
				throw new RuntimeException("Relational Database "+definition.getConnection().getServerName()+" is not supported for writing of data.");				
			}
			
			if (backendType.equals(BackendType.HBM)) {
				log.debug("Using hibernate backend for Datastore "+getLocator());
				backend = new HibernateStore(definition);
			}
			else {
				log.debug("Using generic backend for Datastore "+getLocator());
				backend = new GenericStore(definition);
			}

		}
		catch (Exception e) {
			lock.writeLock().unlock();
			throw new RuntimeException(e);
		}
	}

	public void write() throws RuntimeException {
		if (backend != null)
			backend.write();
		else
			throw new RuntimeException("Datastore not initialized for writing.");
	}

	public void commit() throws RuntimeException {
		try {
			backend.commit();
		}
		finally { //unlock datastore
			lock.writeLock().unlock();
		}
	}

	@Override
	public long getCurrentRowCount() throws RuntimeException {
		IRelationalConnection connection = getConnectionObject();
		Row result = connection.getProcessor("select count(*) from "+getQueryTable(), true, 1).current();
		connection.close();
		return Long.parseLong(result.getColumn(0).getValueAsString());
	}

	@Override
	public void close() throws RuntimeException {
		queryTable = null;
		if (backend != null)
			backend.close();		
	}
	
	public ConnectorTypes getConnectorType() {
		if (connector.equals("Temporary"))
			return ConnectorTypes.TEMPORARY;
		if (connector.equals("Drillthrough"))
			return ConnectorTypes.PERSISTED_INTERN;
		Locator loc = Locator.parse(getConnector());
		if (loc==null)
			return null;
		if (loc.getManager().equals("connections"))
			return ConnectorTypes.PERSISTED_EXTERN;
		if (loc.getManager().equals("extracts"))
			return ConnectorTypes.DIRECT;
		return null;
	}
}