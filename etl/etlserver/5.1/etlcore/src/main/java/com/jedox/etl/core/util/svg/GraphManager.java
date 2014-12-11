package com.jedox.etl.core.util.svg;
import java.util.List;
import java.util.Properties;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.project.IProject;

public class GraphManager{
	
	private static GraphManager manager=null;
	
	private GraphManager(){
	}
	
	public static synchronized GraphManager getInstance(){
		if(manager==null){
			manager = new GraphManager();
		}
		return manager;
	}
	
	public synchronized String getSVG(IProject project, String componentName, Properties graphProperties, List<Locator> invalidComponentsLocs, Long id) throws ConfigurationException, RuntimeException {
		Locator compLoc = Locator.parse(componentName);
		if(ConfigManager.getInstance().findElement(compLoc)==null){
			throw new ConfigurationException(compLoc.getManager().substring(0, compLoc.getManager().length()-1) + " " + compLoc.getName() + " was not found.");
		}
		GraphUtilNG gu = new GraphUtilNG(project, componentName, graphProperties, invalidComponentsLocs,id);
		gu.generate();
		return gu.getSVG();
	}
	
}
