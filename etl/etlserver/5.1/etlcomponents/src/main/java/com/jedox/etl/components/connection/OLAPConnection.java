/**
#*   @brief <Description of Class>
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
package com.jedox.etl.components.connection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Level;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.AbstractOLAPConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.palojlib.premium.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionInfo;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.premium.main.ConnectionManager;
import com.jedox.palojlib.main.ClientInfo;

public class OLAPConnection extends AbstractOLAPConnection implements IOLAPConnection {

	private static final Log log = LogFactory.getLog(OLAPConnection.class);
	protected IConnection connection;
	private boolean sslPreferred;	
	private boolean alwaysStopWithLogout; 
	
	/* version info no longer required as olap version check is done in palojlib */		
	private int versionMajor=0;
	private int versionMinor=0;
	private int versionBuild=0;
	//private ProxySelector originalProxySelector;

	protected IConnectionConfiguration getConnectionConfiguration() {
		IConnectionConfiguration cc = new ConnectionConfiguration();
		cc.setHost(getHost());
		cc.setPort(getPort());
		cc.setUsername(getUser());
		cc.setPassword(getPassword());
		cc.setTimeout(getTimeout()*1000);
		cc.setSslPreferred(sslPreferred);
		cc.setClientInfo(new ClientInfo("Jedox ETL","connection " + getName() + " in " + getLocator().getRootName()) );
		return cc;
	}

	/**
	 * Internal connection creation
	 * @param prop
	 * @return
	 */
	protected IConnection connect2OLAP() throws RuntimeException {
		try {
			setProxyIfSpecified();
			log.debug("Opening connection "+getName());
			IConnectionConfiguration connConfig = getConnectionConfiguration();
			log.debug("OLAP Connection Timeout set to "+getTimeout()+" seconds.");
			connection = ConnectionManager.getInstance().getConnection(connConfig);
			httpsPort = connection.getServerInfo().getHttpsPort();
			checkSSL();
			connection.openInternal();
			log.debug("Connection "+getName()+" is open.");
			setVersion(connection);
			return connection;
		} catch (Exception e) {
			throw new RuntimeException("Failed to open OLAP connection "+getName()+". Please check your connection specification. "+e.getMessage());
		}
	}

	public com.jedox.palojlib.interfaces.IConnection open() throws RuntimeException {
		if ((connection == null) || !connection.isConnected())
			connection = connect2OLAP();
		return connection;
	}

	public void close() {
		log.debug("Closing connection "+getName());
		try {
			if (!isKept() && (connection != null)) {
				// if it is closing as a result of a stop call, then it should call it with type=1
				if(alwaysStopWithLogout || getContext().getState().getStatus().equals(Codes.statusStopping) || 
						getContext().getState().getStatus().equals(Codes.statusInterrupting))
					connection.close(true);
				else 
					connection.close(false);
				connection = null;
				super.close();
			}
		}
		catch (Exception e) {};
	}

	public void setVersion(IConnection conn) {
		if (conn !=null) {
			IConnectionInfo info = conn.getServerInfo();
			versionMajor  = Integer.parseInt(info.getMajorVersion());
			versionMinor  =  Integer.parseInt(info.getMinorVersion().toString());
			versionBuild  =  Integer.parseInt(info.getBuildNumber().toString());
		}
	}


	public String getServerName() {
		return "palo";
	}

	public void test() throws RuntimeException {
		super.test();
		/* uncomment this to force database to be existent in palo server */
		if (open().getDatabaseByName(getDatabase()) == null)
			log.warn("Database "+getDatabase()+" not found in connection "+getName()+
					 ". If the database is created in a Dimension-Load, the warning can be ignored.");
	}

	public void init() throws InitializationException {
		try {
			super.init();
			Level level = (Boolean.parseBoolean(getParameter("debug","false"))== true?Level.DEBUG:Level.INFO);
			com.jedox.palojlib.managers.LoggerManager.getInstance().setLevel(level);
			sslPreferred = Boolean.parseBoolean(getParameter("sslPreferred","false"));
			alwaysStopWithLogout = Boolean.parseBoolean(getParameter("alwaysStopWithLogout","true"));
			
			try {
				com.jedox.palojlib.http.SocketManager.getInstance().setActive(Boolean.parseBoolean(getParameter("reuseSockets","true")));
			} catch (Exception e) {
				log.debug(e.getMessage());
			}
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

/*  version info no longer required as olap version check is done in palojlib */	
	public boolean isOlderVersion(int major, int minor, int buildNumber) {
		return ( (major>versionMajor) ||
				 (( major==versionMajor) && (minor>versionMinor)) ||
				 (( major==versionMajor) && (minor==versionMinor) && (buildNumber>versionBuild)) );
	}


}
