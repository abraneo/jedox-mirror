/**
 *   @brief
 *  
 *   @file
 *  
 *   Copyright (C) 2008-2013 Jedox AG
 *  
 *   @author Andreas Froehlich
 */
package com.jedox.etl.components.config.extract;

import java.util.List;
import java.util.Properties;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;

public class MetadataExtractConfigurator extends TableSourceConfigurator {

	private Properties properties;	

	public Properties getProperties() throws ConfigurationException {
		return properties;
	}
	
	protected void setProperties() throws ConfigurationException {
		Element query = getXML().getChild("query");
		String selector = query.getChildTextTrim("selector");
		if (selector==null || selector.isEmpty())
			throw new ConfigurationException("A selector is required.");
		properties = new Properties();
		properties.put("selector", selector);
		
		List<Element> filterList = getChildren(query,"filter");		
		//define aliases for output columns
		for (int i=0; i<filterList.size(); i++) {
			String name= filterList.get(i).getAttributeValue("name").trim();
			String value = filterList.get(i).getAttributeValue("value").trim();
			properties.put(name, value);
		}
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		setProperties();
	}

}
