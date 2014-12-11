package com.jedox.etl.core.scriptapi;


import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ComponentDescriptor;

public abstract class ScriptAPI implements IScriptAPI {
	
	protected static final Log log = LogFactory.getLog(ScriptAPI.class);

	private ComponentDescriptor descriptor;
	protected Properties properties = new Properties();

	@Override
	public String getExtensionPoint() {
		return descriptor.getParameters().getProperty("extensionPoint", descriptor.getName());
	}

	@Override
	public void setAPIDescriptor(ComponentDescriptor descriptor) {
		this.descriptor = descriptor;
	}
	
	public void setProperties(Properties properties) {
		this.properties.putAll(properties);
	}
	
	

}
