package com.jedox.palojlib.http;

import com.jedox.palojlib.main.ClientInfo;
import com.jedox.palojlib.main.ConnectionConfiguration;

/**
 * extends the ConnectionConfiguration to manage ssl communication and contextId info
 * @author khaddadin
 *
 */
public final class HttpHandlerInfo extends ConnectionConfiguration{
	
	public boolean useSsl;
	public int sslPort;
	public int majorVersion; 
	public int minorVersion; 

	public HttpHandlerInfo(String host,String port, String username, String password, boolean sslPreferred, int timeout, String contextId, boolean useSsl, String sslPort,String olapMajorVersion,String olapMinorVersion,ClientInfo info){
		super.setHost(host);
		super.setPort(port);
		super.setUsername(username);
		super.setPassword(password);
		super.setSslPreferred(sslPreferred);
		super.setTimeout(timeout);
		super.contextId = contextId;
		super.setClientInfo(info);
		this.useSsl = useSsl;
		this.sslPort = Integer.parseInt(sslPort);
		this.majorVersion = Integer.parseInt(olapMajorVersion);
		this.minorVersion = Integer.parseInt(olapMinorVersion);
	}
}
