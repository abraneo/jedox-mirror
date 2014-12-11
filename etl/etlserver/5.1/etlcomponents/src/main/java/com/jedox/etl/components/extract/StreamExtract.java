package com.jedox.etl.components.extract;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IStreamReadable;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;

/**
 * 
 * @author Christian Schwarzinger
 * License: GPLv2
 */

public class StreamExtract extends TableSource implements IExtract {
	
	private long timeout;
	private int columns;
	
	public StreamExtract() {
		setConfigurator(new TableSourceConfigurator());
	}
	
	public IStreamReadable getConnection() throws RuntimeException
	{
		IConnection connection = super.getConnection();
		if((connection != null) && (connection instanceof IStreamReadable)) return (IStreamReadable)connection;
		throw new RuntimeException("Streamable connection is needed for extract " + getName() + ".");
	}

	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = getConnection().getProcessor(getAliasMap(),size,timeout);
		for (int i=1; i<=columns; i++) {
			processor.current().addColumn(new Column(getAliasMap().getAlias(i,"column"+Integer.toString(i))));
		}
		return initProcessor(processor,Facets.OUTPUT);
	}
	
	public Row getOutputDescription() throws RuntimeException {
		return getAliasMap().getOutputDescription();
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			columns = Integer.valueOf(getConfigurator().getParameter("columns", "1"));
			timeout = Long.valueOf(getConfigurator().getParameter("timeout", "0"));
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
