/**
 * 
 */
package com.jedox.etl.core.config;

import java.util.ArrayList;
import java.util.HashMap;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;

/**
 * @author khaddadin
 *
 */
public class VariablesDependencyRename extends VariablesDependencyHandler {

	public VariablesDependencyRename(ArrayList<Locator> referenceLocators) {
		super(referenceLocators);
	}

	/* (non-Javadoc)
	 * @see com.jedox.etl.core.config.VariablesDependencyHandler#elementFound(java.lang.String)
	 */
	@Override
	protected void elementFound(Locator reference) throws ConfigurationException {
		cm.updateChangedInfo(this.project, lastComponent, this.session,false);
		cm.setDirty(reference);

	}
	
	public void rename(HashMap<String,String> newNames) throws ConfigurationException {
		try {	
			for(String oldName:this.newNames.keySet()){
				if(newNames.get(oldName)==null){
					throw new Exception(oldName + " does has a new name.");
				}else{
					this.newNames.put(oldName, newNames.get(oldName));
				}
				newNames.remove(oldName);
			}
			if(newNames.size()!=0){
				throw new Exception("Number of given references does not match the number of new names.");
			}
			cm = ConfigManager.getInstance();
			handleVariables(cm);
		} catch (Exception e) {
			throw new ConfigurationException("Error renaming variables: "+e.getMessage());
		}		
	}

}
