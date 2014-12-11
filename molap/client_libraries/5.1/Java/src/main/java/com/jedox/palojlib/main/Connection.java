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
 *   You may obtain a copy of the License at
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
import com.jedox.palojlib.interfaces.IDatabase;

public class Connection implements IConnection {

	protected boolean isConnected;
	private ConnectionHandler connectionHandler;
	protected final ConnectionConfiguration connectionConfiguration;

	/**/
	HashMap<String, Database> databases = null;
	private ConnectionInfo info = null;

	public ConnectionConfiguration getConnectionConfiguration() {
		return connectionConfiguration;
	}

	protected Connection(IConnectionConfiguration connectionConfiguration) throws PaloException, PaloJException {

		this.connectionConfiguration = (ConnectionConfiguration) connectionConfiguration;
		connectionHandler = new ConnectionHandler((ConnectionConfiguration) connectionConfiguration);
		this.info = connectionHandler.getServerInfo();
		this.isConnected = false;
	}

	public boolean isConnected() {
		return isConnected;
	}

	public ConnectionInfo getServerInfo() {
		return getServerConnectionInfo();
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

	public String open() throws RuntimeException {
		String session = null;
		try {
			session = connectionHandler.open();
			
		} catch (Exception e) {
			throw new RuntimeException("Could not connect to OLAP server at host '"
							+ connectionConfiguration.getHost() + "' as user '"
							+ connectionConfiguration.getUsername() + "', " + e.getMessage());
		}

		this.isConnected = true;
		return session;
	}

	public void close() throws PaloException {
		close(false);
	}
	
	public void close(boolean stop) throws PaloException {
		connectionHandler.close(stop);
		this.isConnected = false;
		if(databases!=null){
			for(Database db:databases.values()){
				db.resetCaches();
			}
			databases.clear();
			this.databases = null;
		}
	}

	public Database[] getDatabases() throws PaloException, PaloJException {
		
		checkCache();
		return databases.values().toArray(new Database[databases.size()]);
	}

	public Database addDatabase(String name) throws PaloException, PaloJException {
		
		connectionHandler.addDatabase(name);
		checkCache();
		return databases.get(name.toLowerCase());
	}

	public Database getDatabaseByName(String name) throws PaloException, PaloJException{
		
		checkCache();
		return databases.get(name.toLowerCase());
	}

	public void save() throws PaloException, PaloJException {
		Database[] dbs = getDatabases();
		for (Database db : dbs) {
			db.save();
		}
	}

	private void checkCache() throws PaloException, PaloJException {
		if(!isConnected()){
			throw new PaloJException("Connection is not open, please call connection.open()");
		}
		
		int oldToken = info.getToken();
		int newToken = connectionHandler.getServerToken();
		
		if(!cacheExists() || oldToken!=newToken){
			this.info.token = newToken;
			databases = connectionHandler.getDatabases();
		}
	}
	
	private boolean cacheExists(){
		return (databases!=null);
	}
	
	public void resetCache(){
		databases = null;
	}

	public SvsInfo getSvsInfo() throws PaloException, PaloJException {
		return connectionHandler.getSvsInfo();
	}

	public UserInfo getUserInfo(boolean withPermission) throws PaloException, PaloJException {
		return connectionHandler.getUserInfo(withPermission);
	}
	
	@Override
	public void removeDatabase(IDatabase db) throws PaloException, PaloJException {
		connectionHandler.removeDatabase(((Database)db).getId());
	}

}
