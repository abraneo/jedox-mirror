/**
 * 
 */
package com.jedox.etl.core.config;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;

/**
 * @author khaddadin
 *
 */
public class VariablesDependencyFind extends VariablesDependencyHandler {
	
	private HashMap<String,LinkedHashSet<String>> refLists = new HashMap<String, LinkedHashSet<String>>();

	public VariablesDependencyFind(ArrayList<Locator> referenceLocators) {
		super(referenceLocators);
	}
	
	public HashMap<String,LinkedHashSet<String>> getDependents() throws ConfigurationException {
		try {
			for(Locator referenceLocator:referenceLocators){
				refLists.put(referenceLocator.getName(), new LinkedHashSet<String>());
			}
			cm = ConfigManager.getInstance().spawnReadOnlyInstance(referenceLocators.get(0).getRootLocator());
			handleVariables(cm);
		} catch (Exception e) {
			throw new ConfigurationException("Error in determination of dependencies for variables: "+e.getMessage());
		}
		return refLists;
	}

	/* (non-Javadoc)
	 * @see com.jedox.etl.core.config.VariablesDependencyHandler#elementFound(java.lang.String)
	 */
	@Override
	protected void elementFound(Locator reference) throws ConfigurationException {
		 refLists.get(reference.getName()).add(project.getAttributeValue("name")+"."+lastComponent.getName()+"s."+lastComponent.getAttributeValue("name"));
	}

}
