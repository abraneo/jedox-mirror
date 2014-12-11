package com.jedox.etl.components.config.load;


import org.jdom.Element;
import org.jdom.IllegalNameException;
import com.jedox.etl.core.component.ConfigurationException;

public class XMLConfigurator extends FileConfigurator {
	
	public String getRootName() throws ConfigurationException{
		Element e = getXML().getChild("root");
		if(e!=null && !e.getTextTrim().isEmpty()){
			try{
				new Element(e.getTextTrim());
				return e.getTextTrim();
			}catch(IllegalNameException ine){
				throw new ConfigurationException(e.getTextTrim() + " is illigal value for xml element name.");
			}
		}
		return null;
	}
	
	public String getRowName() throws ConfigurationException{
		Element e = getXML().getChild("row");
		if(e!=null && !e.getTextTrim().isEmpty()){
			try{
				new Element(e.getTextTrim());
				return e.getTextTrim();
			}catch(IllegalNameException ine){
				throw new ConfigurationException(e.getTextTrim() + " is illigal value for xml element name.");
			}
		}
		return null;
	}
	
	public boolean getColumnNameAsTag(){

		Element e = getXML().getChild("columnNameAsTag");
		if(e!=null){
			return Boolean.valueOf(e.getTextTrim());
		}
		return false;
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
	}	
}
