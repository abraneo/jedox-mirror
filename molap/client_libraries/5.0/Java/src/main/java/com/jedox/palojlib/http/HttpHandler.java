package com.jedox.palojlib.http;
/**
 *
 */

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


import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InterruptedIOException;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import org.apache.log4j.Logger;

import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.managers.LoggerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.main.ClientInfo;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.util.Helpers;

/**
 * Handler to send the palo requests, for each unique connection there is one handler. 
 * @author khaddadin
 *
 */
public class HttpHandler{

    /** a cr and lf.*/
	private static final byte[] CRLF = new byte[] {(byte) 13, (byte) 10};

    private final String host;
    private int port;
    private final int sslport;
    private final int olapMajorVersion;
    private final int olapMinorVersion;
    protected final String username;
    protected final String password;
    protected String client;
    protected String clientDescription;
	private boolean useSslPort; //this value decides which socket connection to use
	private final int timout;
	protected static final String LOGIN_REQUEST = "/server/login?";
	private static final String LOGOUT_REQUEST = "/server/logout?";
	protected String SessionId;
	protected String contextId = "";
	private boolean secondTimeTry = false; 
	protected ConnectionConfiguration config = null;

	private Logger log = LoggerManager.getInstance().getLogger(HttpHandler.class.getSimpleName());

	public HttpHandler(ConnectionConfiguration config,boolean useSslPort, int sslPort,int majorVersion, int minorVersion)  {
		this.host = config.getHost();
		if(!useSslPort){
			this.port = Integer.parseInt(config.getPort());
		}
		else{
			this.port = sslPort;
		}
		this.sslport = sslPort;
		this.username = Helpers.urlEncode(config.getUsername());
		this.password = config.getPassword();
		this.useSslPort = useSslPort;
		this.timout = config.getTimeout();
		this.contextId = config.getContextId();
		this.olapMajorVersion = majorVersion;
		this.olapMinorVersion = minorVersion;
		ClientInfo info = config.getClientInfo();
		if(info!=null){
			this.client = config.getClientInfo().getClient();
			this.clientDescription = config.getClientInfo().getClientDesc();
		}
		this.config = config;
	}
	
	private boolean loginOnlySsl(){
		/*if(this.olapMajorVersion>=6 || (this.olapMajorVersion==5 && this.olapMinorVersion>0)){
			return true;
		}*/
		return false;
	}

	/**
	 * set the session id by making a palo server call
	 * @throws PaloException
	 */
	public synchronized void setSessionId() throws PaloException{

		boolean backupUseSslPort = this.useSslPort;
		int backupPort = this.port;
		if(loginOnlySsl()){
			this.useSslPort=true;
			this.port=sslport;
		}
		
		if(this.config.getLoginRequest()!=null){
			String [][]response = send(this.config.getLoginRequest(),false,false);
			this.SessionId = response[0][0];
		}else{
	
			try{				
				StringBuilder LOGIN_REQUEST_BUFFER = new StringBuilder(LOGIN_REQUEST);
				StringBuilder currentRequest = LOGIN_REQUEST_BUFFER.append(HttpHandlerManager.getSessionInfoString(this.client,this.clientDescription)).append("&user=").append(username).append("&password=").append(Helpers.encrpytPassword(password));
				this.config.setLoginRequest(currentRequest);
				HttpHandlerManager.getInstance().httpHandlersInfos.get(this.contextId).setLoginRequest(currentRequest);
				String [][]response = send(currentRequest,false,false);
				this.SessionId = response[0][0];
			}catch(PaloException e){
				// for supervision server
				if(e.getCode().equals("1019")){
					StringBuilder LOGIN_REQUEST_BUFFER = new StringBuilder(LOGIN_REQUEST);
					StringBuilder currentRequest = LOGIN_REQUEST_BUFFER.append(HttpHandlerManager.getSessionInfoString(this.client,this.clientDescription)).append("&user=").append(username).append("&password=").append(Helpers.urlEncode(password));
					this.config.setLoginRequest(currentRequest);
					HttpHandlerManager.getInstance().httpHandlersInfos.get(this.contextId).setLoginRequest(currentRequest);
					String [][]response = send(currentRequest,false,false);
					this.SessionId = response[0][0];
				}else{
					throw e;
				}
			}
		}
		
		//set it back in case this http handler is used again (it will not be until now)
		if(loginOnlySsl()){
			this.useSslPort=backupUseSslPort;
			this.port=backupPort;
		}

		HttpHandlerManager.getInstance().addSession(contextId, this.SessionId);
	}

	/**
	 * set the session id by a given session
	 * @param sessionId
	 * @throws PaloException
	 */
	public final synchronized void setSessionId(String sessionId) throws PaloException{
		this.SessionId = sessionId;
	}

	/**
	 * expire a session id
	 * @throws PaloException
	 */
	public final synchronized void resetSessionId() throws PaloException{


		StringBuilder LOGOUT_REQUEST_BUFFER = new StringBuilder(LOGOUT_REQUEST);
		StringBuilder currentRequest = LOGOUT_REQUEST_BUFFER.append("sid=").append(this.SessionId);
		send(currentRequest,false,false);
		SessionId = null;
	}

	/**
	 * send the http request to Palo OLAP server
	 * @param requestbuffer the request
	 * @param withSession boolean value to indicate if a session should be added to request
	 * @param onlyToken indicates if only the token is needed from the response
	 * @return response as a array of string where each string indicate one line
	 * @throws Exception
	 */
	protected final String[] httpsend(StringBuilder requestbuffer,boolean withSession, boolean onlyToken)
			throws Exception {

		if(withSession){
			if((requestbuffer.charAt(requestbuffer.length()-1)) != '?')
				requestbuffer.append('&');

			requestbuffer.append("sid=").append(SessionId);
		}

		String request = "GET ".concat(requestbuffer.toString()).concat(" HTTP/1.1\r\n");
		log.debug("UseSSL:" + useSslPort + ". Request:" + requestbuffer.toString());

		FixedLengthInputStream  in =null;
		BufferedOutputStream  out =null;

		Socket srvConnection = null;
		SSLSocket srvSslConnection = null;
		//for communication:
		BufferedOutputStream toServer;
		BufferedInputStream fromServer;

		try{
			// with SSL
			if(this.useSslPort){
				SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
				srvSslConnection = (SSLSocket) factory.createSocket(host,port);
				srvSslConnection.setSoTimeout(this.timout);
				srvSslConnection.setReuseAddress(true);
				srvSslConnection.setSoLinger(true, 0);

				int outSize = Math.min(srvSslConnection.getSendBufferSize(), 2048);
				int inSize = Math.min(srvSslConnection.getReceiveBufferSize(), 2048);
				toServer = new BufferedOutputStream(srvSslConnection.getOutputStream(),outSize);
				fromServer = new BufferedInputStream(srvSslConnection.getInputStream(),inSize);

			}else{
				srvConnection = new Socket(host, port);
				srvConnection.setSoTimeout(this.timout);
				srvConnection.setReuseAddress(true);
				srvConnection.setSoLinger(true, 0);

				int outSize = Math.min(srvConnection.getSendBufferSize(), 2048);
				int inSize = Math.min(srvConnection.getReceiveBufferSize(), 2048);
				toServer = new BufferedOutputStream(srvConnection.getOutputStream(),outSize);
				fromServer = new BufferedInputStream(srvConnection.getInputStream(),inSize);
			}

		}catch(Exception e){
			throw new RuntimeException("Could not connect to OLAP server at host '"+ host + "' on port '"+  port + "'  ==> " + e.getMessage());
		}

		try {

			toServer.write(request.getBytes("UTF-8"));
			toServer.write(CRLF);
			toServer.flush();
			out = new BufferedOutputStream(toServer);

			// get response:
			HeaderParser headerHandler = new HeaderParser();
			headerHandler.parse(fromServer);
			in = new FixedLengthInputStream(fromServer,headerHandler.getContentLength());

			// read content
			ArrayList<String> respLines = new ArrayList<String>();
			for (;;) {
				String response = new HttpParser().readRawLine(in);
				if (response.trim().length() < 1) {
					break;
				}
				respLines.add(response);
			}

			if (headerHandler.getErrorCode() != 200) {
				String [] result =  respLines.toArray(new String[respLines.size()]);
				if (result.length > 0) {
					result[0] = "PALOERROR" + result[0];
				}
				return result;
			}

			if(onlyToken && headerHandler.getToken() != null ){
				return new String[]{headerHandler.getToken()};
			}

			return (String[]) respLines.toArray(new String[respLines.size()]);
		} catch (SocketException se) {
			throw new Exception("OLAP Server not responding." + se.getMessage());
		} catch (InterruptedIOException ie) {
			//timeout exception:
			throw new Exception("OLAP Server not responding.", ie);
		} finally {
			{
				//if(toServer != null) {toServer.flush();toServer.close();}
				//if(fromServer!= null){fromServer.close();}
				if(in != null) {in.close();in=null;}
				if(out != null) {out.close();out=null;}
				//if(srvConnection!= null){srvConnection.close();}
				//if(srvSslConnection != null){srvSslConnection.close();}
			}
		}
	}

	public final String[][] send(StringBuilder request,boolean withSession,boolean onlyToken) throws PaloException{

		String[] result = {};
		StringBuilder requestBackup = new StringBuilder(request.toString());

		try {

			if(!withSession){
				result = httpsend(request,withSession,onlyToken);
			}else{
				result = httpsend(request,withSession,onlyToken);
			}
		}catch(Exception e){
			throw new RuntimeException(e.getMessage());
		}

		// if the result id empty or starts with word PALOERROR then throw an exception
		if(result.length != 0 && result[0].startsWith("PALOERROR")){
			PaloException ex = new PaloException(result[0]);
			if(ex.getCode().equals("1015") && this.secondTimeTry==false){
				this.secondTimeTry=true;
				log.debug("Session is invalid, trying to get a new one.");
				setSessionId();
				try {
					result = httpsend(requestBackup,withSession,onlyToken);
				} catch (Exception e) {}
				if(result.length != 0 && result[0].startsWith("PALOERROR"))
					throw new PaloException(result[0]);
			}else{
				throw ex;
			}		
		}
		return parse(result);

	}

	/**
	 * parse the one dimensional array of response lines to 2-dimensional array, where semicolon is the delimiter of the lines.
	 * @param response Palo OLAP server response
	 * @return
	 */
	private final String[][] parse(String[] response) {
		if(response == null)
			return new String[0][];

		String[][] res = new String[response.length][];
		for(int i=0;i<response.length;++i) {
			res[i] = new HttpParser().parseLine(response[i],';');
		}
		return res;
	}

}
