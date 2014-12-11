package com.jedox.etl.core.connection;

import java.util.ArrayList;
import java.util.Properties;

import com.jedox.etl.core.component.InitializationException;


public class MetadataCriteria {
	
	private String name;
	private ArrayList<String> filterList = new ArrayList<String>();		

	
	public MetadataCriteria(String name) {
		this.name = name;
	}
	
	public void setFilters(String[] filters) {
		filterList.clear();
		for (String f : filters)
			filterList.add(f);
		// this.filters = filters;
	}
	
	public String[] getFilters() {
		return filterList.toArray(new String[filterList.size()]);
	}
	
	public String getName() {
		return name;
	}
	
	public void checkFilters(Properties properties) throws InitializationException {
		for (Object key : properties.keySet()) {
			String filter = (String)key;
			if (!filter.equals("selector") && !filterList.contains(filter))
				throw new InitializationException("Filter criteria "+key.toString()+" not possible in selector "+getName());		
		}
	}

}
