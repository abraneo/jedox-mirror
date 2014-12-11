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


import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InterruptedIOException;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import org.apache.log4j.Logger;
import com.jedox.palojlib.managers.HttpHandlerInfo;
import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.managers.LoggerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.util.Helpers;

/**
 * Handler to send the palo requests, for each unique connection there is one handler. 
 * @author khaddadin
 *
 */
public class HttpHandlerLegacy{

    /** a cr and lf.*/
	protected static final byte[] CRLF = new byte[] {(byte) 13, (byte) 10};
	protected HttpHandlerInfo info = null;
	protected static final String LOGIN_REQUEST = "/server/login?";
	protected static final String LOGOUT_REQUEST = "/server/logout?";
	protected String SessionId;
	
	private Logger log = LoggerManager.getInstance().getLogger(HttpHandlerLegacy.class.getSimpleName());

	public HttpHandlerLegacy(HttpHandlerInfo info)  {
		this.info = info;
	
		//if the session configuration is used where the user is empty 
		//if(info.getEncodedUsername()==null||info.getEncodedUsername().isEmpty())
		//	this.secondTimeTry = true;
	}

	/**
	 * set the session id by making a palo server call
	 * @throws PaloException
	 */
	public synchronized void setSessionId() throws PaloException{
			
			if(this.info.getLoginRequest()!=null){
				String [][]response = send(this.info.getLoginRequest(),false,false);
				this.SessionId = response[0][0];
			}else{
		
				try{				
					StringBuilder LOGIN_REQUEST_BUFFER = new StringBuilder(LOGIN_REQUEST);
					StringBuilder currentRequest = LOGIN_REQUEST_BUFFER.append(HttpHandlerManager.getSessionInfoString(info.getClientInfo())).append("&user=").append(info.getEncodedUsername()).append("&password=").append(Helpers.encrpytPassword(info.getPassword()));
					this.info.setLoginRequest(currentRequest);
					String [][]response = send(currentRequest,false,false);
					this.SessionId = response[0][0];
				}catch(PaloException e){
					// for supervision server
					if(e.getCode().equals("1019")){
						StringBuilder LOGIN_REQUEST_BUFFER = new StringBuilder(LOGIN_REQUEST);
						StringBuilder currentRequest = LOGIN_REQUEST_BUFFER.append(HttpHandlerManager.getSessionInfoString(info.getClientInfo())).append("&user=").append(info.getEncodedUsername()).append("&password=").append(Helpers.urlEncode(info.getPassword()));
						String [][]response = send(currentRequest,false,false);
						this.SessionId = response[0][0];
					}else{
						throw e;
					}
				}
			}

		HttpHandlerManager.getInstance().addSession(info.getContextId(), this.SessionId);
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
	public synchronized void resetSessionId(boolean stop) throws PaloException{


		StringBuilder LOGOUT_REQUEST_BUFFER = new StringBuilder(LOGOUT_REQUEST);
		boolean withStop = (stop && (this.info.majorVersion>=6 || (this.info.majorVersion>=5 && this.info.minorVersion>=1)));
		StringBuilder currentRequest = LOGOUT_REQUEST_BUFFER.append("sid=").append(this.SessionId).append(withStop?"&type=1":"");
		if(withStop)
			this.info.setResendRequestIfError(false);
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
	protected String[] httpsend(StringBuilder requestbuffer,boolean withSession, boolean onlyToken)
			throws Exception {

		if(withSession){
			if((requestbuffer.charAt(requestbuffer.length()-1)) != '?')
				requestbuffer.append('&');

			requestbuffer.append("sid=").append(SessionId);
		}

		String request = "GET ".concat(requestbuffer.toString()).concat(" HTTP/1.1\r\n");
		log.debug("UseSSL:" + info.useSsl + ". Request:" + requestbuffer.toString());

		FixedLengthInputStream  in =null;
		BufferedOutputStream  out =null;

		Socket srvConnection = null;
		SSLSocket srvSslConnection = null;
		//for communication:
		BufferedOutputStream toServer;
		BufferedInputStream fromServer;

		try{
			// with SSL
			if(info.useSsl){
				SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
				srvSslConnection = (SSLSocket) factory.createSocket(info.getHost(),info.sslPort);
				srvSslConnection.setSoTimeout(info.getTimeout());
				srvSslConnection.setReuseAddress(true);
				srvSslConnection.setSoLinger(true, 0);

				int outSize = Math.min(srvSslConnection.getSendBufferSize(), 2048);
				int inSize = Math.min(srvSslConnection.getReceiveBufferSize(), 2048);
				toServer = new BufferedOutputStream(srvSslConnection.getOutputStream(),outSize);
				fromServer = new BufferedInputStream(srvSslConnection.getInputStream(),inSize);

			}else{
				srvConnection = new Socket(info.getHost(), Integer.parseInt(info.getPort()));
				srvConnection.setSoTimeout(info.getTimeout());
				srvConnection.setReuseAddress(true);
				srvConnection.setSoLinger(true, 0);

				int outSize = Math.min(srvConnection.getSendBufferSize(), 2048);
				int inSize = Math.min(srvConnection.getReceiveBufferSize(), 2048);
				toServer = new BufferedOutputStream(srvConnection.getOutputStream(),outSize);
				fromServer = new BufferedInputStream(srvConnection.getInputStream(),inSize);
			}

		}catch(Exception e){
			throw new RuntimeException("Could not connect to OLAP server at host '"+ info.getHost() + "' on port '"+  (info.useSsl?info.sslPort:info.getPort()) + "'  ==> " + e.getMessage());
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
				if(srvConnection!= null){srvConnection.close();}
				if(srvSslConnection != null){srvSslConnection.close();}
			}
		}
	}

	public String[][] send(StringBuilder request,boolean withSession,boolean onlyToken) throws PaloException{

		String[] result = {};
		StringBuilder requestBackup = new StringBuilder(request.toString());

		try {

			result = httpsend(request,withSession,onlyToken);
		}catch(Exception e){
			throw new RuntimeException(e.getMessage());
		}

		// if the result id empty or starts with word PALOERROR then throw an exception
		if(result.length != 0 && result[0].startsWith("PALOERROR")){
			PaloException ex = new PaloException(result[0]);
			if(this.info.isResendRequestIfError() &&  ex.getCode().equals("1015")){
				if(!requestBackup.toString().startsWith(HttpHandlerLegacy.LOGOUT_REQUEST)){
					log.debug("Session is invalid, trying to get a new one.");
					try {
						setSessionId();
						result = httpsend(requestBackup,withSession,onlyToken);
					} catch (Exception e) {
						log.error(e.getMessage(), e);
					}
				}else{
					log.debug("Session is already invalid.");
				}
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
	 * @return parsed string
	 */
	protected final String[][] parse(String[] response) {
		if(response == null)
			return new String[0][];

		String[][] res = new String[response.length][];
		for(int i=0;i<response.length;++i) {
			res[i] = new HttpParser().parseLine(response[i],';');
		}
		return res;
	}

}
