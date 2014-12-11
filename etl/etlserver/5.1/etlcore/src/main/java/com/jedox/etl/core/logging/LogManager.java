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
package com.jedox.etl.core.logging;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.Appender;
import org.apache.log4j.RollingFileAppender;
import org.apache.log4j.PatternLayout;

import java.io.File;
import java.io.IOException;
import com.jedox.etl.core.config.Settings;
/**
 * Manager class for central log management. Creates a {@link RollingFileAppender} log, which logs to the log directory (defined in config.xml) of the ETL-Server. Uses the log4j backend.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class LogManager {
	
	private static LogManager instance;
	private static final String rollingFileAppenderName = "etlRollingFileAppender";
	
	/**
	 * gets the singleton instance of this LogManager
	 * @return the LogManager
	 */
	public static LogManager getInstance() {
		if (instance == null) {
			instance = new LogManager();
			String filename = Settings.getInstance().getLogDir() + File.separator + "etlserver.log";
			try {
				RollingFileAppender appender = new RollingFileAppender(new PatternLayout("%d %p %t %c - %m%n"),filename,true);
				appender.setName(rollingFileAppenderName);
				appender.setMaxFileSize("10MB");
				appender.setMaxBackupIndex(20);				
				Logger.getRootLogger().addAppender(appender);
			}
			catch (IOException e) {
				Logger.getRootLogger().warn("Cannot create logfile "+filename+": "+e.getMessage());	
			}
			//Logger.getRootLogger().removeAppender("etlconsole");
		}
		return instance;
	}
	
	/**
	 * sets the level of the root logger (ignoring all messages with lower level)
	 * @param level the level as String
	 */
	public void setLevel(String level) {
		if (level != null)
			Logger.getRootLogger().setLevel(Level.toLevel(level.toUpperCase()));
	}
	
	/**
	 * sets the level of the root logger (ignoring all messages with lower level)
	 * @param level the level
	 */
	public void setLevel(Level level) {
		if (level != null)
			Logger.getRootLogger().setLevel(level);
	}
	
	/**
	 * gets the level of the root logger
	 * @return the root logger's level
	 */
	public Level getLevel() {
		return Logger.getRootLogger().getLevel();
	}
	
	/**
	 * adds an appender / a log to the root logger
	 * @param appender the appender to add
	 */
	public void addAppender(Appender appender) {
		Logger.getRootLogger().addAppender(appender);
	}
	
	/**
	 * removes an appender / a log from the root logger
	 * @param appender the appender to remove
	 */
	public void removeAppender(Appender appender) {
		Logger.getRootLogger().removeAppender(appender);
	}
	
	/**
	 * shuts down the logging backend.
	 */
	public void shutdown() {
		org.apache.log4j.LogManager.shutdown();
	}

	public void setLogFileSize(String logFileSize) {
		if (logFileSize!=null) 
			((RollingFileAppender)Logger.getRootLogger().getAppender(rollingFileAppenderName)).setMaxFileSize(logFileSize);
	}

	public void setLogBackupIndex(String logBackupIndex) {
		if (logBackupIndex!=null)
			((RollingFileAppender)Logger.getRootLogger().getAppender(rollingFileAppenderName)).setMaxBackupIndex(Integer.parseInt(logBackupIndex));
	}
	
	public void setDebugClasses(String logDebugClasses){
		if(logDebugClasses!=null && !logDebugClasses.isEmpty() && Logger.getRootLogger().getLevel().equals(Level.DEBUG)){
			setLevel(Level.INFO);
			String[] classesOrPackages = logDebugClasses.split(";");
			for(String s:classesOrPackages){
				Logger.getLogger(s).setLevel(Level.DEBUG);
			}
		}
	}

}
