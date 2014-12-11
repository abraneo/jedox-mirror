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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.ConnectException;
import java.net.UnknownHostException;
import java.sql.SQLException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.connection.AbstractOLAPConnection;
import com.jedox.etl.core.olap4j.ConnectionWrapper;
import com.jedox.etl.core.util.SSLUtil.SSLModes;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;



public class OLAPXmlaConnection extends AbstractOLAPConnection implements IOLAPConnection {
	
	private static final Log log = LogFactory.getLog(OLAPXmlaConnection.class);

	private ConnectionWrapper connection;
	
	protected String getCatalog() {
		return getDatabase();
	}
	
	protected ConnectionWrapper connect2OLAP() throws RuntimeException {
		try {			
			setProxyIfSpecified();
			log.debug("Opening connection "+getName());
			checkSSL();
			//CSCHW: Assignment of custom keystore
			/*System.setProperty("javax.net.ssl.trustStore",Settings.getConfigDir()+File.separator+"keystore");
			System.setProperty("javax.net.ssl.keyStore",Settings.getConfigDir()+File.separator+"keystore");
			System.setProperty("javax.net.ssl.keyStorePassword","password");
			System.setProperty("javax.net.ssl.keyStoreType","JKS");
			System.setProperty("javax.net.debug", "all");
			System.setProperty("java.security.debug", "all");
		    */

			connection = new ConnectionWrapper(getHost(),getPort(),getUser(),getPassword(),getCatalog());
			connection.open();
			
			log.debug("Connection "+getName()+" is open.");
			
			return connection;
		} catch (Exception e) {
			throw new RuntimeException("Failed to open OLAP4J connection "+getName()+". Please check your connection specification. "+e.getMessage());
		}
	}
	
	
	@SuppressWarnings("restriction")
	public ConnectionWrapper open() throws RuntimeException {
		try {
			if ((connection == null) || !connection.getOlapConnection().isClosed()) { 
				connection = connect2OLAP();
				try { //test access to connection
					connection.getOlapConnection().getCatalog();
				}
				catch (Exception e) {
					Throwable cause = e;
					String message = cause.getMessage();
					while (cause.getCause() != null) cause=cause.getCause();
					if (cause instanceof UnknownHostException) {
						message = "Host "+cause.getMessage()+" is unknown.";
					}  
					else if (cause instanceof IOException && cause.getMessage().contains("401")) {
						message = "Please check username, password and the connection url: "+cause.getMessage();
					}
					else if (cause instanceof IOException && cause.getMessage().contains("HTTPS hostname wrong")) {
						message = "Host name validation failed. Please install a valid certificate on host: "+cause.getMessage();
					}
					else if (cause instanceof FileNotFoundException && getSSLMode().equals(SSLModes.verify)) {
						message = "SSL-Keystore file not found: " +cause.getMessage();
					}
					else if (cause instanceof sun.security.provider.certpath.SunCertPathBuilderException) {
						message = "Cannot verify ssl certificate path - consider using ssl=trust: "+cause.getMessage();
					}
					else if (cause instanceof java.security.UnrecoverableKeyException) {
						message = "Wrong keystore passord: "+cause.getMessage();
					}
					else if (cause instanceof ConnectException) {
						message = "Cannot connect to XMLA Provider: "+cause.getMessage();
					}
					else if (cause instanceof java.security.cert.CertificateException && cause.getMessage().contains("No subject alternative DNS name matching")) {
						message = "Host name validation failed. Please install a valid certificate on host: "+cause.getMessage();
					} else {
						message = message +": "+ cause.getMessage();
						// e.printStackTrace();
					}
					throw new RuntimeException(message);
				}
			}
		} catch (SQLException e) {
			throw new RuntimeException(e.getMessage());
		}
		return connection;
	}

	public void close() {
		log.debug("Closing connection "+getName());
		try {
			if (!isKept() && (connection != null)) {
				connection.close();
				connection = null;
				super.close();
			}
		}
		catch (Exception e) {};
	}
	

	@Override
	public boolean isOlderVersion(int Major, int Minor, int BuildNumber) {
		return false;
		/* TODO: clarify the need.
		try {
			if (connection == null) open();
			return ( (Major>Integer.parseInt(connection.getServerInfo().getMajorVersion())) ||
					 (( Major==Integer.parseInt(connection.getServerInfo().getMajorVersion())) && (Minor>Integer.parseInt(connection.getServerInfo().getMinorVersion()))) ||
					 (( Major==Integer.parseInt(connection.getServerInfo().getMajorVersion())) && (Minor==Integer.parseInt(connection.getServerInfo().getMinorVersion())) && (BuildNumber>Integer.parseInt(connection.getServerInfo().getBuildNumber()))));
		}
		catch (Exception e) {
			return false;
		}
		*/
	}
	
	public void test() throws RuntimeException {
		super.test();
		/* uncomment this to force database to be existent in palo server */
		getDatabase(false,true);
	}
	
	public String getServerName() {
		return "XMLA";
	}
	
}
