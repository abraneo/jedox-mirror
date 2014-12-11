package com.jedox.palojlib.http;


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


import java.io.IOException;
import java.io.InputStream;
import java.net.SocketException;

import org.apache.log4j.Logger;

import com.jedox.palojlib.managers.LoggerManager;

/**
 * parses the palo header and saves it's information
 * @author khaddadin
 *
 */
public class HeaderParser {

	private int contentLength;
	private int errorCode = -1;
	private String errorMessage;
	private String token;
	
	@SuppressWarnings("unused")
	private Logger log = LoggerManager.getInstance().getLogger(HeaderParser.class.getSimpleName());

	/**
	 * parses the header response
	 * @param in
	 * @throws NumberFormatException
	 * @throws IOException
	 * @throws SocketException
	 */
	public final void parse(InputStream in) throws NumberFormatException, IOException, SocketException {

		String response = null;
		String token = null;

       for(;;) {
			response = new HttpParser().readLine(in);
			if (response.trim().length() < 1) {
				break;
			}
            //uncomment if needed
			//log.debug("header:" + response);

			if (response.startsWith("HTTP/1.1")) {
				String code = response.substring(9).trim();
				int index = code.indexOf(" ");
				setErrorCode(Integer.parseInt(code.substring(0, index)));
				setErrorMessage(code.substring(index).trim());
			}

			if (response.startsWith("X-PALO-SV") || response.startsWith("X-PALO-DB") || response.startsWith("X-PALO-DIM") || response.startsWith("X-PALO-CC")) {
				token = response.substring(response.indexOf(':')+2);
			}

			if(response.startsWith("Content-Length:"))
				this.contentLength = Integer.parseInt(response.substring(16));

       }
       this.token =  token;
	}

	/**
	 * set the error code from palo response (only if error exists)
	 * @param errorCode
	 */
	private final void setErrorCode(int errorCode) {
		this.errorCode = errorCode;
	}

	/**
	 * set the error message from palo response (only if error exists)
	 * @param errorMessage
	 */
	private final void setErrorMessage(String errorMessage) {
		this.errorMessage = errorMessage;
	}

	/**
	 * get the content length of the response
	 * @return content length of palo resonse
	 */
	public final  int getContentLength() {
		return contentLength;
	}

	/**
	 * get the token of component that is used in the palo call
	 * @return Palo OLAP server token
	 */
	public final String getToken() {
		return this.token;
	}

	/**
	 * set the error code from palo response (only if error exists)
	 * @return errorCode
	 */
	public final int getErrorCode() {
		return errorCode;
	}

	/**
	 * get the error code from palo response (only if error exists)
	 * @return errorcode
	 */
	public final String getErrorMessage() {
		return errorMessage;
	}

}