/**
*   @brief <Description of Class>
*  
*   @file
*  
*   Copyright (C) 2008-2013 Jedox AG
*  
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License (Version 2) as published
*   by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
*  
*   This program is distributed in the hope that it will be useful, but WITHOUT
*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*   more details.
*  
*   You should have received a copy of the GNU General Public License along with
*   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
*   Place, Suite 330, Boston, MA 02111-1307 USA
* 
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs, 
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible  
*   OEM Commercial License.
*  
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right 
*   (commercial copyright) has Jedox AG, Freiburg.
*  
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.project;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

import org.jdom.Element;

import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.util.docu.DocuUtil;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Project extends Component implements IProject {
	
	private HashMap<String,List<String>> dependencyMapWithVariables = null;
	// Dependencies for Variables not yet implemented in this version
	private HashMap<String,List<String>> dependencyMapWithoutVariables = null;
	
	public Project() {
		setConfigurator(new BasicConfigurator());
	}
	
	public ContextManager getContextManager() {
		return (ContextManager)getManager(ITypes.Contexts);
	}
	
	public void test() throws RuntimeException {
		//context to be tested in is set and all components are already initialized by ConfigManager via Executor calling ConfigManager#getComponent
		super.test();
		//test all loads of the project
		for (IComponent load :getContextManager().get(getContextName()).getManager(ITypes.Loads).getAll()) {
			load.test();
		}
	}
	
	private Map<String,IComponent> getConnectionMap() {
		HashMap<String,IComponent> result = new HashMap<String,IComponent>();
		for (IComponent connection : getContextManager().get(IContext.defaultName).getManager(ITypes.Connections).getAll()) 
			result.put(connection.getLocator().toString(),connection);
		return result;
	}
	
	public List<IComponent> getModelComponents() {
		ArrayList<IComponent> result = new ArrayList<IComponent>();
		for (IComponent connection : getContextManager().get(IContext.defaultName).getManager(ITypes.Connections).getAll()) 
			result.add(connection);
		for (IComponent extract : getContextManager().get(IContext.defaultName).getManager(ITypes.Extracts).getAll()) 
			result.add(extract);
		for (IComponent transform : getContextManager().get(IContext.defaultName).getManager(ITypes.Transforms).getAll()) 
			result.add(transform);
		for (IComponent load : getContextManager().get(IContext.defaultName).getManager(ITypes.Loads).getAll()) 
			result.add(load);
		for (IComponent job : getContextManager().get(IContext.defaultName).getManager(ITypes.Jobs).getAll()) 
			result.add(job);
		return result;	
	}
	
	private String getBaseComponent (IComponent component) {
		if (component instanceof IView) //use base source of view, since view is no independent component from project view.
			return ((IView)component).getBaseSource().getLocator().toString();
		else
			return component.getLocator().toString();		
	}
	
	
	public Map<String,List<String>> getDependencyMap(boolean includeVariables) {
		
		if(includeVariables && this.dependencyMapWithVariables!=null)
			return this.dependencyMapWithVariables;
		else if(!includeVariables && this.dependencyMapWithoutVariables!=null)
			return this.dependencyMapWithoutVariables;
		
		HashMap<String,List<String>> dependencyMap = new HashMap<String,List<String>>();
		for (IComponent c : getModelComponents()) {
			ArrayList<String> qnames = new ArrayList<String>();
			for (IComponent d : c.getDependencies(new ArrayList<IComponent>(), false)) {
				// functions are not part of this dependency map, but the dependencies of the functions have to be considered				
				if (d instanceof IFunction) {
					for (IComponent dfun : d.getDependencies(new ArrayList<IComponent>(), false)) {
						if (dfun instanceof IFunction) {}
						// ToDo: a function may currently not have a dependency on a function. 
						else 
							qnames.add(getBaseComponent(dfun));
					}
				}	
				else 
					qnames.add(getBaseComponent(d));
			}		
			dependencyMap.put(c.getLocator().toString(), qnames);	
		}	
		
		// if needed collect the variables as well
		// not yet imlemented in this version!
/*		
		if(includeVariables){
			for (Object variableObj: getContextManager().get(IContext.defaultName).getVariables().keySet()){
				try {
					Locator variableLoc  = Locator.parse(getName()+".variables."+variableObj.toString()); 
					ArrayList<Locator> dependents = new VariableDependencyHandler(variableLoc).getDependents();
					for(Locator dependent : dependents){
						dependencyMap.get(dependent.toString()).add(variableLoc.toString());
					}
					dependencyMap.put(variableLoc.toString(), new ArrayList<String>());
				} catch (Exception e)  {
					
				}		
			}
			this.dependencyMapWithVariables = dependencyMap;
		}else{
*/		
			this.dependencyMapWithoutVariables = dependencyMap;
//		}
		return dependencyMap;
	}

	private List<String> getDirectDependencies(Map<String,List<String>> map, String qname) {
		return map.get(qname);
	}
	
	public List<String> getDirectDependencies(String qname,boolean includeVariables) {
		return getDirectDependencies(getDependencyMap(includeVariables),qname);
	}
	
	public Map<String,List<String>> getAllDependencies (String qname,boolean includeVariables) {
		return getAllDependencies(getDependencyMap(includeVariables),qname);

	}
	
	private  Map<String,List<String>> getAllDependencies(Map<String,List<String>> map, String qname) {
		HashMap<String,List<String>> result = new HashMap<String,List<String>>();		
		List<String> directDeps = getDirectDependencies(map,qname);
		if (directDeps != null) {
			result.put(qname, directDeps);
				// get the dependencies of each dependent component
			for (String n : directDeps) {
				if (!getConnectionMap().containsKey(n)) //do not resolve deps of connections
					result.putAll(getAllDependencies(map,n));
			}	
		}
		return result;
	}
	
	private List<String> getDirectDependents(Map<String,List<String>> map, String qname) {
		List<String> out = new ArrayList<String>();
		for (String k : map.keySet()) {
			List<String> someDeps = map.get(k);
			if (someDeps.contains(qname)) {
				out.add(k);
			}
		}
		return out;				
	}
	
	public List<String> getDirectDependents(String qname,boolean includeVariables) {
		return getDirectDependents(getDependencyMap(includeVariables),qname);
	}
	
	private Map<String,List<String>> getAllDependents(Map<String,List<String>> map, String qname) {
		HashMap<String,List<String>> result = new HashMap<String,List<String>>();
		//direct dependents
		List<String> out = getDirectDependents(map,qname);
		result.put(qname, out);
		// get the dependents of each dependent component
		for (String k : out) {
			if (!getConnectionMap().containsKey(k)) //do not resolve deps of connections
				result.putAll(getAllDependents(map,k));
		}
		return result;
	}

	public Map<String,List<String>> getAllDependents(String qname,boolean includeVariables) {
		return getAllDependents(getDependencyMap(includeVariables),qname);
	}
	
	
	public Element getComponentDescription() throws ConfigurationException {
		DocuUtil util = new DocuUtil(this, null, null, null);
		return util.getElement();
	}
	
	public void invalidate(){
		this.dependencyMapWithoutVariables =null;
		this.dependencyMapWithVariables = null;
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			addManager(new ContextManager());
			getContextManager().provideDefault();	
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
