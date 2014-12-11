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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.persistence.generic.adaptor;

import java.math.BigDecimal;
import java.sql.Timestamp;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.Statement;
import java.sql.SQLException;
import java.sql.Time;
import java.util.Hashtable;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

public class Oracle extends GenericPersistor {

	//external connection
    private IRelationalConnection connection;

    public Oracle(IConnection connection) {
    	if (connection instanceof IRelationalConnection)
    		this.connection = (IRelationalConnection)connection;
    }

	@Override
	protected Connection setupConnection() throws RuntimeException {
		if (connection != null) {
			Connection conn =  connection.open();
			try {
				PreparedStatement changeSession = conn.prepareStatement("ALTER SESSION SET NLS_NUMERIC_CHARACTERS='.,'");
				changeSession.execute();
			} catch (SQLException e) {
				getLog().warn("Could not change the session parameter NLS_NUMERIC_CHARACTERS, " + e.getMessage());
			}
			return conn;
		}
		return null;
	}

	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "VARCHAR2(4000)");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "FLOAT");
		lookup.put(Float.class.getCanonicalName(), "REAL");
		lookup.put(java.sql.Date.class.getCanonicalName(), "DATE");
		lookup.put(java.util.Date.class.getCanonicalName(), "DATE");
		lookup.put(Time.class.getCanonicalName(), "TIMESTAMP");
		lookup.put(Timestamp.class.getCanonicalName(), "TIMESTAMP");
		lookup.put(BigDecimal.class.getCanonicalName(), "NUMERIC");
		lookup.put(Boolean.class.getCanonicalName(), "BOOLEAN");
		lookup.put(Byte.class.getCanonicalName(), "SMALLINT");
		lookup.put(Short.class.getCanonicalName(), "SMALLINT");
		lookup.put(Long.class.getCanonicalName(), "BIGINT");
		lookup.put(Object.class.getCanonicalName(), "BLOB");
		lookup.put(byte[].class.getCanonicalName(), "BLOB");
		lookup.put("default", "VARCHAR2(4000)");
		return lookup;
	}

	protected void createInsertTrigger(String persistentName, String columnName) throws RuntimeException {
		try {
			Statement stm = getConnection().createStatement();

			//get only table Name in case it has a schema appended to it
			//String table_Name = persistentName.substring(persistentName.lastIndexOf('.')+1);
			String tName = String.valueOf("counter"+persistentName.hashCode());
			tName = tName.replaceAll("-", "");
			try {
				stm.execute("DROP SEQUENCE "+tName+"_sequence");
			}
			catch (SQLException e) {
				getLog().debug("Sequence "+tName+" does not exist. Creating it.");
			}

			stm.execute("CREATE SEQUENCE "+tName+"_sequence START WITH 1 INCREMENT BY 1");
			getLog().debug("CREATE SEQUENCE "+tName+"_sequence START WITH 1 INCREMENT BY 1");
			stm.execute("CREATE OR REPLACE TRIGGER "+tName+"_trigger BEFORE INSERT ON "+persistentName+" FOR EACH ROW BEGIN SELECT "+tName+"_sequence.NEXTVAL INTO :NEW."+columnName+" FROM DUAL; END;");
			getLog().debug("CREATE OR REPLACE TRIGGER "+tName+"_trigger BEFORE INSERT ON "+persistentName+" FOR EACH ROW BEGIN SELECT "+tName+"_sequence.NEXTVAL INTO :NEW."+columnName+" FROM DUAL; END;");
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to create sequence and trigger for "+persistentName+": "+e.getMessage());
		}
	}

	protected String getGeneratedKeySyntax() {
		return "NUMBER PRIMARY KEY";
	}


}
