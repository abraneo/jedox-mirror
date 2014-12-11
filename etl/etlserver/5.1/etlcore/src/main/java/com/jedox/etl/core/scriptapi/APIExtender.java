package com.jedox.etl.core.scriptapi;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.IManager.LookupModes;
import com.jedox.etl.core.util.CustomClassLoader;

public class APIExtender {
	
	private static final String removeCommentRegExpFrom1 = "((?s).+?)(/\\*(?s).+?\\*/)((?s).+?)";
	private static final String removeCommentRegExpTo1 = "$1$3";
	private static final String removeCommentRegExpFrom2 = "\n[\\s]*//.+?\n";
	private static final String removeCommentRegExpTo2 = "\n";
	
	public class ScriptDependenciesDescriptor {
		public IComponent component;
		public ParameterScan scan;
		public String methodString;
		public String[] parameters;
	}
	
	public class ScriptDependenciesResult {
		public IManager manager;
		public List<ScriptDependenciesDescriptor> descriptors;
	}
	
	public class ParameterScan {
		
		public ITypes.Managers type;
		public int position;
		
		public ParameterScan(ITypes.Managers type, int position) {
			this.type = type;
			this.position = position;
		}
	}
	
	private static APIExtender instance = new APIExtender();
	private static final Log log = LogFactory.getLog(APIExtender.class);

    private Map<String,Map<String,List<ParameterScan>>> scanMethodsBuffer = new HashMap<String,Map<String,List<ParameterScan>>>();
	
	
	public static APIExtender getInstance() {
		return instance;
	}
	/* Fallback APIS
	<scriptapis>
		<component name="JobAPI"
			class="com.jedox.etl.components.scriptapi.JobAPI">
			<parameter name="extensionPoint">API</parameter>
		</component>
		<component name="FunctionAPI"
			class="com.jedox.etl.components.scriptapi.FunctionAPI">
			<parameter name="extensionPoint">API</parameter>
		</component>
		<component name="OlapAPI"
			class="com.jedox.etl.components.scriptapi.OlapAPI">
			<parameter name="extensionPoint">OLAP</parameter>
		</component>
	</scriptapis>
	*/
	protected List<ComponentDescriptor> getFallbackAPIs() {
		List<ComponentDescriptor> result = new ArrayList<ComponentDescriptor>();
		ComponentDescriptor job = new ComponentDescriptor("JobAPI", "com.jedox.etl.components.scriptapi.JobAPI", "scriptapis", false, "JobAPI", null,null,null);
		job.addParameter("extensionPoint", "API");
		ComponentDescriptor function = new ComponentDescriptor("FunctionAPI", "com.jedox.etl.components.scriptapi.FunctionAPI", "scriptapis", false, "FunctionAPI", null,null,null);
		function.addParameter("extensionPoint", "API");
		ComponentDescriptor olap = new ComponentDescriptor("OlapAPI", "com.jedox.etl.components.scriptapi.OlapAPI", "scriptapis", false, "OlapAPI", null,null,null);
		olap.addParameter("extensionPoint", "OLAP");
		ComponentDescriptor filter = new ComponentDescriptor("FilterAPI", "com.jedox.etl.core.scriptapi.FilterAPI", "scriptapis", false,"FilterAPI", null,null,null);
		filter.addParameter("extensionPoint", "API");
		result.add(job);
		result.add(function);
		result.add(olap);
		result.add(filter);
		return result;
	}
	
	public Map<String,IScriptAPI> getAPIExtensions(IComponent component) {
		Map<String,IScriptAPI> result = new HashMap<String,IScriptAPI>();
		List<ComponentDescriptor> descriptors = ComponentFactory.getInstance().getComponentDescriptors("scriptapis");
		if (descriptors.isEmpty()) descriptors = getFallbackAPIs(); //fallback to standard, if there are no defined because of old component config
		for (ComponentDescriptor descriptor : descriptors) {
			try {
				Class<?> apiClass = CustomClassLoader.getInstance().loadClass(descriptor.getClassName());
				IScriptAPI api = (IScriptAPI) apiClass.newInstance();
				api.setAPIDescriptor(descriptor);
				if (api.isUsable(component)) {
					result.put(api.getExtensionPoint(), api);
				}
			}
			catch (Exception e) {
				if (e instanceof ClassNotFoundException) {
					log.warn("Class "+descriptor.getClassName()+" not found for scriptapi of type "+descriptor.getName()+". Please check component.xml.");
				}
				else {
					log.warn("Error initializing script api extension "+descriptor.getName()+" "+e.getMessage());
				}
			}
		}
		return result;
	}
	
	public IManager getGuessedDependencies(IComponent component, String script) throws ConfigurationException {
		List<String> scripts = new ArrayList<String>();
		scripts.add(script);
		return getGuessedDependencies(component, scripts).manager;
	}
	
	private Map<String,List<ParameterScan>> getScanMethods (IScriptAPI scriptAPI) {
		String classname=scriptAPI.getClass().getCanonicalName();
		if (scanMethodsBuffer.containsKey(classname)) {
			return scanMethodsBuffer.get(classname);
		} 
		else {
            Class<?> c = scriptAPI.getClass();
            Method[] methods = c.getMethods();
            Map<String,List<ParameterScan>> scanMethods = new HashMap<String,List<ParameterScan>>();
            for (Method m : methods) {
            	Annotation[][] parameterAnnotation = m.getParameterAnnotations();
            	List<ParameterScan> scanList = new ArrayList<ParameterScan>();
            	for (int i=0; i<parameterAnnotation.length; i++) {
            		for (Annotation a : parameterAnnotation[i]) {
            			if (a instanceof Scanable) {
            				ParameterScan ps = new ParameterScan(((Scanable)a).type(),i);
            				scanList.add(ps);
            			}
            		}
            	}
            	if (!scanList.isEmpty()) scanMethods.put(scriptAPI.getExtensionPoint()+"."+m.getName(), scanList);
            }	
            scanMethodsBuffer.put(classname, scanMethods);
            return scanMethods;
		}
	}
	
	public ScriptDependenciesResult getGuessedDependencies(IComponent component, List<String> scripts) throws ConfigurationException {
		ScriptDependenciesResult result = new ScriptDependenciesResult();
		IManager manager = new MixedManager();
		manager.setLookupMode(LookupModes.Locator);
		result.manager = manager;
		result.descriptors = new ArrayList<ScriptDependenciesDescriptor>();
		Map<String,IScriptAPI> extensions = APIExtender.getInstance().getAPIExtensions(component);
		try {
			Map<String,List<ParameterScan>> scanMethods = new HashMap<String,List<ParameterScan>>();			
			for (String extensionPoint : extensions.keySet()) {
				scanMethods.putAll(getScanMethods(extensions.get(extensionPoint)));
			}				
			for (String scriptOriginal : scripts) {
				
				//clear the comments
				log.debug("script before removing the comments: " + scriptOriginal);
				String scriptReduced = (" " + scriptOriginal + " ").replaceAll(removeCommentRegExpFrom1, removeCommentRegExpTo1);
				scriptReduced = scriptReduced.replaceAll(removeCommentRegExpFrom2, removeCommentRegExpTo2);
				log.debug("script after removing the comments: " + scriptReduced);
				
	            for (String methodName : scanMethods.keySet()) {
	            	String scanString = methodName+"(";
	            	int lastIndex = 0;
	            	while (lastIndex != -1) {
	            		lastIndex = scriptReduced.indexOf(scanString, lastIndex);
	            		if (lastIndex != -1) {
	            			int start = lastIndex+scanString.length();
	            			int end = scriptReduced.indexOf(")", start);
	            			String parameterString = scriptReduced.substring(start, end).trim();
	            			String[] parameters = parameterString.split("\\,(?=([^\"]*\"[^\"]*\")*[^\"]*$)");
	            			List<ParameterScan> scans= scanMethods.get(methodName);
	            			for (ParameterScan scan : scans) {
	            				String parameter = (scan.position < parameters.length) ? parameters[scan.position] : null;
	            				if (parameter != null && parameter.startsWith("\"") && parameter.endsWith("\"")) {
		            				String componentName = parameter.substring(1, parameter.length()-1);
		            				try {
		            					IComponent depComponent = component.getContext().getComponent(new Locator().add(component.getLocator().getRootName()).add(scan.type.toString()).add(componentName));
		            					manager.add(depComponent);
		            					ScriptDependenciesDescriptor descriptor = new ScriptDependenciesDescriptor();
		            					descriptor.component = depComponent;
		            					descriptor.scan = scan;
		            					descriptor.methodString = scanString;
		            					descriptor.parameters = parameters;
		            					result.descriptors.add(descriptor);
		            				}
		            				catch (Exception e) {
		            				   log.debug("Dependency guess on component "+component.getName()+" failed: "+e.getMessage());
		            				}
		            			}
	            			}
	            			lastIndex += scanString.length();
	            		}
	            	}
	            }
            }
        }
        catch (Exception e) {
           throw new ConfigurationException("Error while guessing script dependencies: "+e.getMessage());
        }
		return result;
	}

}
