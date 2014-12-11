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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.core.persistence.generic.adaptor;

import java.io.File;
import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.Date;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Time;
import java.sql.Timestamp;
import java.util.Hashtable;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

/**
 * @author khaddadin
 *
 */
public class Sqlite extends GenericPersistor {
	
	private IConnection connection;
	private static final Log log = LogFactory.getLog(Sqlite.class);
	private static String dataDir;
	
	public Sqlite(IConnection connection){
		this.connection = connection;
	}

	@Override
	protected Connection setupConnection() throws RuntimeException {
		//System.setProperty("sqlite.purejava", "false");
		if (connection instanceof IRelationalConnection){
			Connection conn = ((IRelationalConnection)connection).open();
			dataDir = connection.getDatabase().substring(0, connection.getDatabase().lastIndexOf("\\"));
			return conn;
		}
		else
			throw new RuntimeException("Sqlite connection could not be done.");
		
		
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
	
	protected void dropSchemaInternal(String name) throws RuntimeException {
		Statement statement;
		try {
			getConnection().setAutoCommit(true);
			statement = getConnection().createStatement();
			statement.execute("DETACH DATABASE "+name);
			statement.close();
			getConnection().setAutoCommit(false);
		} catch (SQLException e) {
			log.debug("Failed to drop schema "+name+": "+e.getMessage());
		}
	}
	
	protected void createSchemaInternal(String name) throws SQLException, RuntimeException {

		getConnection().setAutoCommit(true);
		Statement statement = getConnection().createStatement();
		statement.execute("ATTACH DATABASE \""+ dataDir + File.separatorChar+ name + ".db\""  + " AS " + name + ";");
		statement.close();
		getConnection().setAutoCommit(false);
	}

	@Override
	protected String getGeneratedKeySyntax() {
		return "INTEGER PRIMARY KEY ";
	}

	@Override
	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "TEXT");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "REAL");
		lookup.put(Date.class.getCanonicalName(), "REAL");
		lookup.put(Time.class.getCanonicalName(), "INTEGER");
		lookup.put(Timestamp.class.getCanonicalName(),"INTEGER");
		lookup.put(BigDecimal.class.getCanonicalName(),"INTEGER");
		lookup.put(Boolean.class.getCanonicalName(), "TEXT");
		lookup.put(Byte.class.getCanonicalName(), "INTEGER");
		lookup.put(Short.class.getCanonicalName(), "INTEGER");
		lookup.put(Long.class.getCanonicalName(), "INTEGER");
		lookup.put(Float.class.getCanonicalName(), "REAL");	
		lookup.put(Object.class.getCanonicalName(), "TEXT");
		lookup.put(byte[].class.getCanonicalName(), "TEXT");
		lookup.put("default", "TEXT");
		return lookup;
	}
}
