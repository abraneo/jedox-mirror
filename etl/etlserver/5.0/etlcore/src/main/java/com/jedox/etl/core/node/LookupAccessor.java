package com.jedox.etl.core.node;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * DRAFT FOR FUTURE EXTENSION - NOT IN USE NOW
 * @author chris
 *
 */
public class LookupAccessor {
	
	private Row lookupRow;
	private Map<String,List<List<Object>>> lookup = new HashMap<String,List<List<Object>>>();
	
	public LookupAccessor(IProcessor source, Row keyRow, Row lookupRow) throws RuntimeException {
		this.lookupRow = lookupRow;
		Row row = source.next();
		while (row != null) {		
			String key = concatKeyValues(keyRow);
			List<List<Object>> data = lookup.get(key);
			if (data == null) {
				data = new ArrayList<List<Object>>();
				lookup.put(key, data);
			}
			//get Column values
			List<Object> values = new ArrayList<Object>();
			for (IColumn c : lookupRow.getColumns()) {
				values.add(c.getValue());
			}
			data.add(values);
			row = source.next();
		}
	}
	
	private String concatKeyValues(Row row) {
		StringBuffer buf = new StringBuffer();
		for (IColumn c : row.getColumns()) {
			buf.append("@");
			buf.append(c.getValueAsString());
		}
		return buf.toString();
	}
	
	public RowBuffer lookup(Row keyRow) throws RuntimeException {
		String key = concatKeyValues(keyRow);
		List<List<Object>> result = lookup.get(key);
		if (result != null && !result.isEmpty()) {
			RowBuffer buffer = new RowBuffer();
			buffer.addColumns(lookupRow);
			buffer.addValues(result);
			buffer.moveToFirst();
			return buffer;
		}
		return null;
	}
	
	
	
}
