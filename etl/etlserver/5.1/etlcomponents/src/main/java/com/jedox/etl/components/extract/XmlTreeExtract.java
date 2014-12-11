package com.jedox.etl.components.extract;

import java.io.File;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import com.jedox.etl.components.config.extract.XmlTreeExtractConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.TreeManagerProcessor;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.core.util.XSLTUtil;

public class XmlTreeExtract extends TreeSource implements IExtract {
	
	private static final Log log = LogFactory.getLog(XmlTreeExtract.class);
	
	private String xslt;
	//private String root;
	private XMLNormalizer normalizer;
	
	public XmlTreeExtract() {
		setConfigurator(new XmlTreeExtractConfigurator());
	}

	public XmlTreeExtractConfigurator getConfigurator() {
		return (XmlTreeExtractConfigurator)super.getConfigurator();
	}

	public IXmlConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IXmlConnection))
			return (IXmlConnection) connection;
		throw new RuntimeException("XML connection is needed for extract "+getName()+".");
	}
	
	
	public ITreeProcessor buildTree() throws RuntimeException {
		Document document = getConnection().open();
		if (xslt != null) {
			File xsltFile = new File(xslt);			
			document=XSLTUtil.applyXslt(document,xsltFile);
			log.debug(XMLUtil.w3cDocumentToString(document));
		}
		normalizer.normalize(document);
		TreeManagerNG treeBridge = new TreeManagerNG(normalizer);
		setTreeManager(treeBridge);
		return initTreeProcessor(new TreeManagerProcessor(treeBridge),Facets.HIDDEN);	
	}

	protected ConnectionManager getConnectionManager() {
		return (ConnectionManager)getManager(ITypes.Connections);
	}	
	
	public void init() throws InitializationException {
		super.init();
		IXmlConnection xsltConn=getConfigurator().getXSLTConnection();
		if (xsltConn!=null) {
			xslt=getConfigurator().getXSLTConnection().getDatabase();
			try {
				getConnectionManager().add(xsltConn);
			}
			catch (Exception e) {
				throw new InitializationException(e.getMessage());
			}				
		}	
		normalizer = getConfigurator().getNormalizer();				
	}

}
