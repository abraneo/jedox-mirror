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

import java.math.BigDecimal;
import java.sql.Timestamp;
import java.sql.Connection;
import java.util.Date;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Time;
import java.util.Hashtable;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hsqldb.Server;
import org.hsqldb.jdbc.JDBCDataSource;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

public class Hsqldb extends GenericPersistor {
	
	private static final Log log = LogFactory.getLog(Hsqldb.class);
	public final String persistenceMemory = "MEMORY";
	public final String persistenceDisc = "CACHED";
	//internal server
	private Server hsqlServer;
	
	//external connection
    private IConnection connection = null;
	
	public Hsqldb(IConnection connection) {
		this.connection = connection;
	}
	
	public void commit() {
		try {
			Statement statement = getConnection().createStatement();
			statement.execute("COMMIT");
		} catch (Exception e) {}
	}
	
	protected void createSchemaInternal(String name) throws SQLException, RuntimeException {
		Statement statement = getConnection().createStatement();
		statement.execute("CREATE SCHEMA "+name+" AUTHORIZATION DBA");
	}
	
	protected Connection setupConnection() throws RuntimeException {
		//if external connection is set
		if (connection instanceof IRelationalConnection)
			return ((IRelationalConnection)connection).open();
		//else create an internal connection
		Connection connection = null;
		JDBCDataSource ds = new JDBCDataSource();
		ds.setDatabase("jdbc:hsqldb:hsql://localhost:"+hsqlServer.getPort()+"/internal");
		ds.setUser("sa");
		ds.setPassword("etl");
		try {
			connection = ds.getConnection();
			connection.setAutoCommit(false);
			Statement stm = connection.createStatement();
			stm.execute("SET IGNORECASE TRUE;");
		}
		catch (Exception e) {
			log.error("Failed to (re-)establish internal connection.");
		}
		return connection;
	}
	
	/**
	 * Inits Translation Table from java-types to hsqldb data types used in SQL create table statement
	 *
	 */
	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "VARCHAR(32672)");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "DOUBLE");
		lookup.put(Date.class.getCanonicalName(), "DATE");
		lookup.put(Time.class.getCanonicalName(), "TIME");
		lookup.put(Timestamp.class.getCanonicalName(), "TIMESTAMP");
		lookup.put(BigDecimal.class.getCanonicalName(), "NUMERIC");
		lookup.put(Boolean.class.getCanonicalName(), "BOOLEAN");
		lookup.put(Byte.class.getCanonicalName(), "TINYINT");
		lookup.put(Short.class.getCanonicalName(), "SMALLINT");
		lookup.put(Long.class.getCanonicalName(), "BIGINT");
		lookup.put(Object.class.getCanonicalName(), "OBJECT");
		lookup.put(byte[].class.getCanonicalName(), "BINARY");
		lookup.put("default", "VARCHAR(32672)");
		return lookup;
	}

	public void disconnect() {
		super.disconnect();
		try {
			Statement statement = getConnection().createStatement();
			//statement.execute("SHUTDOWN SCRIPT");
			statement.execute("SHUTDOWN COMPACT");
		} catch (Exception e) {}
		hsqlServer.setErrWriter(null);
		hsqlServer.setLogWriter(null);
		//hsqlServer.stop(); 
	}

	protected String getGeneratedKeySyntax() {
		return "int IDENTITY";
	}


}
