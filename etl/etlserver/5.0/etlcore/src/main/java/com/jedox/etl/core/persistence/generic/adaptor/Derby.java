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
package com.jedox.etl.core.persistence.generic.adaptor;

import java.sql.ResultSet;
import java.sql.Timestamp;
import java.sql.Connection;
import java.sql.Date;
import java.sql.Time;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Hashtable;
import java.util.Properties;
import java.io.File;
import java.math.BigDecimal;
import java.lang.Float;


import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.generic.GenericPersistor;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.config.Settings;

public class Derby extends GenericPersistor {

	private static final Log log = LogFactory.getLog(Derby.class);

	//for internal persistence use
	private String driver = "org.apache.derby.jdbc.EmbeddedDriver";
    private String protocol = "jdbc:derby:";
    private String dbDir = Settings.getInstance().getPersistenceDir();
    private String dbName = dbDir+File.separator+"etl_temporary"; // the name of the database
    private String logFile = Settings.getInstance().getLogDir() + File.separator + "derby.log"; //the derby log file
    private boolean recreate = true;

    //for external persistence use
    private IConnection connection = null;


	public Derby() {
		//default internal persistence
		init();
	}

	public Derby(String dbName, boolean recreate) {
		//internal persistence for special purpose such as drillthrough
		this.dbName = dbDir + File.separator + dbName;
		this.recreate = recreate;
		init();
	}

	private void init() {
		try {
			//specify log file location
			System.setProperty("derby.stream.error.file", logFile);
			Class.forName(driver);
			log.debug("Initialized internal derby persistence.");
			//getSchemas();
			//getTables(executionSchema);
		}
		catch (Exception e) {
			log.error("Failed to find derby driver: "+driver);
		}
	}

	public Derby(IConnection connection) {
		//connection to external derby database
		this.connection = connection;
	}

	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "VARCHAR (32672)");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "DOUBLE");
		lookup.put(Date.class.getCanonicalName(), "DATE");
		lookup.put(Time.class.getCanonicalName(), "TIME");
		lookup.put(Timestamp.class.getCanonicalName(), "TIMESTAMP");
		lookup.put(BigDecimal.class.getCanonicalName(), "NUMERIC(30,?)");
		lookup.put(Boolean.class.getCanonicalName(), "CHAR () FOR BIT DATA");
		lookup.put(Byte.class.getCanonicalName(), "SMALLINT");
		lookup.put(Short.class.getCanonicalName(), "SMALLINT");
		lookup.put(Long.class.getCanonicalName(), "BIGINT");
		lookup.put(Float.class.getCanonicalName(), "FLOAT");	
		lookup.put(Object.class.getCanonicalName(), "LONG VARCHAR FOR BIT DATA");
		lookup.put(byte[].class.getCanonicalName(), "LONG VARCHAR FOR BIT DATA");
		lookup.put("default", "VARCHAR (32672)");
		return lookup;
	}

	public boolean existsFileDB(String path) {
		return new File(path).exists();
	}

	public Connection createFileDB(String dbName, boolean create) {
		Connection connection = null;
		Properties properties = new Properties();
		//optimize for large tables
		properties.put("derby.storage.pageSize", "32768");
		properties.put("derby.storage.pageCacheSize","10000");
		if (create) {
			try {
				//explicitly delete database
				FileUtil.deleteDirectory(new File(dbName));
				connection = DriverManager.getConnection(protocol +
						dbName + ";create=true", properties);
				log.debug("Connected to internal derby database " + dbName);
				connection.setAutoCommit(false);
			}
			catch (Exception e1) {
				log.error("Failed to establish internal connection, make sure that you have full control over the etlserver directory.");
				log.error("ETL server will not start correctly;hence it will be interrupted.");
				System.exit(0);
			}
		}
		else {
			try {
				connection = DriverManager.getConnection(protocol +
				dbName + ";create=false", properties);
				log.debug("Created and connected to internal derby database " + dbName);
				connection.setAutoCommit(false);
			}
			catch (Exception e2) {
				log.error("Failed to (re-)establish internal connection, make sure that you have full control over the etlserver directory.");
				log.error("ETL server will not start correctly;hence it will be interrupted.");
				System.exit(0);
			}
		}
		return connection;
	}

	@Override
	protected Connection setupConnection() throws RuntimeException {
		//if external connection is set
		if (connection instanceof IRelationalConnection)
			return ((IRelationalConnection)connection).open();
		//else create a internal connection
		Connection connection = createFileDB(dbName,(recreate == false) ? !existsFileDB(dbName) : true);
		return connection;
	}

	public void disconnect() {
		if (connection != null)
			connection.close(); //close external connection
		else if (hasConnection()) { //close internal connection if in use
			super.disconnect();
			 try
	         {
	             // the shutdown=true attribute shuts down Derby
	             DriverManager.getConnection("jdbc:derby:;shutdown=true");
	             // To shut down a specific database only, but keeep the
	             // engine running (for example for connecting to other
	             // databases), specify a database in the connection URL:
	             //DriverManager.getConnection("jdbc:derby:" + dbName + ";shutdown=true");
	         }
	         catch (SQLException se)
	         {
	             if (( (se.getErrorCode() == 50000) && ("XJ015".equals(se.getSQLState()) ))) {
	                 // we got the expected exception
	                 log.debug("Derby shut down normally");
	                 // Note that for single database shutdown, the expected
	                 // SQL state is "08006", and the error code is 45000.
	             } else {
	                 // if the error code or SQLState is different, we have
	                 // an unexpected exception (shutdown failed)
	                 log.debug("Derby did not shut down normally: "+se.getMessage());
	             }
	         }
		}
	}

	protected String getGeneratedKeySyntax() {
		return "INTEGER PRIMARY KEY GENERATED ALWAYS AS IDENTITY";
	}

	public void renameTable(Locator source, Locator target, Row columnDefinition, boolean recreateEmptySource) throws RuntimeException {
		String sourceName = getPersistentName(source);
		String targetName = escapeName(target.getName());
		dropTable(target);
		try {
			Statement stm = getConnection().createStatement();
			String sql = "RENAME TABLE "+sourceName+" TO "+targetName;
			stm.execute(sql);
		}
		catch (SQLException e) {
			throw new RuntimeException("Failed to rename table from "+source+" to "+"target: "+e.getMessage());
		}
		if (recreateEmptySource)
			createTable(source, columnDefinition);
	}
	
	protected void dropSchemaInternal(String name) throws RuntimeException {
		Statement statement;
		try {
			statement = getConnection().createStatement();
			ResultSet tables = getConnection().getMetaData().getTables(null, name.replaceAll(getMetadataModule().getQuote(), ""), null, null);
			String table = "";
			while(tables.next()){
				table = tables.getString("TABLE_NAME");
				try{
					statement.execute("DROP Table "+ name+"."+escapeName(table));
				} catch (SQLException e) {
					log.debug("Failed to drop table " + table + " in schema "+name+": "+e.getMessage());
				}
			}
			tables.close();
			
	
			boolean deleted = statement.execute("DROP SCHEMA "+name + " RESTRICT");
			statement.close();
		} catch (SQLException e) {
			log.debug("Failed to drop schema "+name+": "+e.getMessage());
		}
	}

}
