package com.jedox.etl.components.config.load;

import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;

public class JsonConfigurator extends FileConfigurator {
	
	public String getRootName() throws ConfigurationException{
		Element e = getXML().getChild("root");
		if(e!=null && !e.getTextTrim().isEmpty()){
			return e.getTextTrim();
		}
		return null;
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
	}	
}
