package com.jedox.etl.components.config.transform;

public class RConfigurator extends TableTransformConfigurator {
	
	public String getScript() {
		return getXML().getChildTextTrim("script");
	}
	
	public String getDataset() {
		return getXML().getChild("script").getAttributeValue("dataset", getName());
	}
	
	public boolean useMemoryBuffer() {
		return getXML().getChild("script").getAttributeValue("buffer", "file").equalsIgnoreCase("memory");
	}

}
