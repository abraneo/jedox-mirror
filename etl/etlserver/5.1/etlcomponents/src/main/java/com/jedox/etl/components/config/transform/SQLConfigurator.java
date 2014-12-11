package com.jedox.etl.components.config.transform;

import java.util.List;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.Row;

public class SQLConfigurator extends TableSourceConfigurator implements
		ITransformConfigurator {

	private Row row = new Row();
	private TransformConfigUtil util;

	public Row getRow() throws RuntimeException {
		return row;
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		return util.getFunctions();
	}
	
	public boolean isPersistent() {
		return getXML().getChild("query").getAttributeValue("persist", "false").equalsIgnoreCase("true");
	}

	public void configure() throws ConfigurationException {
		super.configure();
		try {
			util = new TransformConfigUtil(getXML(), getLocator(), getContext());
		} catch (Exception e) {
			throw new ConfigurationException("Failed to configure transform "
					+ getName() + ": " + e.getMessage());
		}
	}
}
