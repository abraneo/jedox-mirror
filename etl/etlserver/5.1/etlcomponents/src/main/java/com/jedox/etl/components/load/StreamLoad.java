package com.jedox.etl.components.load;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.load.LoadConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IStreamWritable;
import com.jedox.etl.core.load.Load;

public class StreamLoad extends Load {
	
	private static final Log log = LogFactory.getLog(StreamLoad.class);
	
	public StreamLoad() {
		setConfigurator(new LoadConfigurator());
	}
	
	protected String getEncoding() throws RuntimeException {
		if (getConnection() != null) {
			return getConnection().getEncoding();
		}
		return null;
	}
	
	//get connection for writing.
	public IStreamWritable getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IStreamWritable)) {
			return (IStreamWritable)connection;
		}
		throw new RuntimeException("Stream writable connection is needed for load "+getName()+".");
	}

	@Override
	public void executeLoad() {
		try {
			getConnection().getWriter(false).write(getProcessor());
		}
		catch (Exception e) {
			log.error("Failed to write to stream: "+e.getMessage());
			//e.printStackTrace();
		}
	}

}
