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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.logging;

import java.util.HashSet;
import org.apache.commons.logging.Log;

/**
 * Wrapper class for the logging of messages. Identical messages occuring multiple times are written only once to the underlying log.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class MessageHandler implements Log {
	
	private HashSet<Object> messages = new HashSet<Object>();
	private Log log;
	
	
	public MessageHandler(Log log) {
		this.log = log;
	}
	
	private boolean isLogged(Object message) {
		if (messages.contains(message))
			return false;
		else {
			messages.add(message);
			return true;
		}
	}
	
	public boolean willBeLogged(Object message) {
		if (messages.contains(message))
			return false;
		else {
			return true;
		}
	}


	public void debug(Object message) {
		if (isLogged(message))
			log.debug(message);
	}

	public void error(Object message) {
		if (isLogged(message))
			log.error(message);
	}

	public void fatal(Object message) {
		if (isLogged(message))
			log.fatal(message);
	}

	public void info(Object message) {
		if (isLogged(message))
			log.info(message);
	}

	public void warn(Object message) {
		if (isLogged(message))
			log.warn(message);
	}

	public void debug(Object message, Throwable throwable) {
		if (isLogged(message))
			log.debug(message,throwable);
	}
	
	public void error(Object message, Throwable throwable) {
		if (isLogged(message))
			log.error(message,throwable);
	}

	public void fatal(Object message, Throwable throwable) {
		if (isLogged(message))
			log.fatal(message, throwable);
	}

	public void info(Object message, Throwable throwable) {
		if (isLogged(message))
			log.info(message, throwable);	
	}

	public void warn(Object message, Throwable throwable) {
		if (isLogged(message))
			log.warn(message, throwable);
	}
	
	public boolean isDebugEnabled() {
		return log.isDebugEnabled();
	}

	public boolean isErrorEnabled() {
		return log.isErrorEnabled();
	}

	public boolean isFatalEnabled() {
		return log.isFatalEnabled();
	}

	public boolean isInfoEnabled() {
		return log.isInfoEnabled();
	}

	public boolean isTraceEnabled() {
		return log.isTraceEnabled();
	}

	public boolean isWarnEnabled() {
		return log.isWarnEnabled();
	}

	public void trace(Object message) {
		log.trace(message);
	}

	public void trace(Object message, Throwable throwable) {
		log.trace(message, throwable);
		
	}

}
