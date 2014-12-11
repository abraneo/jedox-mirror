package com.jedox.etl.core.connection;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.processor.IProcessor;

public interface IStreamReadable extends IConnection {
	
	public IProcessor getProcessor(IAliasMap map, int size, long timeout) throws RuntimeException ;

}
