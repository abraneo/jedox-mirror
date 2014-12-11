package com.jedox.etl.core.scriptapi;


import java.util.Properties;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.IComponent;

public interface IScriptAPI {
	
	public boolean isUsable(IComponent component);
	public String getExtensionPoint();
	public void setAPIDescriptor(ComponentDescriptor descriptor);
	public void setProperties(Properties properties);
	public void close();

}
