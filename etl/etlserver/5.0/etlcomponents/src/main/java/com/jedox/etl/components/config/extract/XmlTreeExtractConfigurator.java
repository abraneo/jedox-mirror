package com.jedox.etl.components.config.extract;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.component.RuntimeException;

public class XmlTreeExtractConfigurator extends TreeSourceConfigurator {
	
	private XMLNormalizer normalizer;
	
	public String getXslt() {
		return getXML().getChildTextTrim("xslt");
	}
	
	public XMLNormalizer getNormalizer() {
		return normalizer;
	}
	
	private void setNormalizer() throws ConfigurationException {
		try {
			normalizer = new XMLNormalizer();
			Element n = getXML().getChild("normalizer");
			if (n != null) {
				String elementTarget = n.getChildTextTrim("element");
				String contentTarget = n.getChildTextTrim("content");
				if (elementTarget != null && elementTarget.isEmpty()) elementTarget = XMLNormalizer.defaultElementTarget;
				if (contentTarget != null && contentTarget.isEmpty()) contentTarget = XMLNormalizer.defaultContentTarget;
				normalizer.setElementTarget(elementTarget);
				normalizer.setContentTarget(contentTarget);
			}
		}
		catch (RuntimeException e) {
			throw new ConfigurationException(e);
		}
	}
	
	
	public void configure() throws ConfigurationException {
		super.configure();
		setNormalizer();
	}


}
