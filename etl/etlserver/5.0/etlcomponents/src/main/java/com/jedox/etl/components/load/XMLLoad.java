package com.jedox.etl.components.load;

import java.io.StringReader;
import java.util.Properties;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;

import com.jedox.etl.components.config.load.XMLConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.node.xml.XMLDeNormalizer;
import com.jedox.etl.core.node.xml.XMLTreeManager;

public class XMLLoad extends Load {

	private static final Log log = LogFactory.getLog(XMLLoad.class);

	public XMLLoad() {
		setConfigurator(new XMLConfigurator());
	}

	public XMLConfigurator getConfigurator() {
		return (XMLConfigurator)super.getConfigurator();
	}

	protected String getEncoding() throws RuntimeException {
		if (getConnection() != null) {
			return getConnection().getEncoding();
		}
		return null;
	}

	//get connection for writing.
	public IXmlConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IXmlConnection)) {
			IXmlConnection c = (IXmlConnection) connection;
			return c;
		}
		throw new RuntimeException("XML File connection is needed for load "+getName()+".");
	}


	@Override
	public void execute() {
		if (isExecutable()) {
			try {
				log.info("Starting XML File load "+getName());
				Properties properties = new Properties();
				properties.put("mode", getMode().toString());
				XMLTreeManager xmlBridge = new XMLTreeManager(((ITreeSource)getView().getBaseSource()).generate());
				XMLDeNormalizer denormalizer = getConfigurator().getDenormalizer();
				Document document = denormalizer.denormalize(xmlBridge.getDocument());
				try {
					String xslt = getParameter("xslt",null);
					String root = getParameter("root",null);
					if (root != null) properties.put("root", root);
					if (xslt != null && !xslt.isEmpty()) {
						StreamSource xsltSource = new StreamSource(new StringReader(xslt));
						TransformerFactory tFactory = TransformerFactory.newInstance();
						Transformer transformer;
						transformer = tFactory.newTransformer(xsltSource);
						DOMSource source = new DOMSource(document);
						DOMResult result = new DOMResult();
						transformer.setOutputProperty(OutputKeys.INDENT,"yes");
						transformer.transform(source, result); 
						document = (Document) result.getNode();
					}
				}
				catch (ConfigurationException e) {
					throw new RuntimeException(e);
				} catch (TransformerConfigurationException e) {
					throw new RuntimeException(e);
				}
				catch (TransformerException e) {
					throw new RuntimeException(e);
				}
				getConnection().save(document, properties);
			}
			catch (Exception e) {
				log.error("Failed to write to file: "+e.getMessage());
				log.debug("",e);
			}
			log.info("Finished load "+getName());
	}
	}
	
	public void test() throws RuntimeException {
		super.test();
		if (hasConnection())
			getConnection().test();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			if (!getView().isTreeBased()) {
				throw new InitializationException("XML Load needs tree based source.");
			}
		}
		catch (RuntimeException e) {
			throw new InitializationException(e);
		}
	}

}
