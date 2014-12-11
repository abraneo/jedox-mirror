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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;

public class AccessConnection extends RelationalConnection {

	private static final Log log = LogFactory.getLog(AccessConnection.class);

	protected String getConnectionString(String parameter) {
		String connectionString = getProtocol();

		String[] odbcDrivers = null;
		String odbcDriver = "";
		try {
			odbcDrivers = getConfigurator().getParameter("OdbcDrivers", "").split("\\|");
		} catch (ConfigurationException e) {
			log.error("OdbcDrivers is not defined in Access Connection"+getName());
		}
		
		odbcDriver = getODBCDriver(odbcDrivers);
		connectionString += "Driver={"+odbcDriver+"};DBQ=";
		connectionString += getDatabaseString();
		if (parameter.length() > 0) {
			connectionString += ";"+parameter;
		}
		return connectionString;
	}

	private String getODBCDriver(String[] odbcDrivers) {

		for(String driver:odbcDrivers){
			try{
				Class.forName(getDriver());
				String connectionString = getProtocol();
				driver = driver.trim();
				connectionString += "Driver={"+driver+"};DBQ=";
				connectionString += getDatabaseString();
				DriverManager.getConnection(connectionString,getUser(), getPassword());
				log.info("Using driver " + driver + " for access connection " + getName()); 
				return driver;
			}catch(Exception e){
				log.debug(e.getMessage());
			}
		}
		return odbcDrivers[0];

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
