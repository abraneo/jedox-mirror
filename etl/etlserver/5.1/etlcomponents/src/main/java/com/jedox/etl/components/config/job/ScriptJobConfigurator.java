/**
 *   @brief <Description of Class>
 *  
 *   @file
 *  
 *   Copyright (C) 2008-2013 Jedox AG
 *  
 *  
 *   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.components.config.job;

import java.util.LinkedList;
import java.util.List;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.IManager.LookupModes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.job.JobConfigurator;
import com.jedox.etl.core.scriptapi.MixedManager;


public class ScriptJobConfigurator extends JobConfigurator {

	private LinkedList<String> scripts = new LinkedList<String>();

	private void setScripts() throws ConfigurationException {
		List<Element> scriptList = getChildren(getXML(), "jobscript");
		if (scriptList.size() == 0)
			throw new ConfigurationException("A script should be included in job " + getName() + ".");
			
		for (Element e : scriptList) {
			String script = e.getTextTrim();
			if (script.trim().isEmpty())
				throw new ConfigurationException("The script in job " + getName() + " is empty.");				
			scripts.add(e.getTextTrim());
		}
	}

	public List<String> getScripts() {
		return scripts;
	}
	
	public IManager getDeclaredDependencies() throws ConfigurationException {
		IManager manager = new MixedManager();
		Element uses = getXML().getChild("uses");
		if (uses != null) {
			@SuppressWarnings("unchecked")
			List<Element> depList = uses.getChildren();
			manager.setLookupMode(LookupModes.Locator);
			for (Element element : depList) {
				try {
					manager.add(getContext().getComponent(new Locator().add(getLocator().getRootName()).add(element.getName()+"s").add(element.getAttributeValue("nameref"))));
				}
				catch (Exception e) {
					throw new ConfigurationException(element.getName()+" "+element.getAttributeValue("nameref") + " is not a valid dependency: "+e.getMessage());
				}
			}
		}
		return manager;
	}
	
	

	@Override
	public void configure() throws ConfigurationException {
		super.configure();
		if (getOwnVariables().size()>0)
			throw new ConfigurationException("The definition of static variables in script job " + getName() + " is no longer supported. Use method setProperty() in script instead.");
		setScripts();
	}

}
