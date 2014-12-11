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
package com.jedox.palojlib.main;

import java.util.HashMap;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;

public class Connection implements IConnection {

	protected boolean isConnected;
	private ConnectionHandler connectionHandler;
	protected final ConnectionConfiguration connectionConfiguration;

	/**/
	HashMap<String, Database> databases = new HashMap<String, Database>();
	private ConnectionInfo info;

	public ConnectionConfiguration getConnectionConfiguration() {
		return connectionConfiguration;
	}

	protected Connection(IConnectionConfiguration connectionConfiguration) throws PaloException, PaloJException {

		this.connectionConfiguration = (ConnectionConfiguration) connectionConfiguration;
		connectionHandler = new ConnectionHandler((ConnectionConfiguration) connectionConfiguration);
		this.info = connectionHandler.getServerInfo();
		this.info.token = -1;// this will be used as flag to check if databases (cache) are read or not 
		this.isConnected = false;
	}

	public boolean isConnected() {
		return isConnected;
	}

	public ConnectionInfo getServerInfo() {
		return getServerConnectionInfo();
	}

	private void reinitCache(ComponentInfo serverInfo) throws PaloException, NumberFormatException, PaloJException {
		databases = connectionHandler.getDatabases();
		this.info = (ConnectionInfo)serverInfo;
	}

	private ValidationResult validateCache() throws PaloException, PaloJException {
		if(!isConnected()){
			throw new PaloJException("Connection is not open, please call connection.open()");
		}
		
		return new ValidationResult(this.info,getServerConnectionInfo());
	}

	private ConnectionInfo getServerConnectionInfo() throws PaloException, PaloJException{
		
		ConnectionInfo info = null;
		try {
			info = (ConnectionInfo)this.info.clone();
		} catch (CloneNotSupportedException e) {
			throw new PaloJException(e.getMessage());
		}
		info.token = connectionHandler.getServerToken();
		return info;
	}

	public void open() throws RuntimeException {
		try {
			connectionHandler.open();
		} catch (Exception e) {
			throw new RuntimeException("Could not connect to OLAP server at host '"
							+ connectionConfiguration.getHost() + "' as user '"
							+ connectionConfiguration.getUsername() + "', " + e.getMessage());
		}

		this.isConnected = true;
	}

	public void close() throws PaloException {
		connectionHandler.close();
		this.isConnected = false;
	}

	public Database[] getDatabases() throws PaloException, PaloJException {
		
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}
		return databases.values().toArray(new Database[databases.size()]);
	}

	public Database addDatabase(String name) throws PaloException, PaloJException {
		
		connectionHandler.addDatabase(name);
		reinitCache(getServerConnectionInfo());
		return getDatabaseByName(name);
	}

	public Database getDatabaseByName(String name) throws PaloException, PaloJException{
		
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}
		return databases.get(name.toLowerCase());
	}

	public void save() throws PaloException, PaloJException {
		Database[] dbs = getDatabases();
		for (Database db : dbs) {
			db.save();
		}
	}

	public SvsInfo getSvsInfo() throws PaloException, PaloJException {
		ValidationResult result = validateCache();
		/*if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}*/
		return connectionHandler.getSvsInfo();
	}

}
