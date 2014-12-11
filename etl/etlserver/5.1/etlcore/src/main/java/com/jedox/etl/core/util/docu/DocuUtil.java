package com.jedox.etl.core.util.docu;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.VariablesDependencyFind;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.core.util.svg.GraphManager;

public class DocuUtil {

	Element projElem;
	IProject project;
	private static final Log log = LogFactory.getLog(DocuUtil.class);

	public DocuUtil(IProject project, Locator focusComponent, String[] graphLocators, Properties graphProperties) throws ConfigurationException, NumberFormatException{
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
		log.info("Building the header part of the documentation.");
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
		//
		Element date = new Element("date");
		date.setText(new SimpleDateFormat("dd.MM.yyyy HH:mm").format(new Date()));
		root.addContent(date);
		//
		String modified=project.getConfigurator().getXML().getAttributeValue("modified");
		if (modified!=null) {		
			Element modifiedElem = new Element("modified");
			modifiedElem.setText(new SimpleDateFormat("dd.MM.yyyy HH:mm").format(Double.parseDouble(modified)));
			root.addContent(modifiedElem);
		}	
		//
		Element projectXML = project.getConfigurator().getXML();
		Element description = new Element("description");
		Element headers = projectXML.getChild("headers");
		if(headers!=null){
			Element header = headers.getChild("header");
			if(header!=null){
				Element comment = header.getChild("comment");
				if(comment!=null)
					description.setText(comment.getTextTrim());
			}
		}
		root.addContent(description);
		//
		IContext context = project.getContextManager().get(IContext.defaultName);
		Element variables = new Element("variables");
		//Properties vars = project.getConfigurator().getVariables();
		log.info("Building the variables part of the documentation.");
		List<?> variablesInputList = new ArrayList<Element>();
		Element variablesNode = ConfigManager.getInstance().get(project.getLocator()).getChild("variables");
		if(variablesNode!=null)
			variablesInputList = variablesNode.getChildren("variable");
		ArrayList<Locator> variableLocators = new ArrayList<Locator>();
		HashMap<String, LinkedHashSet<String>> map = null;
		for (Object varObj : variablesInputList) {
			Element varElement = (Element)varObj;			
			Locator loc = project.getLocator().clone().add(ITypes.Managers.variables.toString()).add(varElement.getAttributeValue("name"));
			variableLocators.add(loc);
		}
		
		if(variableLocators.size()!=0){
			VariablesDependencyFind varHandler = new VariablesDependencyFind(variableLocators);
			map = varHandler.getDependents();
		}
		for (Object varObj : variablesInputList) {
			Element varElement = (Element)varObj;
			Element var = new Element("variable");
			var.setAttribute("name", varElement.getAttributeValue("name"));
			//VariableDependencyHandler varHandler = new VariableDependencyHandler(Locator.parse(project.getName() + ".variables." + varElement.getAttributeValue("name")));
			Element varDesc = new Element("description");
			if(varElement.getChild("comment")!=null)
				varDesc.setText(varElement.getChildTextTrim("comment"));
			Element varDefault = new Element("default");
			if(varElement.getChild("default")!=null)
				varDefault.setText(varElement.getChildTextTrim("default"));
			var.addContent(varDesc);
			var.addContent(varDefault);
			Element varUsedBy = new Element("usedby");
			HashSet<String> dependents = map.get(varElement.getAttributeValue("name"));
			for(String dep:dependents){
				Locator depLoc = Locator.parse(dep);
				Element depElement = new Element(depLoc.getType());
				depElement.setText(depLoc.getName());
				varUsedBy.addContent(depElement);
			}
			if(dependents.size()!=0)
				var.addContent(varUsedBy);
			variables.addContent(var);
		}
		root.addContent(variables);
		
		log.info("Building the connections part of the documentation.");
		root.addContent(getBlockElement(ITypes.Connections,context,includeList));
		log.info("Building the extracts part of the documentation.");
		root.addContent(getBlockElement(ITypes.Extracts,context,includeList));
		log.info("Building the transforms part of the documentation.");
		root.addContent(getBlockElement(ITypes.Transforms,context,includeList));
		log.info("Building the loads part of the documentation.");
		root.addContent(getBlockElement(ITypes.Loads,context,includeList));
		log.info("Building the jobs part of the documentation.");
		root.addContent(getBlockElement(ITypes.Jobs,context,includeList));

		log.info("Building the functions part of the documentation.");
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
		log.info("Finished building the XML documentation file.");
		return root;
	}
	
	
	private void addGraphsToDocumentation(String[] graphLocators) throws ConfigurationException, NumberFormatException {
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
			//ConfigManager.getInstance().getComponent(loc, IContext.defaultName);
			Element graph = new Element("graph");
			graph.setAttribute("class", loc.getType());
			graph.setAttribute("name", loc.getName());				
			try {
				String str = GraphManager.getInstance().getSVG(project, graphLocator, props, new ArrayList<Locator>(),Long.MIN_VALUE);
				org.jdom.Element svgElement=null;
				try {
					svgElement = XMLUtil.stringTojdom(str);
					svgElement.detach();			
				} catch (Exception e) {
					throw new RuntimeException(e);
				} 
				graph.addContent(svgElement);
				graphs.addContent(graph);
			} catch (ConfigurationException e) { throw e; } 
			catch (RuntimeException e1) {
				//should never happen, id is always -1
			}
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
