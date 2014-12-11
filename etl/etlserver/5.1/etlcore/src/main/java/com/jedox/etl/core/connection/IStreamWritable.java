package com.jedox.etl.core.connection;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.writer.IWriter;

public interface IStreamWritable extends IConnection {
	
	/**
	 * gets the writer to write data to the file based backend
	 * @param filename the name of the file to write to
	 * @param append true if data should be appended, false if a new file is to be created
	 * @return
	 */
	public IWriter getWriter(boolean append) throws RuntimeException;

}
