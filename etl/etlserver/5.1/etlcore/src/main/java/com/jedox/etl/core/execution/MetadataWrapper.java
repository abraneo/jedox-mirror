package com.jedox.etl.core.execution;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.util.MetadataWriter;

public class MetadataWrapper extends Wrapper {
	
	public enum MetadataSelectors {
		selectorname, selectorfilter
	}
		
	private static final Log log = LogFactory.getLog(MetadataWrapper.class);
	private Properties properties;
	
	public MetadataWrapper(IComponent component,Properties properties) {
		super(component);
		this.properties = properties;
	}

		
	private StringWriter getSelectorWriter(IConnection connection) throws RuntimeException{
		// get all possible criterias from a connection
		StringWriter out = new StringWriter();
		PrintWriter writer = new PrintWriter(out);
		try {
			writer.println("Criteria");
			for (MetadataCriteria c: connection.getMetadataCriterias()) {
				writer.println(c.getName());				
			}		
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get Metadata: "+e.getMessage());
		}
		return out;
	}	
		
	private StringWriter getSelectorFilterWriter(IConnection connection, String selector) throws RuntimeException{
		// get all possible filters of a criteria from a connection	
		MetadataCriteria criteria=connection.getMetadataCriteria(selector);
		if (criteria==null)
			throw new RuntimeException("Given selector "+selector+" is not valid");			
		StringWriter out = new StringWriter();
		MetadataWriter writer = new MetadataWriter(out);
		try {
			writer.println("Filter");
			for (String filter : criteria.getFilters()) {
				writer.println(filter);								
			}
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get Metadata: "+e.getMessage());
		}
		return out;
	}		
	
	public void execute() {
		try {
			if (getComponent() instanceof IConnection) {
				IConnection connection = ((IConnection)getComponent());
				String selector = properties.getProperty("selector");
				MetadataSelectors sel;
				try {
					sel = MetadataSelectors.valueOf(selector);
				}
				catch (Exception e) {
					getState().setData(connection.getMetadata(properties));
					return;
				};
				switch (sel) {
				case selectorname: getState().setData(getSelectorWriter(connection).toString());  break;
				case selectorfilter: getState().setData(getSelectorFilterWriter(connection, properties.getProperty("selectorname")).toString()); break; 
				}
			}
			else throw new RuntimeException("Component "+getComponent().getLocator().toString()+" is not a connection.");
		} catch (RuntimeException e) {
			log.error(e.getMessage());
		} 
	}

}
