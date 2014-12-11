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
import java.net.Socket;
import java.util.ArrayList;
import javax.net.ssl.SSLSocket;
import org.apache.log4j.Logger;
import com.jedox.palojlib.managers.HttpHandlerInfo;
import com.jedox.palojlib.managers.LoggerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;

/**
 * Handler to send the palo requests, for each unique connection there is one handler. 
 * @author khaddadin
 *
 */
public class HttpHandler extends HttpHandlerLegacy{
	
	private Logger log = LoggerManager.getInstance().getLogger(HttpHandler.class.getSimpleName());

	public HttpHandler(HttpHandlerInfo info)  {
		super(info);
	}

	public synchronized void resetSessionId(boolean stop) throws PaloException{

		try {
			super.resetSessionId(stop);
		} catch (Exception e) {
			log.debug("While logging out the connection: " + e.getMessage());
		}finally{
			SocketManager.getInstance().removeSocketConnection(info,true);
		}
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
		
		//use the Legacy logic
		if(!SocketManager.getInstance().isActive()){
			return super.httpsend(requestbuffer, withSession, onlyToken);
		}

		if(withSession){
			if((requestbuffer.charAt(requestbuffer.length()-1)) != '?')
				requestbuffer.append('&');

			requestbuffer.append("sid=").append(SessionId);
		}

		String request = "GET ".concat(requestbuffer.toString()).concat(" HTTP/1.1\r\n");
		log.debug("Request:" + requestbuffer.substring(0, Math.min(requestbuffer.length(),1000)).toString());

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
				srvSslConnection = (SSLSocket) SocketManager.getInstance().getSocketConnection(info);

				int outSize = Math.min(srvSslConnection.getSendBufferSize(), 2048);
				int inSize = Math.min(srvSslConnection.getReceiveBufferSize(), 2048);
				toServer = new BufferedOutputStream(srvSslConnection.getOutputStream(),outSize);
				fromServer = new BufferedInputStream(srvSslConnection.getInputStream(),inSize);

			}else{
				srvConnection = SocketManager.getInstance().getSocketConnection(info);

				int outSize = Math.min(srvConnection.getSendBufferSize(), 2048);
				int inSize = Math.min(srvConnection.getReceiveBufferSize(), 2048);
				toServer = new BufferedOutputStream(srvConnection.getOutputStream(),outSize);
				fromServer = new BufferedInputStream(srvConnection.getInputStream(),inSize);
			}

		}catch(Exception e){
			throw new RuntimeException("Could not connect to OLAP server at host '"+ info.getHost() + "' on port '"+  (info.useSsl?info.sslPort:info.getPort()) + "': " + e.getMessage());
		}

		try {

			toServer.write(request.getBytes("UTF-8"));
			toServer.write(CRLF);
			toServer.flush();
			out = new BufferedOutputStream(toServer);

			// get response:
			HeaderParser headerHandler = new HeaderParser();
			headerHandler.parse(fromServer);
			
			if(headerHandler.getErrorCode()==-1){
				throw new PaloJException("OLAP server response is empty, please check your antivirus.");
			}
			
			in = new FixedLengthInputStream(fromServer,headerHandler.getContentLength());

			// read content
			ArrayList<String> respLines = new ArrayList<String>();
			for (;;) {
				String response = new HttpParser().readRawLine(in);
				if (response.trim().length() < 1) {
					break;
				}
				//uncomment if needed
				//log.debug("response: " + response);
				respLines.add(response);
			}

			if (headerHandler.getErrorCode() != 200) {
				String [] result =  respLines.toArray(new String[respLines.size()]);
				if (result.length > 0) {
					log.debug("Palo error occured while sending request: " + requestbuffer.toString());
					result[0] = "PALOERROR" + result[0];
				}
				return result;
			}

			if(onlyToken && headerHandler.getToken() != null ){
				return new String[]{headerHandler.getToken()};
			}

			return (String[]) respLines.toArray(new String[respLines.size()]);
		} catch (Exception se) {
			throw new Exception("OLAP Server not responding." + se.getMessage());
		} finally {
			{
				if(out != null) {out.flush();}
			}
		}
	}

	public final String[][] send(StringBuilder request,boolean withSession,boolean onlyToken) throws PaloException{

		//use the Legacy logic
		if(!SocketManager.getInstance().isActive()){
			return super.send(request, withSession, onlyToken);
		}
		
		String[] result = {};
		StringBuilder requestBackup = new StringBuilder(request.toString());

		try {
			result = httpsend(request,withSession,onlyToken);
		}catch(Exception e){
			try {
				log.debug(e.getMessage() + ".Try to resend the request.");
				SocketManager.getInstance().removeSocketConnection(info,false);
				if(this.info.isResendRequestIfError())
					result = httpsend(request,false,onlyToken);
			} catch (Exception e1) {
				throw new RuntimeException(e1.getMessage());
			}
		}

		// if the result id empty or starts with word PALOERROR then throw an exception
		if(result.length != 0 && result[0].startsWith("PALOERROR")){
			PaloException ex = new PaloException(result[0]);
			if(this.info.isResendRequestIfError() && ex.getCode().equals("1015")){
				if(!requestBackup.toString().startsWith(HttpHandlerLegacy.LOGOUT_REQUEST)){
					log.debug("Session is invalid, trying to get a new one and resend the request.");
					try {
						setSessionId();
						result = httpsend(requestBackup,withSession,onlyToken);
					} catch (Exception e) {
						//clean before throwing an exception
						SocketManager.getInstance().removeSocketConnection(info,true);
						throw ex;
					}
				}else{
					log.debug("Session is already invalid.");
				}
				if(result.length != 0 && result[0].startsWith("PALOERROR")){
					//clean before throwing an exception
					SocketManager.getInstance().removeSocketConnection(info,true);
					throw new PaloException(result[0]);
				}
			}else{
				//clean before throwing an exception
				SocketManager.getInstance().removeSocketConnection(info,true);
				throw ex;
			}		
		}
		return parse(result);

	}

}
