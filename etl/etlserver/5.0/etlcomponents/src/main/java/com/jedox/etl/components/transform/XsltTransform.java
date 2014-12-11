package com.jedox.etl.components.transform;

import java.io.StringReader;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;

import org.w3c.dom.Document;

import com.jedox.etl.components.config.transform.XsltTransformConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.xml.XMLTreeManager;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.transform.ITransform;

public class XsltTransform extends TreeSource implements ITransform {
	
	private ITreeSource source;
	private String xslt;
	
	public XsltTransform() {
		setConfigurator(new XsltTransformConfigurator());
	}

	public XsltTransformConfigurator getConfigurator() {
		return (XsltTransformConfigurator)super.getConfigurator();
	}
	
	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}
	
	public IOLAPConnection getConnection() throws RuntimeException {
		return null; //no connection needed.
	}
	
	protected ITreeManager buildTree() throws RuntimeException {
		try {
			if (xslt != null && !xslt.isEmpty()) {
				XMLTreeManager xmlBridge = new XMLTreeManager(source.generate());
				Document document = xmlBridge.getDocument();
				StreamSource xsltSource = new StreamSource(new StringReader(xslt));
				TransformerFactory tFactory = TransformerFactory.newInstance();
				Transformer transformer;
				transformer = tFactory.newTransformer(xsltSource);
				DOMSource source = new DOMSource(document);
				DOMResult result = new DOMResult();
				transformer.setOutputProperty(OutputKeys.INDENT,"yes");
				transformer.transform(source, result); 
				document = (Document) result.getNode();
				xmlBridge = new XMLTreeManager(document);
				setTreeManager(new TreeManagerNG(xmlBridge));
			}
			else {
				setTreeManager(source.generate());
			}
		} catch (TransformerConfigurationException e) {
			throw new RuntimeException(e);
		}
		catch (TransformerException e) {
			throw new RuntimeException(e);
		}
		return getTreeManager();
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			source = getConfigurator().getSource();
			getSourceManager().add(source);
			xslt = getConfigurator().getXslt();
		}
		catch (Exception e) {
			throw new InitializationException("In transform " + getConfigurator().getName() + ": " + e.getMessage());
		}
	}

}
