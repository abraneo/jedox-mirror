package com.jedox.etl.components.extract;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IFileConnection;
import com.jedox.etl.core.connection.IRelationalConnection;

public class FileExtract extends SQLExtract {
	
	public IFileConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IRelationalConnection)) {
			IFileConnection c = (IFileConnection) connection;
			c.setExternalParameter("skip", getConnectionParameter("skip","0"));
			c.setExternalParameter("columns", getConnectionParameter("columns","0"));
			c.setExternalParameter("end", getConnectionParameter("end","0"));
			c.setExternalParameter("start", getConnectionParameter("start","1"));
			return c;
		}
		throw new RuntimeException("File connection is needed for extract "+getName()+".");
	}
	
	private String getConnectionParameter(String key, String defaultValue) throws RuntimeException {
		try {
			return getParameter(key,defaultValue);
		}
		catch (ConfigurationException e) {
			throw new RuntimeException(e);
		}
	}

}
