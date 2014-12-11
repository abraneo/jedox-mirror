package com.jedox.etl.components.transform;

import org.w3c.dom.Document;

import com.jedox.etl.components.config.transform.XsltTransformConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.xml.XMLDeNormalizer;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.node.xml.XMLTreeManager;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.TreeManagerProcessor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.util.XSLTUtil;

public class XsltTransform extends TreeSource implements ITransform {
	
	private ITreeSource source;
	private String xslt;
	private XMLNormalizer normalizer;
	private XMLDeNormalizer denormalizer;	
	
	
	
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
	
	public ITreeProcessor buildTree() throws RuntimeException {
		if (xslt != null && !xslt.isEmpty()) {
			try {			
				XMLTreeManager xmlBridge = new XMLTreeManager(source.generate().getManager());
				Document document = denormalizer.denormalize(xmlBridge.getDocument());
				document=XSLTUtil.applyXslt(document, xslt);			
				normalizer.normalize(document);
				setTreeManager(new TreeManagerNG(normalizer));
			} catch (RuntimeException e) {
				throw new RuntimeException("Error in transform "+getName()+": "+e.getMessage());
			}
		}	
		else {
			setTreeManager(source.generate().getManager());
		}
		return initTreeProcessor(new TreeManagerProcessor(getTreeManager()),Facets.HIDDEN);
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			source = getConfigurator().getSource();
			getSourceManager().add(source);
			xslt = getConfigurator().getXslt();
			normalizer = getConfigurator().getNormalizer();				
			denormalizer = getConfigurator().getDenormalizer();			
			
		}
		catch (Exception e) {
			throw new InitializationException("In transform " + getConfigurator().getName() + ": " + e.getMessage());
		}
	}

}
