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

package com.jedox.etl.core.config.connection;

import org.jdom.Element;
import org.jasypt.util.text.BasicTextEncryptor;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.IConnection.FetchModes;
import com.jedox.etl.core.connection.IConnection.ProxyTypes;
import com.jedox.etl.core.util.TypeConversionUtil;
/**
 * Basic Configurator class for all types of Connections
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ConnectionConfigurator extends BasicConfigurator {

	private static final Log log = LogFactory.getLog(BasicConfigurator.class);

	/**
	 * gets the host name of the connection
	 * @return the host name
	 * @throws ConfigurationException
	 */
	public String getHost() throws ConfigurationException {
		
		String host = getXML().getChildTextTrim("host");
		if(host!= null && !host.isEmpty()){
			return (host.split("@"))[0];
		}
		return host;
	}
	

	/**
	 * get the proxy host
	 * @return proxy host name
	 * @throws ConfigurationException
	 */
	public String getProxyHost() throws ConfigurationException {
		
		String[] proxyInfo = getXML().getChildTextTrim("host").split("@");
		if(proxyInfo.length == 1){
			return null;
		}
		if(proxyInfo.length>=2 && proxyInfo.length<=3){
			return proxyInfo[1].split(":")[0];
		}
		throw new ConfigurationException("host name is invalid, maximum two '@' are allowed to appear in the host name.");
	}
	
	/**
	 * get the proxy port
	 * @return proxy port
	 * @throws ConfigurationException
	 */
	public int getProxyPort() throws ConfigurationException {
		
		String[] proxyInfo = getXML().getChildTextTrim("host").split("@");
		if(proxyInfo.length == 1){
			return -1;
		}
		if(proxyInfo.length>=2 && proxyInfo.length<=3){
			String info[]= proxyInfo[1].split(":");
			if(info.length!=2)
				return -1;
			else{
				if(new TypeConversionUtil().isNumeric(info[1])){
					return Integer.parseInt(info[1]);
				}else{
					throw new ConfigurationException("Proxy port should be an integer value.");
				}
			}
		}
		throw new ConfigurationException("host name is invalid, maximum two '@' are allowed to appear in the host name.");
	}
	
	/**
	 * get the proxy host type
	 * @return proxy host type
	 * @throws ConfigurationException
	 */
	public ProxyTypes getProxyType() throws ConfigurationException {
		
		String[] proxyInfo = getXML().getChildTextTrim("host").split("@");
		if(proxyInfo.length == 1){
			return null;
		}
		if(proxyInfo.length == 2){
			return ProxyTypes.HTTP;
		}
		if(proxyInfo.length == 3){
			if(!(proxyInfo[2].toUpperCase().equals("HTTP") || proxyInfo[2].toUpperCase().equals("SOCKS"))){
				throw new ConfigurationException("proxy type can only have values {\"socks\",\"http\"}.");
			}
			if(ProxyTypes.valueOf(proxyInfo[2].toUpperCase()).equals(ProxyTypes.SOCKS)){
				return ProxyTypes.SOCKS;
			}else if(ProxyTypes.valueOf(proxyInfo[2].toUpperCase()).equals(ProxyTypes.HTTP)){
				return ProxyTypes.HTTP;
			}else{
				throw new ConfigurationException("Proxy type can only have values \"http\" or \"socks\".");
			}
		}
		throw new ConfigurationException("host name is invalid, maximum two '@' are allowed to appear in the host name.");
	}
	
	public boolean hasProxy() throws ConfigurationException {

		String host = getXML().getChildTextTrim("host");
		if(host!= null && !host.isEmpty()){
			String[] proxyInfo = getXML().getChildTextTrim("host").split("@");

			if(proxyInfo.length == 1){
				return false;
			}

			if(proxyInfo.length>=2 && proxyInfo.length<=3){
				return true;
			}

			throw new ConfigurationException("host name is invalid, maximum two '@' are allowed to appear in the host name.");
		}

		return false;
	}
	
	

	/**
	 * gets the port of the connection
	 * @return the port number
	 * @throws ConfigurationException
	 */
	public String getPort() throws ConfigurationException {
		return getXML().getChildTextTrim("port");
	}

	/**
	 * gets the username of the connection
	 * @return the username
	 * @throws ConfigurationException
	 */
	public String getUser() throws ConfigurationException {
		String user = getXML().getChildTextTrim("user");
		if (user == null) return "sa";
		return user;
	}

	/**
	 * gets the username of the connection
	 * @return the username
	 * @throws ConfigurationException
	 */
	public String getDriver() throws ConfigurationException {
		String driver = getXML().getChildTextTrim("driver");
		if (driver == null) return "";
		return driver;
	}

	/**
	 * gets the (decrypted) password for the given username of this connection
	 * @return the password
	 * @throws ConfigurationException
	 */
	public String getPassword() throws ConfigurationException {
		Element passwordElement = getXML().getChild("password");
		String password = "";
		if (passwordElement != null) {
			password = passwordElement.getTextTrim();
			if (passwordElement.getAttributeValue("encrypted","false").equalsIgnoreCase("true")) {
				try {
					BasicTextEncryptor crypt = new BasicTextEncryptor();
					crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));
					password = crypt.decrypt(password);
				}
				catch (Exception e) {
					log.debug("Decryption of password for connection "+getName()+" failed.");
//					password = " ";
				}
			}
		}
		return password;
	}

	/**
	 * gets the database accessed via this connection
	 * @return the database name
	 * @throws ConfigurationException
	 */
	public String getDatabase() throws ConfigurationException {
		String database = getXML().getChildTextTrim("database");
		return database;
	}
	
	/**
	 * gets the connection timout value in seconds
	 * @return the timeout value in seconds
	 * @throws ConfigurationException
	 */
	public int getTimeout() throws ConfigurationException {
		String timeout = getXML().getChildTextTrim("timeout");
		if(timeout!=null){
			return Integer.parseInt(getXML().getChildTextTrim("timeout"));
		}
		return Integer.parseInt(getParameter("Timeout","1200"));		
	}

	/**
	 * gets the protocol of this connection
	 * @return the protocol name
	 * @throws ConfigurationException
	 */
	public String getProtocol() throws ConfigurationException {
		String protocol = getXML().getChildTextTrim("driver");
		//if (protocol == null) protocol = "jdbc:"+getType()+":";
		return protocol;
	}

	/**
	 * gets the type of this connection mapping to the implementing class
	 * @return the connection type
	 * @throws ConfigurationException
	 */
	public String getType() throws ConfigurationException {
		String type = getXML().getAttributeValue("type","unknown");
		return type;
	}

	/**
	 * gets the URL of this connection
	 * @return the connection URL
	 * @throws ConfigurationException
	 */
	public String getURL() throws ConfigurationException {
		return getXML().getChildTextTrim("url");
	}

	/**
	 * gets the encoding of text sent via this connection
	 * @return the text encoding
	 * @throws ConfigurationException
	 */
	public String getEncoding() throws ConfigurationException {
		return getXML().getChildTextTrim("encoding");
	}

	/**
	 * gets the locale of connection if exists from component.xml
	 * @return the localization
	 * @throws ConfigurationException
	 */
	public String getLocale() throws ConfigurationException {
		return getParameter("locale","");
	}

	private String printFetchModes() {
		StringBuffer output = new StringBuffer();
		for (FetchModes mode : FetchModes.values()) {
			output.append(mode.toString() +" ");
		}
		return output.toString();
	}
		
	/**
	 * gets the fetch mode of this connection.
	 * @return the fetch mode
	 * @throws ConfigurationException
	 */
	public FetchModes getFetchMode() throws ConfigurationException {
		String mode = getParameter("fetch","buffered");
		try { 
			return FetchModes.valueOf(mode.toUpperCase());
		}
		catch (Exception e) {
			throw new ConfigurationException("Attribute 'fetch' has to be set to either "+printFetchModes()+".");
		}	
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
	}
}
