package com.jedox.etl.components.config.extract;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.component.RuntimeException;

public class XmlTreeExtractConfigurator extends TreeSourceConfigurator {
	
	private XMLNormalizer normalizer;
	private IXmlConnection xsltConnection;
		
	public void setXSLTConnection() throws ConfigurationException {
		Element xslt = getXML().getChild("xslt");
		if(xslt!=null){		
			String connectionID = xslt.getChild("connection").getAttributeValue("nameref");						
			IConnection connection = (IConnection)getContext().getComponent(getLocator().getRootLocator().add(ITypes.Connections).add(connectionID));
			if (connection instanceof IXmlConnection)
			   xsltConnection = (IXmlConnection) connection;
			else
				throw new ConfigurationException("XML connection is needed for XSTL in extract "+getName()+".");
		}
	}	
	
	public IXmlConnection getXSLTConnection() {
		return xsltConnection;
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
		setXSLTConnection();
		setNormalizer();
	}


}
