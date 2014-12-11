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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;


import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.NamingUtil;


public class H2Connection extends RelationalConnection {

	protected String getConnectionString(String parameter) {
		String connectionString = super.getConnectionString(parameter);

		if(connectionString.endsWith(";"))
			connectionString = connectionString.substring(0,connectionString.length()-1);
		
		return connectionString;
	}
	
	protected String getDataDir() {
		if (super.getDatabaseString().startsWith("$"))
			return Settings.getInstance().getPersistenceDir();
		else return super.getDataDir();
	}
	
	protected String getDatabaseString() {
		if (super.getDatabaseString().startsWith("$"))
			return super.getDatabaseString().substring(1);
		else return super.getDatabaseString();
	}
	
	public void init() throws InitializationException {
		super.init();
		//drop database if needed
		if (FileUtil.isRelativ(getDatabaseString()) && getConnectionParameters().getProperty(NamingUtil.internal("drop"), "false").equalsIgnoreCase("true")) {
			FileUtil.delete(getDatabase()+".h2.db");
			FileUtil.delete(getDatabase()+".trace.db");
		}
	}
	
	public synchronized java.sql.Connection open() throws RuntimeException {
		java.sql.Connection connection = super.open(); 
		//CSCHW: Register extension aggregate functions. Registry will fail, if already present.
		try {
			connection.createStatement().execute("CREATE AGGREGATE FIRST FOR \"com.jedox.etl.components.connection.extensions.H2AggregateFirst\"");
		}
		catch (Exception e) {}
		try {
			connection.createStatement().execute("CREATE AGGREGATE LAST FOR \"com.jedox.etl.components.connection.extensions.H2AggregateLast\"");
		}
		catch (Exception e) {}
		return connection;
	}
}
