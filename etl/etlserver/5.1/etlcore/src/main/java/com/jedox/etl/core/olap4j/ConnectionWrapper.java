package com.jedox.etl.core.olap4j;

import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.main.ConnectionInfo;
import com.jedox.palojlib.main.SvsInfo;
import com.jedox.palojlib.main.UserInfo;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import org.olap4j.OlapConnection;
import org.olap4j.OlapException;
import org.olap4j.OlapWrapper;
import org.olap4j.driver.xmla.XmlaOlap4jDriver;
import org.olap4j.metadata.Catalog;


public class ConnectionWrapper implements IConnection {
	
	private class Olap4jConnectionInfo extends ConnectionInfo {
		public Olap4jConnectionInfo(String majorVersion, String minorVersion, String buildNumber, String encryptionType,
			String httpsPort) {
			super(majorVersion,minorVersion,buildNumber,buildNumber,encryptionType,httpsPort);
		}
	}
	
	
	//private static final Log log = LogFactory.getLog(ConnectionWrapper.class);
	
	private OlapConnection connection;
	private ConnectionConfiguration configuration;
	private Olap4jConnectionInfo info;
	private String connectionString;
	
	private Map<String, DatabaseWrapper> databaseLookup = new HashMap<String,DatabaseWrapper>();
	
	public ConnectionWrapper(String host, String port, String user, String password, String database) throws ClassNotFoundException, SQLException {
		
		Class.forName("org.olap4j.driver.xmla.XmlaOlap4jDriver");
		connectionString = "jdbc:xmla:Server="+host;
//		if (port!=null)
//			connectionString += ":"+port;
//		if (user!=null)
//			connectionString += ";User='" + user + "'";
//		if (password!=null)
//			connectionString += ";Password='" + password + "'";
		if (database!=null)
			connectionString += ";Catalog='" + database + "'";
		
		XmlaOlap4jDriver driver = (XmlaOlap4jDriver)DriverManager.getDriver(connectionString);
		configuration = new ConnectionConfiguration();
		configuration.setHost(host);
		configuration.setPassword(password);
		configuration.setPort(port);
		configuration.setSslPreferred(false);
		configuration.setTimeout(0);
		configuration.setUsername(user);
		String majorVersion = String.valueOf(driver.getMajorVersion());
		String minorVersion = String.valueOf(driver.getMinorVersion());
		String encryptionType = (connectionString.startsWith("https")) ? "2" : "0"; 
		info = new Olap4jConnectionInfo(majorVersion,minorVersion,driver.getVersion(),encryptionType,port != null ? port : "0");
	}

	@Override
	public ConnectionConfiguration getConnectionConfiguration() {
		return configuration;
	}

	@Override
	public ConnectionInfo getServerInfo() {
		return info;
	}

	@Override
	public boolean isConnected() {
		try {
			return (connection != null) && (!connection.isClosed());
		}
		catch (SQLException e) {
			return false;
		}
	}

	@Override
	public String open() throws PaloException {
		try {
			if (!isConnected()) {
				java.sql.Connection jdbcConnection = DriverManager.getConnection(connectionString,configuration.getUsername(),configuration.getPassword());
				OlapWrapper wrapper = (OlapWrapper) jdbcConnection;
				connection = wrapper.unwrap(OlapConnection.class);
			}
		}
		catch (SQLException e) {
			throw new PaloException("Cannot open connection "+connectionString+": "+e.getMessage());
		}
		
		return "";
	}

	public void close() throws PaloException {
		try {
			connection.close();
			for (DatabaseWrapper wrapper : databaseLookup.values()) {
				wrapper.clearLookup();
			}
			databaseLookup.clear();
		}
		catch (SQLException e) {
			throw new PaloException(e.getMessage());
		}
	}

	@Override
	public IDatabase[] getDatabases() throws PaloException {
		try {
			int size = connection.getOlapDatabases().size();
			IDatabase[] databases = new IDatabase[size];
			for (int i=0; i<size;i++) {
				databases[i] = databaseLookup.get(connection.getOlapCatalogs().get(i).getName());
				if (databases[i] == null) {
					DatabaseWrapper wrapper = new DatabaseWrapper(connection.getOlapCatalogs().get(i));
					databases[i] = wrapper;
					databaseLookup.put(wrapper.getName(), wrapper);
				}
			}
			return databases;
		}
		catch (OlapException e) {
			throw new PaloException(e.getMessage());
		}
	}

	@Override
	public IDatabase addDatabase(String name) throws PaloException {
		throw new PaloException("Creating databases not supported by OLAP4j provider.");
	}

	@Override
	public IDatabase getDatabaseByName(String name) throws PaloJException {
		IDatabase result = databaseLookup.get(name);
		if (result == null) {
			try {
				for (Catalog c : connection.getOlapCatalogs()) {
					if (c.getName().equals(name)) {
						DatabaseWrapper wrapper = new DatabaseWrapper(c);
						databaseLookup.put(wrapper.getName(), wrapper);
						return wrapper;
					}
				}
				throw new PaloJException("Database "+name+" cannot be found.");
			}
			catch (OlapException e) {
				throw new PaloJException(e.getMessage());
			}
		}
		return result;
	}

	@Override
	public void save() throws PaloException {
		//do nothing here, since we cannot create any objects using this wrapper anyway
		/*
		try {
			connection.commit();
		}
		catch (SQLException e) {
			throw new PaloException(e.getMessage());
		}
		*/
	}
	
	public OlapConnection getOlapConnection() {
		return connection;
	}

	@Override
	public SvsInfo getSvsInfo() throws PaloException, PaloJException {
		throw new PaloJException("SvsInfo not supported by OLAP4j provider.");
	}
	
	@Override
	public UserInfo getUserInfo(boolean arg0) throws PaloException,
			PaloJException {
		throw new PaloJException("getUserInfo not supported by OLAP4j provider.");
	}

	@Override
	public void removeDatabase(IDatabase arg0) throws PaloException,
			PaloJException {
		throw new PaloJException("removeDatabase not supported by OLAP4j provider.");
		
	}

	@Override
	public void close(boolean Stop) throws PaloException {
		throw new PaloJException("close(boolean Stop) not supported by OLAP4j provider.");
	}

	

}
