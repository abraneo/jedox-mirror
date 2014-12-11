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
package com.jedox.etl.core.persistence.hibernate;

import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.hibernate.Criteria;
import org.hibernate.Query;
import org.hibernate.StatelessSession;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.hibernate.ejb.Ejb3Configuration;
import org.hibernate.cfg.Configuration;
import org.hibernate.criterion.Projections;

import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.connection.IConnection.FetchModes;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode;
import com.jedox.etl.core.node.RelationalNode.UpdateModes;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.IStore;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.SQLUtil;
import com.jedox.etl.core.util.TypeConversionUtil;

public class HibernatePersistor implements IStore {

	private class HibernateProcessor extends Processor {

		private Row row;
		private Iterator<Map<String,Object>> iterator;

		public HibernateProcessor(Row row, Iterator<Map<String,Object>> iterator) {
			this.row = row;
			this.iterator = iterator;
		}

		protected boolean fillRow(Row row) throws Exception {
			if (iterator.hasNext()) {
				Map<String,Object> map = iterator.next();
				for (String key: map.keySet()) {
					row.getColumn(key).setValue(map.get(key));
				}
				return true;
			}
			return false;
		}

		protected Row getRow() {
			return row;
		}

		@Override
		protected void init() throws RuntimeException {
			// nothing to do here.
		}
	}

	private SessionFactory sessionFactory;
	private StatelessSession session;
	private String name;
	private Row dataDefinition;
	private Row primaryKeys;
	private Transaction persistTransaction;
	private MappingHelper mappingHelper;
	private PersistorDefinition definition;
	private long rowCountInitial = 0;
	private static final Log log = LogFactory.getLog(HibernatePersistor.class);


	public HibernatePersistor(PersistorDefinition definition) throws RuntimeException {
		this.definition = definition;
		mappingHelper = new MappingHelper(definition);
		setName(definition.getLocator().getName());
		if (getConnection().autoCreateSchemata()) {//create schema if needed
			if (!getLocator().getPersistentSchema().isEmpty())
				try {
					createSchemaInternal(SQLUtil.quoteName(getLocator().getPersistentSchema(),getConnection().getIdentifierQuote()));
				} catch (RuntimeException e) {}
		}
		setDataDefinition(initDataDefinition(definition.getColumnDefinition()));
		rowCountInitial = getCurrentRowCount();
		//delete existing data in mode update
		if (definition.getMode().equals(Modes.UPDATE))
			delete(new Row());
		openTransaction();
	}

	protected PersistorDefinition getDefinition() {
		return definition;
	}

	protected void setName(String name) {
		this.name = name;
	}

	protected String getName() {
		return name;
	}

	protected IRelationalConnection getConnection() {
		return definition.getConnection();
	}

	protected Locator getLocator() {
		return definition.getLocator();
	}

	protected Row getDataDefinition() {
		return dataDefinition;
	}

	protected void setDataDefinition(Row dataDefinition) {
		this.dataDefinition = dataDefinition;
	}

	protected Log getLog() {
		return log;
	}

	protected String getSchemaMode() {
		switch (getDefinition().getMode()) {
		case CREATE: return "create";
		case TEMPORARY: return "create-drop";
		default: return "update";
		}
	}

	protected void setPrimaryKeys(Row primaryKeys) {
		this.primaryKeys = primaryKeys;
	}

	protected Row getPrimaryKeys() {
		return primaryKeys;
	}

	protected Map<String,String> getColumnMap() {
		return mappingHelper.getColumnMap();
	}

	protected String getTablename() throws RuntimeException {
		return getLocator().getPersistentName(getConnection().getIdentifierQuote());
	}

	protected void openTransaction() {
		if (persistTransaction == null) {
			StatelessSession session = getSession();
			persistTransaction = session.beginTransaction();
		}
	}

	protected StatelessSession getSession() {
		return session;
	}

	protected void verifyColumnNames(Row dataDefinition) throws RuntimeException {
		//check if we have column names, which are distinguished only in case.
		Set<String> names = new HashSet<String>();
		for (IColumn c : dataDefinition.getColumns()) {
			names.add(c.getName().toLowerCase());
		}
		if (names.size() != dataDefinition.size()) {
			log.warn("Data has one or more columns, which are distinguishable only by their case. This may cause an error and should be avoided.");
		}
	}

	protected Map<String,String> getCustomConfiguration() {
		Map<String,String> map = new HashMap<String,String>();
		map.put("hibernate.hbm2ddl.auto", getSchemaMode());
		return map;
	}

	private Map<String,String> getConfiguration(IRelationalConnection connection) throws RuntimeException {
		Map<String,String> map = new HashMap<String,String>();
		map.put("hibernate.connection.driver_class", connection.getDriver());
		map.put("hibernate.connection.url", connection.getConnectionUrl());
		map.put("hibernate.connection.username", connection.getUser());
		map.put("hibernate.connection.password", connection.getPassword());
		map.put("hibernate.connection.release_mode", "on_close");
		if(connection.getFetchMode().equals(FetchModes.FULL)) map.put("hibernate.connection.autoClosePStmtStreams", "true");
		//map.put("etl.connection.locator", connection.getLocator().toString());
		//map.put("etl.connection.context", connection.getContextName());
		//map.put("hibernate.connection.provider_class", "com.jedox.etl.core.hibernate.InternalConnectionProvider");
		map.putAll(getCustomConfiguration());
		return map;
	}

	protected void initPersistence(Row dataDefinition, Row primaryKeys) throws RuntimeException {
		try {
			//Level level = Logger.getLogger("org.hibernate").getLevel();
			//turn hibernate logging to warn level
			verifyColumnNames(dataDefinition);
			Logger.getLogger("org.hibernate").setLevel(Level.WARN);
			//load driver class into memory.
			Class.forName(getConnection().getDriver());
  		    getConnection().test();
			Ejb3Configuration ejbConfiguration = new Ejb3Configuration().configure("data",getConfiguration(getConnection()));
			Configuration configuration = ejbConfiguration.getHibernateConfiguration();
			//turn connection autocommit off. this can not be done in an Ejb3Configuration
			configuration.getProperties().setProperty("hibernate.connection.autocommit", "false");
			configuration.addDocument(mappingHelper.createMappingDocument(dataDefinition, primaryKeys));
			sessionFactory = configuration.buildSessionFactory();
			//session = sessionFactory.openStatelessSession(getConnection().open());
			if (getConnection().isSingleton()) { //use singleton connection
				session = sessionFactory.openStatelessSession(getConnection().open());
			}
			else {
				session = sessionFactory.openStatelessSession();
			}
			//Logger.getLogger("org.hibernate").setLevel(level);
		}
		catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}
	}

	protected Row readDataDefinition(IRelationalConnection connection, Locator locator, Row givenDefinition) throws RuntimeException {
		connection.commit();
		String query = "select * from "+getTablename();
		IProcessor processor = connection.getProcessor(query, null, true, false, 1);
		Row row = processor.current();
		processor.close();
		List<String> keyNames = new ArrayList<String>();
		//try to determine keys from database
		try {
			DatabaseMetaData meta = connection.open().getMetaData();
			String schema = (locator.getPersistentSchema().isEmpty()) ? null : locator.getPersistentSchema();
			ResultSet keys = meta.getPrimaryKeys(null, schema, getLocator().getPersistentTable());
			while (keys.next()) {
				String key = keys.getString(4);
				keyNames.add(key);
			}
		} catch (SQLException e) {
			throw new RuntimeException(e);
		}
		//fix for databases not correctly returning primary keys but having internal an internal key / explicit setting of primary key
		if (keyNames.isEmpty() && row.containsColumn(getPrimaryKey(NamingUtil.internalHibernateKeyName()))) {
			keyNames.add(getPrimaryKey(NamingUtil.internalHibernateKeyName()));
			log.warn("Existing table "+getLocator().getPersistentTable()+" does not have primary key specified. Using "+getPrimaryKey(NamingUtil.internalHibernateKeyName())+" as logical primary key.");
		}
		Row definition = new Row();
		Row primaryKeys = new Row();
		for (IColumn source: row.getColumns()) {
			RelationalNode target = ColumnNodeFactory.getInstance().createRelationalNode(source.getName(), null);
			target.mimic(source);
			//set primary key roles from db
			if (keyNames.contains(target.getName())) {
				target.setRole(UpdateModes.primary);
				primaryKeys.addColumn(target);
				//add assigned key to definition also.
				if (givenDefinition.containsColumn(target.getName())) {
					definition.addColumn(target);
				}
			}
			else if (givenDefinition.containsColumn(target.getName())) { //set column roles from given definition
				target.setRole(givenDefinition.getColumn(target.getName(),RelationalNode.class).getRole());
				definition.addColumn(target);
			}
			else {
				log.debug("Existing column '"+target.getName()+"' in table '"+getLocator().getPersistentTable()+"' is not present as data column. Null values will be written instead.");
				target.setRole(UpdateModes.first);
				definition.addColumn(target);
			}
		}
		setPrimaryKeys(primaryKeys);
		//give a warning if read primary keys are different to set ones
		if (getDefinition().getPrimaryKey() != null && (primaryKeys.size() != 1 || !primaryKeys.getColumn(0).getName().equalsIgnoreCase(getDefinition().getPrimaryKey())))
			log.warn("Used keys from existing table "+getLocator().getPersistentTable()+" differ from explicitly set key "+getDefinition().getPrimaryKey()+", which is ignored.");
		//give a warning if givenColumns are not present in read definition
		for (IColumn c : givenDefinition.getColumns()) {
			if (!row.containsColumn(c.getName()))
				log.warn("Data column '"+c.getName()+"' is not present in existing table '"+getLocator().getPersistentTable()+"'. Thus it cannot be written and is ignored.");
		}
		return definition;
	}

	protected Row createDataDefinition(Row row) throws RuntimeException {
		dataDefinition = new Row();
		dataDefinition.setName(MappingHelper.targetNew);
		for (IColumn source: row.getColumns()) {
			Column target = new Column();
			target.mimic(source);
			dataDefinition.addColumn(target);
		}
		Row primaryKeys = new Row();
		IColumn key = null;
		String keyName = getPrimaryKey(NamingUtil.internalHibernateKeyName());
		if (getDefinition().getColumnDefinition().containsColumn(keyName)) {
			if (!getPrimaryKeyGeneration(MappingHelper.strategyAssigned).equals(MappingHelper.strategyAssigned))
				log.warn("Overruling primary key generation strategy on column "+keyName+" by assigning given input data.");
			key = getDefinition().getColumnDefinition().getColumn(keyName);
			getDefinition().setPrimaryKeyGeneration(MappingHelper.strategyAssigned);
		}
		else {
			RelationalNode generatedKey = ColumnNodeFactory.getInstance().createRelationalNode(keyName, null);
			generatedKey.setRole(UpdateModes.primary);
			if (!MappingHelper.strategyGuid.equals(getDefinition().getPrimaryKeyGeneration()))
				generatedKey.setValueType(Long.class);
			else
				generatedKey.setValueType(String.class);
			key = generatedKey;
		}
		primaryKeys.addColumn(key);
		setPrimaryKeys(primaryKeys);
		return dataDefinition;
	}

	protected Row initDataDefinition(Row row) throws RuntimeException {
		Row dataDefinition = null;
		if (!getSchemaMode().startsWith("create")) { //try to read in
			try {
				dataDefinition = readDataDefinition(getConnection(),getLocator(),row);
				dataDefinition.setName(MappingHelper.targetExisting);
			}
			catch (Exception e) {
				log.info("Persistence for "+getLocator().getName()+" does not exists. Creating new one.");
			}
		}
		if (dataDefinition == null) { //create new from given row
			dataDefinition = createDataDefinition(row);
		}
		initPersistence(dataDefinition, getPrimaryKeys());
		return dataDefinition;
	}

	protected List<String> getHibernateNames(Row row) {
		List<String> result = new ArrayList<String>();
		for (IColumn c : row.getColumns())
			result.add(getColumnMap().get(c.getName()));
		return result;
	}

	protected Object convert(IColumn dataColumn, IColumn definitionColumn) {
		Object convertValue = null;
		if (dataColumn != null) {
			definitionColumn.setValue(dataColumn.getValue());
			try {
				convertValue = TypeConversionUtil.convert(definitionColumn);
			}
			catch (Exception e) {
				log.warn("Conversion problem in "+getLocator().getType()+" "+name+": "+e.getMessage()+". Setting value to "+convertValue);
			}
		}
		return convertValue;
	}

	private Map<String,Object> rowToMap(Row row) throws RuntimeException {
		Map<String,Object> result = new HashMap<String, Object>();
		Row definition = getDataDefinition();
		//map from given data to definition
		for (int i=0; i<definition.size(); i++) {
			IColumn definitionColumn = definition.getColumn(i);
			IColumn dataColumn = row.getColumn(definitionColumn.getName());
			if (dataColumn != null) {
				result.put(getColumnMap().get(dataColumn.getName()), convert(dataColumn,definitionColumn));
			}
		}
		return result;
	}

	protected void save(Row row) throws RuntimeException {
		StatelessSession session = getSession();
		try {
			//session.save(getName(),rowToMap(row));
			session.insert(getName(),rowToMap(row));
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	protected void delete(Row row) throws RuntimeException {
		Row data = row;
		String whereClause = SQLUtil.getPrepearedWhereCondition(getHibernateNames(data));
		if (whereClause.isEmpty()) whereClause = " where true = true"; //bugfix. hibernate in this version needs a where clause in delete for some unknown reason and contrary to documentation
		String hqlDelete = "delete from "+getName()+whereClause;
		//System.out.println(hqlDelete);
		try {
			//Query query = getSession().createSQLQuery( sqlDelete );
			Query query = getSession().createQuery( hqlDelete );
			for (int i=0; i<data.size(); i++) {
				IColumn c = data.getColumn(i);
				Object value =  convert(c,getDataDefinition().getColumn(c.getName()));
				query.setParameter(i,value);
			}
			int deletedEntities = query.executeUpdate();
			log.debug("deleted "+deletedEntities+" lines.");
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}


	public void write() throws RuntimeException {
		save(getDefinition().getColumnDefinition());
	}

	protected void flush() {
		if (persistTransaction != null) {
			persistTransaction.commit();
			persistTransaction = null;
		}
	}

	public void commit() throws RuntimeException {
		try {
			//this is needed for all drivers, where no autocommit is done, eg. postgres database
			flush();
			getSession().connection().commit();
			if (!Modes.TEMPORARY.equals(getDefinition().getMode())) {
				long rowCountCurrent = getCurrentRowCount();
				log.info("The number of rows in table " + getName() + " changed from " + rowCountInitial + " to " +  rowCountCurrent);
			}
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}


	public void close() throws RuntimeException {
		try {
			//changes are not committed, commit() has to be called before
			getSession().close();
			sessionFactory.close();
			//try to drop the schema if we are in create drop mode
			if (!getLocator().getPersistentSchema().isEmpty() && getDefinition().getMode().equals(Modes.TEMPORARY)) {
				dropSchemaInternal(SQLUtil.quoteName(getLocator().getPersistentSchema(), getConnection().getIdentifierQuote()));
			}
			dataDefinition = null;
			session = null;
			sessionFactory = null;
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	public long getCurrentRowCount() {
		try{
			Criteria criteria = createCriteria();
			criteria.setProjection(Projections.rowCount());
			return Long.parseLong(criteria.list().get(0).toString());
			//return Long.parseLong(getSession().createQuery("select count(*) from " + getName()).uniqueResult().toString());
		}catch(Exception e){
			log.error(e.getMessage());
		}
		return 0;
	}


	protected void createSchemaInternal(String name) {
		Connection c = null;
		try {
			c = getConnection().open();
			c.setSavepoint();
			Statement statement = c.createStatement();
			statement.execute("CREATE SCHEMA "+name);
			c.commit();
			statement.close();
		}
		catch (Exception e) {
			if (c != null)
				try {
					c.rollback();
				} catch (SQLException e1) {}
			log.debug("Failed to create schema "+name+": "+e.getMessage());
		}
	}

	protected void dropSchemaInternal(String name)  {
		Statement statement;
		Connection c = null;
		try {
			c = getConnection().open();
			statement = c.createStatement();
			c.setSavepoint();
			statement.execute("DROP SCHEMA "+name);
			c.commit();
			statement.close();
		} catch (Exception e) {
			if (c != null)
				try {
					c.rollback();
				} catch (SQLException e1) {}
			log.debug("Failed to drop schema "+name+": "+e.getMessage());
		}
	}

	@SuppressWarnings("unchecked")
	public IProcessor get(int size) throws RuntimeException {
		return getConnection().initProcessor(new HibernateProcessor(getDataDefinition(),getSession().createQuery("from "+getName()).setMaxResults(size).iterate()),Facets.HIDDEN);
	}

	protected Criteria createCriteria() {
		return getSession().createCriteria(getName());
	}

	private String getPrimaryKey(String defaultValue) {
		return getDefinition().getPrimaryKey() != null ? getDefinition().getPrimaryKey() : defaultValue;
	}

	private String getPrimaryKeyGeneration(String defaultValue) {
		return getDefinition().getPrimaryKeyGeneration() != null ? getDefinition().getPrimaryKeyGeneration() : defaultValue;
	}


	//debug helper methods

	/*
	public String toSql(Criteria criteria){
    try{
      CriteriaImpl c = (CriteriaImpl) criteria;
      SessionImpl s = (SessionImpl)c.getSession();
      SessionFactoryImplementor factory = (SessionFactoryImplementor)s.getSessionFactory();
      String[] implementors = factory.getImplementors( c.getEntityOrClassName() );
      CriteriaLoader loader = new CriteriaLoader((OuterJoinLoadable)factory.getEntityPersister(implementors[0]),
        factory, c, implementors[0], s.getEnabledFilters());
      Field f = OuterJoinLoader.class.getDeclaredField("sql");
      f.setAccessible(true);
      return (String) f.get(loader);
    }
    catch(Exception e){
      throw new RuntimeException(e);
    }
    }

	protected String toSql(String hqlQueryText){
	    if (hqlQueryText!=null && hqlQueryText.trim().length()>0){
	      final QueryTranslatorFactory translatorFactory = new ASTQueryTranslatorFactory();
	      final SessionFactoryImplementor factory = (SessionFactoryImplementor) sessionFactory;
	      final QueryTranslator translator = translatorFactory.
	        createQueryTranslator(
	          hqlQueryText,
	          hqlQueryText,
	          Collections.EMPTY_MAP, factory
	        );
	      translator.compile(Collections.EMPTY_MAP, false);
	      return translator.getSQLString();
	    }
	    return null;
	  }
 */

}
