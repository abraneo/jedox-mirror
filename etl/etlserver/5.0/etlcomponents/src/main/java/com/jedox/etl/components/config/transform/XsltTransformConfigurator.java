package com.jedox.etl.components.config.transform;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView;

public class XsltTransformConfigurator extends TreeSourceConfigurator {
	
	private ITreeSource source;
	
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
	}
	
	

}
