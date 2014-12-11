package com.jedox.etl.components.config.transform;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.xml.XMLDeNormalizer;
import com.jedox.etl.core.node.xml.XMLNormalizer;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView;

public class XsltTransformConfigurator extends TreeSourceConfigurator {
	
	private ITreeSource source;
	
	private XMLNormalizer normalizer;
	private XMLDeNormalizer denormalizer;
	
	public XMLDeNormalizer getDenormalizer() {
		return denormalizer;
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

	private void setDenormalizer(XMLNormalizer normalizer) throws ConfigurationException {
		try {
			denormalizer = new XMLDeNormalizer();
			denormalizer.setElementSource(normalizer.getElementTarget());
			denormalizer.setContentSource(normalizer.getContentTarget());
		} catch (RuntimeException e) {
			throw new ConfigurationException(e.getMessage());
		}		
	}
	
	
	protected void setSource() throws ConfigurationException {
		TransformConfigUtil util = new TransformConfigUtil(getXML(),getLocator(),getContext());
		IComponent source = util.getSources().get(0);
		if (source instanceof IView) {
			IView view = (IView)source;
			if (view.isTreeBased()) {
				this.source = (ITreeSource)source;
			}
			else throw new ConfigurationException("Source has to be tree based.");
		}
	}
	
	public ITreeSource getSource() {
		return source;
	}
	
	public String getXslt() {
		return getXML().getChildTextTrim("xslt");
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		setSource();
		setNormalizer();
		setDenormalizer(getNormalizer());
	}
	
	

}
