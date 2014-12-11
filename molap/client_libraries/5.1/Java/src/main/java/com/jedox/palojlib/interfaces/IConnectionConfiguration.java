/**
 * 
 */
package com.jedox.palojlib.interfaces;

import com.jedox.palojlib.interfaces.IConnectionInfo.EncryptionType;
import com.jedox.palojlib.main.ClientInfo;
/**
 * represent a connection configuration to a server
 * @author khaddadin
 *
 */
public interface IConnectionConfiguration {

	/**
	 * get the olap server host
	 * @return host
	 */
	public String getHost();

	/**
	 * get the olap server port
	 * @return port
	 */
	public String getPort();

	/**
	 * get the username
	 * @return username
	 */
	public String getUsername();

	/**
	 * get the username's password
	 * @return password
	 */
	public String getPassword();

	/**
	 * get the amount of time to wait for response before firing a timeout exception.
	 * @return timeout
	 */
	public int getTimeout();

	/**
	 * get whether ssl connection is preferred or not.
	 * It only has an effect if olap server ssl mode is optional {@link EncryptionType#ENCRYPTION_OPTIONAL}
	 * @return sslPreferred
	 */
	public boolean isSslPreferred();

	/**
	 * set the host of the olap server.
	 * @param host
	 */
	public void setHost(String host);

	/**
	 * set the port of the olap connection.
	 * @param port
	 */
	public void setPort(String port);

	/**
	 * set the user name in olap server.
	 * @param username
	 */
	public void setUsername(String username);

	/**
	 * set the password of the user in olap server.
	 * @param password
	 */
	public void setPassword(String password);

	/**
	 * set the amount of time in milliseconds to wait for response before firing a timeout exception.
	 * @param timeout
	 */
	public void setTimeout(int timeout);

	/**
	 * set whether ssl connection is preferred or not.
	 * It only has an effect if olap server ssl mode is optional {@link EncryptionType#ENCRYPTION_OPTIONAL}
	 * @return sslPreferred
	 */
	public void setSslPreferred(boolean sslPreferred);
	
	/**
	 * get the client information of this connection {@link ClientInfo}
	 * @return client info
	 */
	public ClientInfo getClientInfo();
	
	/**
	 * set the client information of this connection {@link ClientInfo}
	 * @param info client info
	 */
	public void setClientInfo(ClientInfo info);
}
