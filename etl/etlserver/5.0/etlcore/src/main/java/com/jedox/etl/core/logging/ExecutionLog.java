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
import java.util.Vector;

import org.apache.log4j.PatternLayout;
import org.apache.log4j.WriterAppender;
import org.apache.log4j.Level;
import org.apache.log4j.spi.LoggingEvent;
import com.jedox.etl.core.execution.ExecutionState;
/**
 * Logging Appender class, which consumes all logging information from components involved in an {@link com.jedox.etl.core.execution.Execution} and updates the shared {@link com.jedox.etl.core.execution.ExecutionState} accordingly. This enables components to use ordinary logging mechanisms and thus eliminates the need for the components to take care of the ExecutionState.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ExecutionLog extends WriterAppender {
	
	private ExecutionState result;
	private int logLimit = 100;
	private int linesOmitted = 0;
	private Level rootLevel;
	private PatternLayout layout = new PatternLayout("%d %5p : %m%n");
	private Vector<String> threadNames = new Vector<String>();
	private boolean failOnError = false;
	private boolean isClosed = false;
	private boolean isActive = true;

	public ExecutionLog(ExecutionState result) {
//		previous layout: "%d %5p [%t] (%F:%L) - %m%n"
		super(new PatternLayout("%d %5p : %m%n"), new java.io.StringWriter());
		this.result = result;
		LogManager.getInstance().addAppender(this);
		rootLevel = LogManager.getInstance().getLevel();
		if (rootLevel.isGreaterOrEqual(Level.WARN)) {
			LogManager.getInstance().setLevel(Level.WARN);
		}
		this.setThreshold(Level.INFO);
	}
	
	/**
	 * sets the name of the thread (which corresponds to the name of the {@link com.jedox.etl.core.execution.Execution}) to accept the logging information from. This prevents logging messages originating from different executing Threads to get mingled.
	 * @param threadName the name of the thread to accept logging messages from
	 */
	public void addTargetThreadName(String threadName) {
		threadNames.add(threadName);
	}
	
	public void removeTargetThreadName(String threadName) {
		threadNames.remove(threadName);
	}
	
	public void setLogLimit(int logLimit) {
		this.logLimit = logLimit;
	}

	public void setFailOnError(boolean failOnError) {
		this.failOnError = failOnError;
	}
	
	private void addInfo() {
		result.addInfos(1);
	}
	
	private void addError() {
		result.addErrors(1);
	}
	
	private void addWarning() {
		result.addWarnings(1);
	}
	
	private boolean failOnError() {
		return failOnError;
	}
	
	private void setFailed() {
		result.setExecutable(false);
	}

	/** @see org.apache.log4j.WriterAppender#subAppend(LoggingEvent) */
	protected void subAppend( LoggingEvent event ) {
		//test if sender acts in the same executive thread 
		if (isActive() && threadNames.contains(event.getThreadName())) {
			// Write all Info messages, write up to logLimit Warning and Error Messages	
			result.addMessage(event.getLevel().toString(),event.getTimeStamp(),layout.format(event));
			if (event.getLevel().equals(Level.INFO)) {
		    	addInfo();
		    	if (result.getInfos() > logLimit*100) {
		    		result.removeMessage(result.getFirstMessageOfType(Level.INFO.toString()));
		    		linesOmitted ++;
		    	}
		    }
		    if (event.getLevel().equals(Level.WARN)) {
		    	addWarning();
		    	if (result.getWarnings() > logLimit) {
		    		result.removeMessage(result.getFirstMessageOfType(Level.WARN.toString()));
		    		linesOmitted ++;
		    	}
		    }
		    if (event.getLevel().equals(Level.ERROR)) {
		    	addError();
		    	if (result.getErrors() > logLimit) {
		    		result.removeMessage(result.getFirstMessageOfType(Level.ERROR.toString()));
		    		linesOmitted ++;
		    	}
		    	if (failOnError()) setFailed(); 
		    }
		    if (event.getLevel().equals(Level.FATAL)) {
		    	addError();
		    	setFailed();
		    }
		}
	}

	/**
	 * get Number of omitted Warning Messages
	 */
	public int getLinesOmitted() {
		return linesOmitted;
	}	
	
	/**
	 * get the Log Limit for Warning Messages
	 */
	public int getLogLimit() {
		return logLimit;
	}	
		
	/**
	 * closes this Appender and removes it from the {@link LogManager}
	 */
	public void close() {
		if (!isClosed) {
			isClosed = true;
			setActive(false);
			LogManager.getInstance().removeAppender(this);
		}
		//super.close();
	}

	public void setActive(boolean isActive) {
		this.isActive = isActive;
	}

	public boolean isActive() {
		return isActive;
	}

}
