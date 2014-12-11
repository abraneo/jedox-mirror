package com.jedox.etl.components.connection;


public class OracleConnection extends RelationalConnection {
	
	protected String getConnectionString(String parameter) {
		String connectionString = getProtocol();
		connectionString += "@";
		if (getHost() != null) connectionString += getHost();
		if (getPort() != null) connectionString += ":"+getPort();
		if (getHost() != null && getDatabase() != null) connectionString += ":";
		if (getDatabase() != null) connectionString += getDatabase();
		//parameter += "FetchSize=1000;";
		if (parameter.length() > 0) {
			connectionString += ":"+parameter;
		}
		return connectionString;
	}
	
	public boolean isSchemaSupported () {
		return true;
//      todo: Check for Oracle XE which doesn't support Schemas		
	}
	
}
