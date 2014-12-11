package com.jedox.etl.components.extract;

import java.io.StringReader;

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

import com.jedox.etl.components.config.extract.XmlTreeExtractConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.source.TreeSource;

public class XmlTreeExtract extends TreeSource implements IExtract {
	
	private static final Log log = LogFactory.getLog(XmlTreeExtract.class);
	
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
	
	protected ITreeManager buildTree() throws RuntimeException {
		Document document = getConnection().open();
		try {
			String xslt = getParameter("xslt",null);
			if (xslt != null && !xslt.isEmpty()) {
				StreamSource xsltSource = new StreamSource(new StringReader(xslt));
				TransformerFactory tFactory = TransformerFactory.newInstance();
				Transformer transformer;
				transformer = tFactory.newTransformer(xsltSource);
				DOMSource source = new DOMSource(document);
				DOMResult result = new DOMResult();
				transformer.setOutputProperty(OutputKeys.INDENT,"yes");
				transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");
				transformer.transform(source, result); 
				document = (Document) result.getNode();
				/*
				StreamResult debug = new StreamResult(System.out);
				transformer.transform(source, debug);
				System.out.flush();
				*/
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
		XMLNormalizer normalizer = getConfigurator().getNormalizer();
		normalizer.normalize(document);
		TreeManagerNG treeBridge = new TreeManagerNG(normalizer);
		setTreeManager(treeBridge);
		/*
		try {
			
				TransformerFactory tFactory = TransformerFactory.newInstance();
				Transformer transformer;
				transformer = tFactory.newTransformer();
				DOMSource source = new DOMSource(normalizer.getDocument());
				transformer.setOutputProperty(OutputKeys.INDENT,"yes");
				transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");
				StreamResult debug = new StreamResult(System.out);
				transformer.transform(source, debug);
				System.out.flush();
		}
		catch (TransformerConfigurationException e) {
			throw new RuntimeException(e);
		}
		catch (TransformerException e) {
			throw new RuntimeException(e);
		}
		*/
		return getTreeManager();	
	}



}
