package com.jedox.etl.core.persistence.mem;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.generic.adaptor.H2;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;

/**
 *
 * @author chris
 */
public class MemPersistor extends H2 {
    private String quote = "\"";
    private static MemPersistor instance;
    
    public static MemPersistor getInstance() {
    	if (instance == null) {
    		instance = new MemPersistor();
    	}
    	return instance;
    }

    private MemPersistor() {
    	super(null);
    }
    
    protected String getCustomEngine() {
    	return " ENGINE "+quote(TableEngine.class.getName());
    }
    
    private String quote(String name) {
        return (quote+name.replaceAll(quote, "")+quote);
    }
    
    @Override
	protected Connection setupConnection() throws RuntimeException {
    	 try {
             Class.forName("org.h2.Driver");
             return DriverManager.getConnection("jdbc:h2:mem:", "sa", "");
             //return DriverManager.getConnection("jdbc:h2:"+Settings.getInstance().getPersistenceDir() + "/mem", "sa", "");
         } catch (ClassNotFoundException cnfe) {
                 throw new RuntimeException("H2 Driver not found: "+cnfe.getMessage());
         } catch (SQLException sqle) {
                 sqle.printStackTrace();
                 throw new RuntimeException("Failed to open internal h2 connection "+sqle.getMessage());
         }
		
	}
    
    public void disconnectThread(){
	}
    
    public boolean isCached(Locator locator) {
    	try {
    		getTableResult(locator, 1).close();
    		//System.err.println(locator.getPersistentName());
    		return true;
    	}
    	catch (Exception e) {
    		return false;
    	}
    }
    
    private Row wrapColumns(Row row) {
    	Row result = new Row();
    	for (IColumn c : row.getColumns()) {
    		RelationalNode node = ColumnNodeFactory.getInstance().createRelationalNode(c.getName(), c);
    		result.addColumn(node);
    	}
    	return result;
    }
    
    public void cache(IProcessor processor, Locator locator) throws RuntimeException {
	    String tableName = locator.getPersistentName();
	    TableEngine.addData(tableName.toUpperCase(), processor);
	    this.createTable(locator, wrapColumns(processor.current()));
    }
    
    public static void close() {
    	if (instance != null) {
    		instance.disconnect();
    		try {
    			if (instance.hasConnection()) instance.getConnection().close();
    		}
    		catch (Exception e) {}
    	}
    }
    
    public static boolean isActive() {
    	return instance != null;
    }
    
    public void dropTable(Locator locator) throws RuntimeException {
    	super.dropTable(locator);
    	TableEngine.removeTable(locator.getPersistentName().toUpperCase());
    }

}

