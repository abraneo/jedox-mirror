package com.jedox.etl.components.load;

import java.io.File;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;

import com.jedox.etl.components.config.load.XMLTreeConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.TreeManagerProcessor;
import com.jedox.etl.core.util.XSLTUtil;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.node.xml.XMLDeNormalizer;
import com.jedox.etl.core.node.xml.XMLTreeManager;

public class XMLTreeLoad extends Load {

	private static final Log log = LogFactory.getLog(XMLTreeLoad.class);
	private String xslt;
	private String root;
	private XMLDeNormalizer denormalizer;	

	public XMLTreeLoad() {
		setConfigurator(new XMLTreeConfigurator());
	}

	public XMLTreeConfigurator getConfigurator() {
		return (XMLTreeConfigurator)super.getConfigurator();
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
	public void executeLoad() {
		try {
			log.info("Starting XML Tree load "+getName());
			XMLTreeManager xmlBridge = new XMLTreeManager(((ITreeSource)getView().getBaseSource()).generate().getManager());
			initTreeProcessor(new TreeManagerProcessor(xmlBridge),Facets.INPUT);
			Document document = denormalizer.denormalize(xmlBridge.getDocument());
				
			if (xslt != null) {
				File xsltFile = new File(xslt);			
				document=XSLTUtil.applyXslt(document,xsltFile);
			}	
			getConnection().save(document, getMode(), root, new Properties());
		}
		catch (Exception e) {
			log.error("Failed to write to file: "+e.getMessage());
			log.debug("",e);
		}
		log.info("Finished load "+getName());
	}
	
	public void test() throws RuntimeException {
		super.test();
		if (hasConnection())
			getConnection().test();
	}

	protected ConnectionManager getConnectionManager() {
		return (ConnectionManager)getManager(ITypes.Connections);
	}	
	
	
	public void init() throws InitializationException {
		super.init();
		try {
			IXmlConnection xsltConn=getConfigurator().getXSLTConnection();
			if (xsltConn!=null) {
				xslt=getConfigurator().getXSLTConnection().getDatabase();
				getConnectionManager().add(xsltConn);
			}	
			root = getParameter("root",null);
			denormalizer = getConfigurator().getDenormalizer();			
			if (!getView().isTreeBased()) {
				throw new InitializationException("XML Load needs tree based source.");
			}
		}
		catch (RuntimeException e) {
			throw new InitializationException(e);
		}
		catch (ConfigurationException e) {
			throw new InitializationException(e);
		}
	}

}
