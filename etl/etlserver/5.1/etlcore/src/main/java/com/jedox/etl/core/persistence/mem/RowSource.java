package com.jedox.etl.core.persistence.mem;

import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;

import java.sql.SQLException;

import org.h2.tools.SimpleRowSource;

/**
 *
 * @author chris
 */
public class RowSource implements SimpleRowSource {

    private IProcessor processor;


    public RowSource(IProcessor processor) {
        this.processor = processor;
    }

    @Override
    public void close() {
       processor.close();
    }

    @Override
    public Object[] readRow() throws SQLException {
    	try {
	    	Row row = processor.next();
	    	if (row != null) {
	    		Object[] values = new Object[row.size()];
	    		for (int i=0; i<row.size(); i++) {
	    		   values[i] = row.getColumn(i).getValue();
	    		}
	    		return values;
	    	}
    	} 
    	catch (Exception e) {
    		throw new SQLException(e);
    	}
        return null;
    }

    @Override
    public void reset() throws SQLException {
        throw new SQLException("Reset not supported.");
    }
    
    protected IProcessor getBaseProcessor() {
    	return processor;
    }

}

