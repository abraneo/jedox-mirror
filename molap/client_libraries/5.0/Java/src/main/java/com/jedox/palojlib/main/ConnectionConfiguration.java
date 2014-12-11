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

import com.jedox.palojlib.interfaces.IConnectionConfiguration;

public class ConnectionConfiguration implements IConnectionConfiguration{

	private String host;
	private String port;
	private String username;
	private String password;
	private int timeout;
	private boolean sslPreferred;
	public String contextId;
	private ClientInfo clientInfo;
	private StringBuilder loginRequest = null;
	
	public ConnectionConfiguration(){
		this.contextId = "config_" + this.hashCode();
	}

	public String getContextId() {
		return this.contextId;
	}

	public String getHost() {
		return this.host;
	}

	public String getPort() {
		return this.port;
	}

	public String getUsername() {
		return this.username;
	}

	public String getPassword() {
		return this.password;
	}

	public int getTimeout() {
		return this.timeout;
	}

	public boolean isSslPreferred() {
		return this.sslPreferred;
	}

	public void setHost(String host) {
		this.host = host;
	}

	public void setPort(String port) {
		this.port = port;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public void setTimeout(int timeout) {
		this.timeout = timeout;
	}

	public void setSslPreferred(boolean sslPreferred) {
		this.sslPreferred = sslPreferred;
	}

	@Override
	public ClientInfo getClientInfo() {
		return clientInfo;
	}

	@Override
	public void setClientInfo(ClientInfo info) {
		this.clientInfo = info;
	}

	public void setLoginRequest(StringBuilder loginRequest) {
		this.loginRequest = loginRequest;
	}

	public StringBuilder getLoginRequest() {
		return loginRequest;
	}

}
