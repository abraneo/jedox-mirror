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
package com.jedox.etl.core.connection;

import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.connection.ConnectionConfigurator;

import java.util.Properties;
//import java.net.InetAddress;

/**
 * Basic abstract Class for unified Connection handling
 * Contains the connections metadata and the connection object
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Connection extends Component implements IConnection {
	private String driver;
	private String database;
	private String type;
	private String user;
	private String password;
	private String host;
	private String port;
	private String url;
	private String protocol;
	private String encoding;
	private FetchModes fetchMode;
	private String proxyHost;
	private int proxyPort;
	private ProxyTypes proxyType;
	private boolean hasProxy;
	private Properties parameters;
	private boolean keepOpen = false;
	private int timeout;

	public Connection() {
		setConfigurator(new ConnectionConfigurator());
	}

	public ConnectionConfigurator getConfigurator() {
		return (ConnectionConfigurator)super.getConfigurator();
	}

	/**
	 * gets the database accessed via this connection
	 */
	public String getDatabase() {
		return database;
	}

	protected String getHost() {
		return host;
	}

	protected String getPort() {
		return port;
	}

	public int getTimeout() {
		return timeout;
	}

	protected String getPassword() {
		return password;
	}

	protected void setPassword(String password) {
		this.password = password;
	}

	protected String getDBType() {
		return type;
	}

	protected String getUser() {
		return user;
	}

	protected void setUser(String user) {
		this.user = user;
	}

	protected String getURL() {
		return url;
	}

	protected String getProtocol() {
		return protocol;
	}

	public FetchModes getFetchMode() {
		return fetchMode;
	}

	protected Properties getConnectionParameters() {
		return parameters;
	}

	public void setDriver(String driver) {
		this.driver = driver;
	}

	public void setProtocol(String jdbc) {
		this.protocol = "jdbc:" + jdbc + ":";
	}

	protected String getDriver() {
		return driver;
	}

	public void keep(boolean keepOpen) {
		this.keepOpen = keepOpen;
	}

	protected boolean isKept() {
		return keepOpen;
	}

	public String getEncoding() {
		if (encoding != null) {
			return encoding;
		}
		return "UTF8";
	}

	public boolean hasProxy() {
		return hasProxy;
	}

	public String getProxyHost() {
		return proxyHost;
	}
	
	public int getProxyPort() {
		return proxyPort;
	}

	public ProxyTypes getProxyType() {
		return proxyType;
	}

	public void init() throws InitializationException {
		try {
			super.init();
			keep(getConfigurator().getParameter("#keep", "false").equalsIgnoreCase("true"));
			database = getConfigurator().getDatabase();
			type = getConfigurator().getType();
			user = getConfigurator().getUser();
			port = getConfigurator().getPort();
			password = getConfigurator().getPassword();
			timeout = getConfigurator().getTimeout();
			host = getConfigurator().getHost();
			url = getConfigurator().getURL();
			fetchMode = getConfigurator().getFetchMode();
			hasProxy = getConfigurator().hasProxy();
			if(hasProxy){
				proxyHost=getConfigurator().getProxyHost();
				proxyPort=getConfigurator().getProxyPort();
				proxyType=getConfigurator().getProxyType();
			}
			if(getConfigurator().getDriver() != "")
				driver = getConfigurator().getDriver();

			//this handles the case if the driver is given by the user
			// if not handled, then it should be already set
			String givenProtocol = getConfigurator().getProtocol();
			if( givenProtocol != null){
				protocol = givenProtocol;
			}
			//protocol = getConfigurator().getProtocol();
			parameters = getConfigurator().getOwnParameters();
			encoding = getConfigurator().getEncoding();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

	public String getServerName() {
		return "Unknown";
	}

	public int compareTo(IConnection connection) {
		return 0;
	}

	public void test() throws RuntimeException {
		super.test();
		open();
	}
	
	protected String getMetadataSelectorValues() {
		StringBuffer result = new StringBuffer();
		for (MetadataCriteria s : getMetadataCriterias())
			result.append(s.getName()+",");
		result.deleteCharAt(result.length()-1);
		return result.toString();
	}
	
	public MetadataCriteria getMetadataCriteria(String name) {
		for (MetadataCriteria c: getMetadataCriterias()) {
			if (c.getName().equals(name)) {
				return c;
			}
		}
		return null;
	}	
	
}
