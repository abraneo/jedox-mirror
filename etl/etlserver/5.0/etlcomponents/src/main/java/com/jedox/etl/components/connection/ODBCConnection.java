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
package com.jedox.etl.components.connection;

import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;

public class ODBCConnection extends RelationalConnection {

	private static final Log log = LogFactory.getLog(ODBCConnection.class);

	protected String getConnectionString(String parameter) {
		String connectionString = getProtocol();
		connectionString += getDatabaseString();
		if (parameter.length() > 0) {
			connectionString += ";"+parameter;
		}
		return connectionString;
	}
	
	protected java.sql.Connection connect2Relational() throws RuntimeException{
		String connectionString = getConnectionUrl();
		try {
			log.debug("Opening connection "+getName());
			//load driver class into memory.
			Class.forName(getDriver());
			// Character encoding set via component.xml 
			try {
				String charset=getConfigurator().getParameter("charSet", "");
				if (!charset.isEmpty()) {
					log.info("Set charset to: "+charset+" in connection "+getName());
					getConnectionParameters().setProperty("charSet",charset);
				}	
			} catch (ConfigurationException e) { 
				throw new RuntimeException("Could not set charset: "+e.getMessage());
			}		
			Properties properties = new Properties();
			properties.putAll(getConnectionParameters());
			properties.setProperty("user", getUser());
			properties.setProperty("password", getPassword());
			
			java.sql.Connection connection =  DriverManager.getConnection(connectionString,properties);
			log.debug("Connection "+getName()+" is open: "+connectionString);
			return connection;
		} catch (ClassNotFoundException cnfe) {
			throw new RuntimeException("Class not found: "+getDriver()+": "+cnfe.getMessage());
		} catch (SQLException sqle) {
			throw new RuntimeException("Failed to open Relational connection "+getName()+" with URL "+connectionString+". Please check your connection specification. "+sqle.getMessage());
		}
	}


	public Statement createStatement(int size) throws SQLException, RuntimeException {
		Statement statement = open().createStatement();
	    try {
	    	statement.setMaxRows(size);
		} catch (SQLException e) {
			log.warn("The advanced option SQL_MAX_ROWS is not supported by the ODBC driver of connection " + getName() + ". The performance may be poor:" +  e.getMessage());
		}
		return statement;
	}

	public void close() {
	   try {
		connection.setAutoCommit(true);
	} catch (SQLException e) {
		log.debug("Error while setting Autocommit back to true:" + e.getMessage());
	}
		super.close();
	}

}
