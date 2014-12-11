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

import java.io.File;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.NamingUtil;

public class DerbyConnection extends RelationalConnection {
	
	protected String getDataDir() {
		if (super.getDatabaseString().startsWith("$"))
			return Settings.getInstance().getPersistenceDir();
		else return super.getDataDir();
	}
	
	protected String getConnectionString(String parameter) {
		String connectionString = getProtocol();
		if (getHost() != null) connectionString += "//"+getHost();
		if (getPort() != null) connectionString += ":"+getPort();
		if (getHost() != null) connectionString += "/";
		connectionString += getDatabase();
		if (parameter.length() > 0) {
			connectionString += ";"+parameter;
		}
		return connectionString;
	}
	
	public String getConnectionUrl() throws RuntimeException {
		//check if we have to use the create parameter for a file database
		//if (FileUtil.isRelativ(getDatabaseString()))
		if (!getConnectionParameters().containsKey("create"))
			getConnectionParameters().setProperty("create", String.valueOf(!(new File(getDatabase()).exists())));
		String url = super.getConnectionUrl();
		getConnectionParameters().remove("create");//this should be checked each time we call getConnectionUrl
		return url;
	}
	
	protected String getDatabaseString() {
		if (super.getDatabaseString().startsWith("$"))
			return super.getDatabaseString().substring(1);
		else return super.getDatabaseString();
	}
/*	
	public String getServerName() {
		return "derby";
	}
*/	
	public void init() throws InitializationException {
		super.init();
		//drop database if needed
		if (FileUtil.isRelativ(getDatabaseString()) && getConnectionParameters().getProperty(NamingUtil.internal("drop"), "false").equalsIgnoreCase("true")) {
			FileUtil.deleteDirectory(new File(getDatabase()));
		}
	}

}
