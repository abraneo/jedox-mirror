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
package com.jedox.palojlib.managers;

import com.jedox.palojlib.main.ConnectionConfiguration;

/**
 * @author khaddadin
 *
 */
/**
 * extends the ConnectionConfiguration to manage SSL communication and server information (needed sometimes to see which request syntax is possible)
 * @author khaddadin
 *
 */
public final class HttpHandlerInfo extends ConnectionConfiguration{
	
	public boolean useSsl = false;
	public int sslPort = 0;
	public int majorVersion = 0; 
	public int minorVersion = 0;
	private StringBuilder loginRequest = null;
	public int buildNumber = 0;
	private boolean resendRequestIfError = true;

	public HttpHandlerInfo(ConnectionConfiguration config){
		overwrite(config);
	}
	
	protected void overwrite(ConnectionConfiguration config){
		setHost(config.getHost());
		setPort(config.getPort());
		setUsername(config.getUsername());
		setPassword(config.getPassword());
		setTimeout(config.getTimeout());
		setSslPreferred(config.isSslPreferred());
		setContextId(config.getContextId());
		setClientInfo(config.getClientInfo());
	}

	/**
	 * @return the loginRequest
	 */
	public StringBuilder getLoginRequest() {
		return loginRequest;
	}

	/**
	 * @param loginRequest the loginRequest to set
	 */
	public void setLoginRequest(StringBuilder loginRequest) {
		this.loginRequest = loginRequest;
	}

	/**
	 * get whether the request should be resent if error occurs
	 * @param resendRequestIfError
	 */
	public boolean isResendRequestIfError() {
		return resendRequestIfError;
	}

	/**
	 * set whether the request should be resent if error occurs
	 * @param resendRequestIfError
	 */
	public void setResendRequestIfError(boolean resendRequestIfError) {
		this.resendRequestIfError = resendRequestIfError;
	}
	
}
