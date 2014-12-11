package com.jedox.etl.core.writer;

import java.io.BufferedReader;

import com.jedox.etl.core.component.RuntimeException;

public interface IFileWriter extends IWriter {
	
	public int writeFromReader(BufferedReader reader, int start, int end) throws RuntimeException ;
	
	public void writeEmptyLines(int lines);	
	
	public void setAutoClose(boolean autoclose);
	
	public void close();

}
