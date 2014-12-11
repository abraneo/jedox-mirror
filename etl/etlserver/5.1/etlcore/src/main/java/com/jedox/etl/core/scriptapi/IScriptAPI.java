package com.jedox.etl.core.scriptapi;


import java.util.Map;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.IComponent;

public interface IScriptAPI {
	
	public boolean isUsable(IComponent component);
	public String getExtensionPoint();
	public void setAPIDescriptor(ComponentDescriptor descriptor);
	public void setProperties(Map<String,Object> properties);
	public String[] getDefaultImports();
	public void close();

}
