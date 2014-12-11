package com.jedox.etl.core.util.docu;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.core.util.svg.GraphUtilFactory;
import com.jedox.etl.core.util.svg.IGraphUtil;

public class DocuUtil {

	Element projElem;
	IProject project;

	public DocuUtil(IProject project, Locator focusComponent, String[] graphLocators, Properties graphProperties) throws ConfigurationException {
		this.project = project;
		List<Locator> invalidComponents = ConfigManager.getInstance().initProjectComponents((IProject)project, IContext.defaultName, true); 
		projElem = getProjectDescription(focusComponent);
		
		addGraphsToDocumentation(graphLocators);
		
		if (!invalidComponents.isEmpty()) {
			Element invalid = new Element("invalid");
			projElem.addContent(invalid);
			for (Locator l : invalidComponents) {
				Element c = new Element(l.getType());
				c.setText(l.getName());
				invalid.addContent(c);
			}
		}		
	}
	
	private Element getBlockElement(String type, IContext context, List<String> includeList) throws ConfigurationException {
		Element typeElement = new Element(type.toString().toLowerCase());
		List<IComponent> comps = Arrays.asList(context.getManager(type).getAll());
		Collections.sort(comps);		
		for (IComponent c : comps) {
			if(includeList==null || includeList.contains(c.getLocator().toString()))
				typeElement.addContent(c.getComponentDescription());
		}
		return typeElement;
	}
		
	private Element getProjectDescription(Locator focusComponent) throws ConfigurationException {
		Element root = new Element("project");
		root.setAttribute("name",project.getName());
		// Only dependent components of the focused component should be part of description
		List<String> includeList=null;
		if (focusComponent!=null && !focusComponent.isRoot()) {
			Map<String,List<String>> deps = project.getAllDependencies(focusComponent.toString(),false);
			includeList = new ArrayList<String>();
			includeList.add(focusComponent.toString());
			for(String key:deps.keySet()) {
				includeList.addAll(deps.get(key));
			}
			Element component = new Element("component");
			component.setAttribute("class", focusComponent.getType());
			component.setAttribute("name", focusComponent.getName());
			root.addContent(component);									
		}
		
		Element version = new Element("version");
		version.setText(String.valueOf(Settings.versionetl));
		root.addContent(version);
		Element date = new Element("date");
		date.setText(new SimpleDateFormat("dd.MM.yyyy hh:mm").format(new Date()));
		root.addContent(date);
		Element description = new Element("description");
		description.setText(project.getConfigurator().getXML().getChildTextTrim("comment"));
		root.addContent(description);
		IContext context = project.getContextManager().get(IContext.defaultName);
		Element variables = new Element("variables");
		List<?> variablesElement = project.getConfigurator().getXML().getChild("variables").getChildren("variable");
		for (Object key : variablesElement) {
			Element varElement = (Element)key;
			Element var = new Element("variable");
			var.setAttribute("name", varElement.getAttributeValue("name"));
			Element varDefault = new Element("default");
			varDefault.setText(varElement.getTextTrim());
			var.addContent(varDefault);
			variables.addContent(var);
		}
		root.addContent(variables);
		
		root.addContent(getBlockElement(ITypes.Connections,context,includeList));
		root.addContent(getBlockElement(ITypes.Extracts,context,includeList));
		root.addContent(getBlockElement(ITypes.Transforms,context,includeList));
		root.addContent(getBlockElement(ITypes.Loads,context,includeList));
		root.addContent(getBlockElement(ITypes.Jobs,context,includeList));

		Element functionsElem = new Element("functions");
		List<IComponent> comps = Arrays.asList(context.getManager(ITypes.Transforms).getAll());
		Collections.sort(comps);		
		for (IComponent c : comps) {
			FunctionManager functionManager = (FunctionManager)c.getManager(ITypes.Functions);
			if (functionManager != null) {
				List<IFunction> functions = Arrays.asList(functionManager.getAll());
				Collections.sort(functions);
				for (IFunction f : functions) {				
					if(includeList==null ||  includeList.contains(c.getLocator().toString())){
						Element functionElem = f.getComponentDescription();
						Element fdependants = new Element("usedby");
						Element fdepthis = new Element(ITypes.Components.transform.toString());
						fdepthis.setText(c.getName());
						fdependants.addContent(fdepthis);
						functionElem.addContent(fdependants);
						functionsElem.addContent(functionElem);
					}
				}
			}	
		}
		root.addContent(functionsElem);
		
		return root;
	}
	
	
	private void addGraphsToDocumentation(String[] graphLocators) throws ConfigurationException {
		if (graphLocators==null || graphLocators.length==0)
			return;
		Element graphs = new Element("graphs");			
		Properties props = new Properties();
		props.setProperty("viewType", "dependencies");			
		props.setProperty("linkPrefix", "#");
		props.setProperty("onClick", "");
		for (String graphLocator : graphLocators) {
			Locator loc = Locator.parse(graphLocator);
			// Check if component exists
			ConfigManager.getInstance().getComponent(loc, IContext.defaultName);
			Element graph = new Element("graph");
			graph.setAttribute("class", loc.getType());
			graph.setAttribute("name", loc.getName());				
			try {
				IGraphUtil util = GraphUtilFactory.getGraphUtil(project, graphLocator, props, new ArrayList<Locator>());
				graph.addContent(util.getXML());
				graphs.addContent(graph);
			} catch (ConfigurationException e) { }
		}
		projElem.addContent(graphs);
	}
	
	public String getDocu() throws ConfigurationException { 
		try {
			return XMLUtil.jdomToString(projElem);
		} catch (IOException e) {
			throw new ConfigurationException(e);
		}
	}	

	public Element getElement() throws ConfigurationException { 
        return projElem;
	}	
	
}
