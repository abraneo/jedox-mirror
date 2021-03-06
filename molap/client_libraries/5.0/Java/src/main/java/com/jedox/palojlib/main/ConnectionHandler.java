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
import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.http.HttpHandler;
import com.jedox.palojlib.interfaces.IDatabase.DatabaseType;

public class ConnectionHandler {

	protected final ConnectionConfiguration connectionConfiguration;
	private static final String SERVER_INFO_REQUEST = "/server/info";
	private static final String SVS_INFO_REQUEST = "/svs/info?";
	private static final String LIST_DATABASE_REQUEST = "/server/databases?";
	private static final String CREATE_DATABASE_REQUEST = "/database/create?";

	public ConnectionHandler(ConnectionConfiguration connectionConfiguration) throws PaloException, PaloJException {
		
		if(!HttpHandlerManager.getInstance().isInitialized(connectionConfiguration))
			HttpHandlerManager.getInstance().initHttpHandler(connectionConfiguration);
		this.connectionConfiguration = connectionConfiguration;
	}

	protected int getServerToken() throws PaloException {

		StringBuilder SERVER_INFO_REQUEST_BUFFER = new StringBuilder(
				SERVER_INFO_REQUEST);
		StringBuilder currentRequest = SERVER_INFO_REQUEST_BUFFER;
		String[][] response = HttpHandlerManager.getInstance().getHttpHandler(connectionConfiguration.getContextId()).send(currentRequest, false, true);
		return Integer.parseInt(response[0][0]);
	}

	public ConnectionInfo getServerInfo() throws PaloException {

		StringBuilder SERVER_INFO_REQUEST_BUFFER = new StringBuilder(
				SERVER_INFO_REQUEST);
		StringBuilder currentRequest = SERVER_INFO_REQUEST_BUFFER;
		// special case, this should be possible even without calling open()
		HttpHandler handler = new HttpHandler(this.connectionConfiguration,false,0,0,0);;
		//handler.setSessionId(); this does not work if ssl is required
		String[][] response = handler.send(currentRequest, false, false);
		return new ConnectionInfo(response[0][0], response[0][1], response[0][2], response[0][3], response[0][4], response[0][5]);
	}

	protected HashMap<String, Database> getDatabases() throws PaloException, NumberFormatException, PaloJException {

		StringBuilder LIST_DATABASE_REQUEST_BUFFER = new StringBuilder(
				LIST_DATABASE_REQUEST);
		StringBuilder currentRequest = LIST_DATABASE_REQUEST_BUFFER
				.append("show_normal=1&show_system=1&show_user_info=1");
		String[][] responses = HttpHandlerManager.getInstance().getHttpHandler(connectionConfiguration.getContextId()).send(currentRequest, true, false);
		HashMap<String, Database> databases = new HashMap<String, Database>();
		DatabaseType type = DatabaseType.DATABASE_NORMAL;
		for (int i = 0; i < responses.length; i++) {
			switch(Integer.parseInt(responses[i][5])){
				case 0:
					type = DatabaseType.DATABASE_NORMAL;
					break;
				case 1:
					type = DatabaseType.DATABASE_SYSTEM;
					break;
				case 3:
					type = DatabaseType.DATABASE_USERINFO;
					break;
				default:
					throw new PaloJException(responses[i][5] + " is not a possible value for database type.");
			}
				
			databases.put(responses[i][1].toLowerCase(), new Database(connectionConfiguration.getContextId(), Integer.parseInt(responses[i][0]),responses[i][1], type,responses[i][2],responses[i][3],responses[i][4],responses[i][6]));
		}
		return databases;
	}

	protected void addDatabase(String name) throws PaloException {

		StringBuilder CREATE_DATABASE_REQUEST_BUFFER = new StringBuilder(
				CREATE_DATABASE_REQUEST);
		StringBuilder currentRequest = CREATE_DATABASE_REQUEST_BUFFER.append("new_name=").append(name).append("&type=0");
		HttpHandlerManager.getInstance().getHttpHandler(connectionConfiguration.getContextId()).send(currentRequest, true, false);
	}

	public void open() throws PaloException {
		HttpHandlerManager.getInstance().getHttpHandler(connectionConfiguration.getContextId());
	}

	public void close() throws PaloException {
		HttpHandlerManager.getInstance().getHttpHandler(connectionConfiguration.getContextId()).resetSessionId();
		HttpHandlerManager.getInstance().removeHttpHandler(this.connectionConfiguration);
	}

	public SvsInfo getSvsInfo() {
		
		StringBuilder SVS_INFO_REQUEST_BUFFER = new StringBuilder(
				SVS_INFO_REQUEST);

		String[][] responses = HttpHandlerManager.getInstance().getHttpHandler(connectionConfiguration.getContextId()).send(SVS_INFO_REQUEST_BUFFER, true, false);
		return new SvsInfo(responses[0][0], responses[0][1], responses[0][2], responses[0][3], responses[0][4]);
		
	}
}
