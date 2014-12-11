/**
 * 
 */
package com.jedox.palojlib.test;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.util.Properties;

import org.apache.log4j.Level;

import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.managers.LoggerManager;

/**
 * @author khaddadin
 * 
 */
public class TestSettings {

	private static TestSettings instance;
	private Properties props = new Properties();

	private TestSettings() {
	}

	public static TestSettings getInstance() {
		if (instance == null) {
			instance = new TestSettings();
			instance.loadProperties();
		}
		return instance;
	}

	private void loadProperties() {

		InputStream input = null;

		try {

			input = new FileInputStream("config/testconfig.properties");
			instance.props.load(input);

		} catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		} finally {
			try {
				if (input != null)
					input.close();
			} catch (IOException e) {
			}
		}
	}

	public void setConfigFromFile(IConnectionConfiguration config) {
		config.setHost((String) props.get("host"));
		config.setPort((String) props.get("port"));
		config.setUsername((String) props.get("user"));
		config.setPassword((String) props.get("password"));
		config.setTimeout(Integer.parseInt((String) props.get("timeout")));
		config.setSslPreferred(Boolean.parseBoolean((String) props.get("sslPreferred")));
	}

	public void setSSL() {
		// sets the keystore path
		try {
			Properties props = new Properties();
			File propFile = new File("config/ssl.properties");
			try {
				props.load(propFile.toURI().toURL().openStream());
			} catch (MalformedURLException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}

			for (Object prop : props.keySet()) {
				System.getProperties().setProperty((String) prop,
						props.getProperty((String) prop));
			}
		} catch (Exception ioe) {
			ioe.printStackTrace();
		}
	}
	
	public void setDebugLevel(){
		LoggerManager.getInstance().setLevel(Level.DEBUG);
	}

}
