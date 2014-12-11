package com.jedox.etl.components.config.load;


import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.node.xml.XMLDeNormalizer;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.component.RuntimeException;

public class XMLConfigurator extends FileConfigurator {
	
	private XMLDeNormalizer denormalizer;
	
	public XMLDeNormalizer getDenormalizer() {
		return denormalizer;
	}
	
	private void setDeNormalizer() throws ConfigurationException {
		try {
			denormalizer = new XMLDeNormalizer();
			Element n = getXML().getChild("denormalizer");
			if (n != null) {
				String elementSource = n.getChildTextTrim("element");
				String contentSource = n.getChildTextTrim("content");
				if (elementSource != null && elementSource.isEmpty()) elementSource = XMLNormalizer.defaultElementTarget;
				if (contentSource != null && contentSource.isEmpty()) contentSource = XMLNormalizer.defaultContentTarget;
				denormalizer.setElementSource(elementSource);
				denormalizer.setContentSource(contentSource);
			}
		}
		catch (RuntimeException e) {
			throw new ConfigurationException(e);
		}
	}
	
	
	public void configure() throws ConfigurationException {
		super.configure();
		setDeNormalizer();
	}

	
}
