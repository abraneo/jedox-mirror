package com.jedox.etl.core.scriptapi;


import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.context.IContext;

public abstract class ScriptAPI implements IScriptAPI {
	
	protected static final Log log = LogFactory.getLog(ScriptAPI.class);

	private ComponentDescriptor descriptor;
	private Map<String,Object> apiProperties = new HashMap<String,Object>();
	private String projectName;
	private IContext context;

	@Override
	public String getExtensionPoint() {
		return descriptor.getParameters().getProperty("extensionPoint", descriptor.getName());
	}
	
	@Override
	public String[] getDefaultImports() {
		return descriptor.getParameters().getProperty("import", "").split(";");
	}

	@Override
	public void setAPIDescriptor(ComponentDescriptor descriptor) {
		this.descriptor = descriptor;
	}
	
	public void setProperties(Map<String,Object> properties) {
		this.apiProperties = properties;
	}
	
	/**
	 * sets a variable with a given value.
	 * @param name the name of the variable
	 * @param value the value of the variable.
	 */
	public void setProperty(@Scanable(type=ITypes.Managers.variables) String name, String value) {
/* 
		Object contextValue = getComponentContext().getExternalVariables().get(name);
		if(contextValue!=null && !contextValue.equals(value))
			log.info("Value " + value + " for variable "+ name+" overides the externally set value " + contextValue + " only for the scope of the script-job itself.");		
*/
		//set in shared api properties. This properties are not stored in the context, but rather used to create children contexts of the given component context 
		apiProperties.put(name, value);
	}

	/**
	 * gets the value of a variable set via {@link #setProperty(String, String)} before
	 * @param name the name of the variable
	 * @return the variable value
	 */
	public Object getProperty(@Scanable(type=ITypes.Managers.variables) String name) {
		Object result = apiProperties.get(name); //first look in (shared) api properties
		if (result == null) result = context.getVariables().get(name); //then look in context variables
		if (result == null) result = context.getParameter().get(name); //finally look in context parameters
		return result;
	}
	
	protected Properties getApiProperties() {
		Properties properties = new Properties();
		for (String key : apiProperties.keySet()) {
			Object value = apiProperties.get(key);
			if (value != null)
				properties.setProperty(key, value.toString());
		}
		return properties;
	}

	/**
	 * clears all variables set prior via {@link #setProperty(String, String)}
	 */
	public void clearProperties() {
		apiProperties.clear();
	}
	
	public boolean isUsable(IComponent component) {
		this.projectName = component.getLocator().getRootName();
		this.context = ((component instanceof IContext) ? (IContext) component : component.getContext());
		//usable for all types of components
		return true;
	}
	
	protected String getProjectName() {
		return projectName;
	}
	
	protected IContext getComponentContext() {
		return context;
	}
	
	public void close() {
		context = null;
		apiProperties.clear();
	}
	

}
