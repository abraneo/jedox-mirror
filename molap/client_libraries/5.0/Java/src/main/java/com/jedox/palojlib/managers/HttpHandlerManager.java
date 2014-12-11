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

package com.jedox.palojlib.managers;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Set;

import org.apache.log4j.Logger;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.http.HttpHandler;
import com.jedox.palojlib.http.HttpHandlerInfo;
import com.jedox.palojlib.main.ClientInfo;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.main.ConnectionHandler;
import com.jedox.palojlib.main.ConnectionInfo;
import com.jedox.palojlib.main.ConnectionInfo.EncryptionType;
import com.jedox.palojlib.util.Helpers;



public class HttpHandlerManager {
	
	// store for which connection the httphandlerInfo was initialized 
	public HashMap<String,HttpHandlerInfo> httpHandlersInfos = new HashMap<String, HttpHandlerInfo>();
	//keep track of the last session created for a certain connection
	//a connection is here defined through it's connection configuration object+random number
	private HashMap<String,String> sessions = new HashMap<String, String>();
	
	private static Logger log = LoggerManager.getInstance().getLogger(HttpHandler.class.getSimpleName());
	private static HttpHandlerManager manager;
	private static LinkedHashMap<String, String> sessionInfos = new LinkedHashMap<String, String>();
	public static final String version = "4.0.$WCREV$";
	public static final String defaultClient = "Palojlib";
	public static final String defaultClientDesc = "Jedox OLAP java client library";
	
	private HttpHandlerManager(){}
	
	public static HttpHandlerManager getInstance(){
		if(manager!=null){
			return manager;
		}
		manager = new HttpHandlerManager();
		setSessionInfos();
		return manager;
	}
	
	private static void setSessionInfos() {
		HttpHandlerManager.sessionInfos.put("client", "%s");
		HttpHandlerManager.sessionInfos.put("lib", "palojlib");
		HttpHandlerManager.sessionInfos.put("lib_ver", version);
		HttpHandlerManager.sessionInfos.put("java", System.getProperty("java.runtime.version"));
		HttpHandlerManager.sessionInfos.put("platform", System.getProperty("os.name") + " " + System.getProperty("os.arch") );
		HttpHandlerManager.sessionInfos.put("desc", "%s");
	}
	
	public static String getSessionInfoString(String client,String desc){
		
		String newName = "{";
		Set<String> keys = HttpHandlerManager.sessionInfos.keySet();
		for(String key:keys){
			newName = newName + "\"" + key + "\":\"" + HttpHandlerManager.sessionInfos.get(key)+ "\",";
		}
		
		newName = String.format(newName, (client!=null?client:defaultClient),(desc!=null?desc:defaultClientDesc));
		
		newName = Helpers.urlEncode(newName.substring(0,newName.length()-1) + "}");
		return "new_name=" + newName;
	}

	/**
	 * initialize the httphandler that will be used for a certain ConnectionConfiguartion
	 * @param config connection configuration
	 * @return httphandler for this connection configuration
	 * @throws PaloException
	 * @throws PaloJException
	 */
	public HttpHandler initHttpHandler(ConnectionConfiguration config) throws PaloException, PaloJException {

		HttpHandler handler = null;
		String contextId = config.getContextId();
				

		handler = new HttpHandler(config,false,0,0,0);
		HttpHandlerInfo httpHandlerInfo = new HttpHandlerInfo(config.getHost(),config.getPort(),config.getUsername(),config.getPassword(),config.isSslPreferred(),config.getTimeout(),config.getContextId(),false, "0","0", "0",config.getClientInfo());
		httpHandlersInfos.put(contextId, httpHandlerInfo);
		
		ConnectionHandler connectionHandler = new ConnectionHandler(config);
		ConnectionInfo info = connectionHandler.getServerInfo();
		httpHandlerInfo.majorVersion = Integer.parseInt(info.getMajorVersion());
		httpHandlerInfo.minorVersion = Integer.parseInt(info.getMinorVersion());
		httpHandlerInfo.sslPort = info.getHttpsPort();

		if(info.getEncryptionType().equals(EncryptionType.ENCRYPTION_REQUIRED) || (info.getEncryptionType().equals(EncryptionType.ENCRYPTION_OPTIONAL) && config.isSslPreferred())){

			handler = new HttpHandler(config,true,info.getHttpsPort(),Integer.parseInt(info.getMajorVersion()),Integer.parseInt(info.getMinorVersion()));
			httpHandlerInfo.useSsl = true;
		}

		log.debug("New Httphandler pattern is configured with SSL=" + httpHandlerInfo.useSsl + " and port=" +  httpHandlerInfo.sslPort);

		return handler;

	}
	
	/**
	 * get existing httphandler for a certain context and attach the last current session to it
	 * @param contextId
	 * @return htpphandler
	 * @throws PaloException
	 * @throws PaloJException
	 */
	public HttpHandler getHttpHandler(String contextId) throws PaloException, PaloJException {

		HttpHandler handler = null;
		HttpHandlerInfo httpHandlerInfo = httpHandlersInfos.get(contextId);
		
		if(httpHandlerInfo==null){
			throw new PaloJException("Connection is not opened.");
		}

		handler = new HttpHandler(httpHandlerInfo, httpHandlerInfo.useSsl, httpHandlerInfo.sslPort,httpHandlerInfo.majorVersion,httpHandlerInfo.minorVersion);
		String sessionId = sessions.get(contextId);
		if(sessionId != null)
			handler.setSessionId(sessions.get(contextId));
		else{
			handler.setSessionId();
		}
		return handler;

	}
	
	public void removeHttpHandler(ConnectionConfiguration config) {

		String contextId = config.getContextId();
		httpHandlersInfos.remove(contextId);
		sessions.remove(contextId);
	}

	public void addSession(String contextId,String sessionId) {
		sessions.put(contextId,sessionId);
	}

	public boolean isInitialized(ConnectionConfiguration connectionConfiguration) {
		if(httpHandlersInfos.get(connectionConfiguration.getContextId()) != null)
			return true;
		return false;
	}
}
