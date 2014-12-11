package com.jedox.etl.components.config.load;


import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.node.xml.XMLDeNormalizer;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IXmlConnection;

public class XMLTreeConfigurator extends FileConfigurator {
	
	private XMLDeNormalizer denormalizer;
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
		setXSLTConnection();
		setDeNormalizer();
	}	
}
