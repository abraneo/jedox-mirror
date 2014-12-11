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
package com.jedox.etl.service;

import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Properties;
import javax.net.ssl.HttpsURLConnection;
import javax.servlet.ServletContext;
import org.apache.axis2.context.ConfigurationContext;
import org.apache.axis2.description.AxisService;
import org.apache.axis2.engine.ServiceLifeCycle;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.InternalConnectionManager;
import com.jedox.etl.core.logging.LogManager;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.NoHostnameVerifier;

public class ETLServiceLifeCycle implements ServiceLifeCycle {

	private static Log log = LogFactory.getLog(ETLServiceLifeCycle.class);
	private static ServletContext servletCtx;
	private static String rootDir;

	public void shutDown(ConfigurationContext configCtx, AxisService service) {
		try {
			// ConfigManager.getInstance().save();
			InternalConnectionManager.getInstance().shutDown();
		}
		catch (Exception e) {
			log.warn("Problem occured while shutting down: "+e.getMessage());
		}
		finally {
			log.info("ShutDown ETL-Serivce");
		}
	}
	
	private String getContextInitParameter(String param){
		// Try to find global directory
		// It has to contain repository and persistence
		String dir = servletCtx.getInitParameter(param);
		if (dir != null && FileUtil.isRelativ(dir))
			dir = rootDir + File.separator + dir;
		
		return dir;
	}

	public void startUp(ConfigurationContext configCtx, AxisService service) {
		
		//servlet context
		servletCtx = (ServletContext)configCtx.getProperty("transport.http.servletContext");
		// Try to find root directory
		// It has to contain config.xml under .\config
		// NOTE: don't call Config.getInstance() before setConfigDir... I know this should be done at a better place but here is good enough for the server
		rootDir = servletCtx.getRealPath("/");
		if (rootDir != null) {
			try {
				Settings.setRootDir(rootDir);
			} catch (IOException e) {
				// log.error("Can't get canonical path to root directory" + rootDir);
			}
		} // else: rely on Config that it finds some reasonable default

		String logDir = getContextInitParameter("etlserver.logdir");
		if (logDir != null) {
			try {
				Settings.setGlobalLogDir(logDir);
			} catch (IOException e) {
				// log.error("Can't get canonical path to global log directory" + logDir);
			}
		} // else: log directory from config.xml used
		
		String dataDir = getContextInitParameter("etlserver.datadir");		
		if (dataDir != null) {
			try {
				Settings.setGlobalDataDir(dataDir);
			} catch (IOException e) {
				// log.error("Can't get canonical path to global data directory" + dataDir);
			}
		} // else: data directory from config.xml used
	
		// Initialise LogManager and override log level with value stored in config.xml
		LogManager.getInstance().setLevel(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("loglevel"));	
		LogManager.getInstance().setLogFileSize(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("logFileSize"));
		LogManager.getInstance().setLogBackupIndex(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("logBackupIndex"));
		LogManager.getInstance().setDebugClasses(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("logDebugClasses"));
				
		log.info("StartUp ETL-Service");
		log.info("Jedox ETL Server Version: "+Settings.getInstance().getVersion());
		log.info("Setting application root directory : " + rootDir);
		log.info("Setting application log directory : " + logDir);
		log.info("Setting application data directory : " + dataDir);
		log.info("Java home: " + System.getProperty("java.home"));
		log.info("Java version: " + System.getProperty("java.version"));
		log.info("Java vendor: " + System.getProperty("java.vendor"));
		setSSLProperties();
		outputSettings(configCtx);
	}
	
	private void setSSLProperties() {
		HttpsURLConnection.setDefaultHostnameVerifier(new NoHostnameVerifier());
		// sets the keystore path
		try{
			 Properties props = new Properties();
			 File propFile = new File(rootDir + "config/ssl.properties");

				try {
					props.load(propFile.toURI().toURL().openStream());
				} catch (MalformedURLException e) {
					log.debug("Try to open SSL connecion " + e.getMessage());
				} catch (IOException e) {
					log.debug("Try to open SSL connecion " + e.getMessage());
				}

			 for (Object prop: props.keySet()) {
				 String value = props.getProperty((String)prop);
				 if( (((String)prop).equals("javax.net.ssl.trustStore")) || (((String)prop).equals("javax.net.ssl.keyStore")) ){
					 value = rootDir.concat(value);
				 }
			     System.getProperties().setProperty((String)prop,value);
			 }
		}catch(Exception ioe){
			log.debug("SSL properties file is not found under the expected folder," + ioe.getMessage());
		}		
	}
	

	
	private void outputSettings(ConfigurationContext configCtx){		
		if (log.isDebugEnabled()) {
			log.debug("config props");
			for (Iterator<?> iterator = configCtx.getPropertyNames(); iterator.hasNext();) {
				String name = (String) iterator.next();
				log.debug(name + ": " + configCtx.getProperty(name));
			}

			log.debug("Attributes:");
			Enumeration<?> en = servletCtx.getAttributeNames();
			while (en.hasMoreElements()) {
				String an = (String)en.nextElement();
				log.debug(an + ": " + servletCtx.getAttribute(an));
			}

			log.debug("InitParameters:");
			Enumeration<?> eni = servletCtx.getInitParameterNames();
			while (eni.hasMoreElements()) {
				String an = (String)eni.nextElement();
				log.debug(an + ": " + servletCtx.getInitParameter(an));
			}
		}

	}

}
