package com.jedox.etl.core.scriptapi;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;


public class FilterAPI extends BaseAPI {
	
	private Map<String,Set<Object>> lookup = new HashMap<String,Set<Object>>();

	public boolean isUsable(IComponent component) {
		return (component instanceof IContext && super.isUsable(component));
	}
	
	private String getKey(String source, String column, String format) {
		return source+"|"+column+"|"+format;
	}
	
	public boolean isPresentInSource(String source, Object value, String column, String format) throws RuntimeException {
		Set<Object> values = lookup.get(getKey(source,column,format));
		if (values == null) {
			values = new HashSet<Object>();
			lookup.put(getKey(source,column,format), values);
			IProcessor processor = setupProcessor(source,format,0);
			Row row = processor.next();
			while (row != null) {
				IColumn c = row.getColumn(column);
				if (c == null) throw new RuntimeException("Column "+column+" does not exist in source "+source+((format==null)?"." : " in format "+format+"."));
				values.add(c.getValue());
				row = processor.next();
			}
		}
		return values.contains(value);
	}
	
	public boolean isPresentInSource(String source, Object value, String column) throws RuntimeException {
		return isPresentInSource(source,value,column,null);
	}
}
