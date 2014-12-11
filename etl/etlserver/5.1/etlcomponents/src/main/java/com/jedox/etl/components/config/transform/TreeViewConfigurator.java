package com.jedox.etl.components.config.transform;

import org.jdom.Element;

import com.jedox.etl.components.config.extract.DimensionExtractConfigurator;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView;

public class TreeViewConfigurator extends DimensionExtractConfigurator {

	private ITreeSource source;
	private TransformConfigUtil util;
	
	protected void setSource() throws ConfigurationException {
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
	
	public DimensionFilterDefinition getDimensionFilterDefinition() throws ConfigurationException {
		Element filter = getXML().getChild("query");
		Element scripts = getXML().getChild("scripts");
		return new DimensionFilterDefinition(getContext(),filter,scripts);
	}
	
	protected void setDimension() {
		//no dimension present
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		util = new TransformConfigUtil(getXML(),getLocator(),getContext());
		setSource();
	}
	
	protected TransformConfigUtil getConfigUtil() {
		return util;
	}

}
