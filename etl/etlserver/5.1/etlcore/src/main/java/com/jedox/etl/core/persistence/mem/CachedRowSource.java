package com.jedox.etl.core.persistence.mem;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.h2.tools.SimpleRowSource;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.TypeConversionUtil;

public class CachedRowSource implements SimpleRowSource {
	
	private class RowSourceProcessor extends Processor {
		
		public RowSourceProcessor() {
		}
		
		@Override
		protected boolean fillRow(Row row) throws Exception {
			Object[] line = readRowInternal();
			if (line == null) return false;
			for (int i=0;i<row.size();i++) {
				row.getColumn(i).setValue(line[i]);
			}
			return true;
		}

		@Override
		protected Row getRow() throws RuntimeException {
			return row;
		}

		@Override
		protected void init() throws RuntimeException {
		}
	}
	
	private Row row;
	private List<Object[]> cache = new ArrayList<Object[]>();
	private int index = 0;
	private static final Log log = LogFactory.getLog(CachedRowSource.class);
    private MessageHandler aggLog;
    
	public CachedRowSource(RowSource baseRowSource) throws RuntimeException {
		row = baseRowSource.getBaseProcessor().current().clone();
		try {
			Object[] line = baseRowSource.readRow();
			while (line != null) {
				cache.add(line);
				line = baseRowSource.readRow();
			}
		}
		catch (SQLException e) {
			throw new RuntimeException(e);
		}
		aggLog = new MessageHandler(log);
	}
	
	private CachedRowSource(List<Object[]> cache, Row row, MessageHandler aggLog) {
		this.cache = cache;
		this.row = row;
		this.aggLog = aggLog;
	}

	private Object[] readRowInternal() {
		if (index < cache.size()) {
			return cache.get(index++);
		}
		return null;
	}
	
	@Override
	public Object[] readRow() throws SQLException {
		Object[]  values = readRowInternal();
		if ( values != null) {
			for (int i=0; i<values.length; i++) {
				try {
				   IColumn c = row.getColumn(i);	
				   c.setValue(values[i]);
	 			   values[i] = TypeConversionUtil.convert(row.getColumn(i));
	 		   }
	 		   catch (RuntimeException re) {
	 	    	 aggLog.warn(re.getMessage());
	 	       }
	 		   if (values[i] instanceof java.util.Date) {
	 			   values[i] = new java.sql.Date(((java.util.Date)values[i]).getTime());
	 		   }
			}
		}
		return  values;
	}

	@Override
	public void close() {
	}

	@Override
	public void reset() throws SQLException {
		index = 0;
	}
	
	public CachedRowSource newInstance() {
		return new CachedRowSource(cache,row,aggLog);
	}
	
	public void clear() {
		close();
		cache.clear();
	}
	
	public IProcessor getProcessor() {
		return newInstance().new RowSourceProcessor();
	}

}
