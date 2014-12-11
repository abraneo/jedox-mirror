
package com.jedox.etl.components.connection;

import java.util.Properties;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;


public class AccessJdbcConnection extends RelationalConnection {
	
	protected String getDatabaseString() {
		return "//" + super.getDatabaseString();
	}
	
	public void init() throws InitializationException {
		super.init();
		boolean logOff = true;;
		try {
			logOff = Boolean.parseBoolean(getParameter("logLevelOff","true"));
			if (logOff) {
				Logger.getLogger("com.healthmarketscience.jackcess").setLevel(Level.OFF);
			}
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
	}
	
	protected Properties getConnectionParameters() {
		
		Properties props = super.getConnectionParameters();
	    if(!props.containsKey("jackcessOpener")){
	    	props.setProperty("jackcessOpener", "com.jedox.etl.components.connection.extensions.CryptCodecOpener");
	    }
	    return props;
	}
}
