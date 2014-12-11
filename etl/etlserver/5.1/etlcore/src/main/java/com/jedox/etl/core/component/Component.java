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
package com.jedox.etl.core.component;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.jdom.Element;

import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.InternalConnectionManager;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.IdProcessor;

/**
 * Abstract base implementation for a component
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Component extends Locatable implements IComponent {

	private IConfigurator configurator;
	private HashMap<String,IManager> managers = new HashMap<String,IManager>();
	private boolean isDirty = false;

	public void setConfigurator(IConfigurator configurator) {
		this.configurator = configurator;
	}

	protected IRelationalConnection getInternalConnection() throws RuntimeException {
		try {
			IRelationalConnection global = InternalConnectionManager.getInstance().getInternalConnection();
			if (getContext() != null) {
				IRelationalConnection c = (IRelationalConnection)getContext().getManager(ITypes.Connections).get(global.getLocator().toString());
				if (c == null) {
					ConnectionManager m = new ConnectionManager();
					c = (IRelationalConnection) m.add(global.getConfigurator().getXML());
					getContext().getManager(ITypes.Connections).add(c);
					getLocator().setQuote(c.getIdentifierQuote());
				}
				return c;
			}
			return global;
		} catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	public IConfigurator getConfigurator() {
		return configurator;
	}

	public Properties getParameter() {
		return getConfigurator().getParameter();
	}

	/**
	 * adds a {@link IManager manager} to this component with the specified {@link IManager.LookupModes lookup} mode.
	 * @param manager the manager
	 * @param mode the mode
	 */
	protected void addManager(IManager manager, IManager.LookupModes mode) {
		manager.setLocator(getLocator().clone().add(manager.getName()),getContext());
		manager.setLookupMode(mode);
		managers.put(manager.getName(), manager);
	}

	/**
	 * adds a {@link IManager manager} to this component.
	 * @param manager the manager
	 */
	protected void addManager(IManager manager) {
		addManager(manager,IManager.LookupModes.Name);
	}

	/**
	 * clears all {@link IManager managers} of this component.
	 */
	protected void clearManagers() {
		for (IManager manager : managers.values()) {
			manager.clear();
		}
		managers.clear();
	}

	public IManager getManager(String type) {
		if (type == null) return null;
		return managers.get(type);
	}

	public IManager[] getManagers() {
		return managers.values().toArray(new IManager[managers.values().size()]);
	}

	/**
	 * gets the value of a parameter
	 * @param name the name of the parameter
	 * @param defaultValue the default value returned when the parameter is not set.
	 * @return the parameter value
	 * @throws ConfigurationException
	 */
	protected String getParameter(String name, String defaultValue) throws ConfigurationException{
		//use this method for backwards compatibility reason!!
		return getConfigurator().getParameter(name, defaultValue);
	}

	public ArrayList<IComponent> getDependencies(ArrayList<IComponent>list, boolean recursive) {
		IManager[] managers = getManagers();
		for (IManager manager : managers) {
			IComponent[] components = manager.getAll();
			for (IComponent component : components) {
				if (!list.contains(component)) {
					//log.debug("Resolving Dependency: "+this.getName()+":"+manager.getName()+":"+component.getName()+" of type "+component.getClass().getName());
					list.add(component);
					if (recursive)
						list = component.getDependencies(list,recursive);
				}
			}
		}
		return list;
	}

	public ArrayList<IComponent> getDependencies() {
		return getDependencies(new ArrayList<IComponent>(),true);
	}

	public void setDirty() {
		isDirty = true;
	}

	public boolean isDirty(boolean checkDependencies) {
		if (!isDirty) {
			if (checkDependencies) {
				ArrayList<IComponent> dependencies = getDependencies(new ArrayList<IComponent>(),true);
				for (IComponent dependency: dependencies){
					if (dependency.isDirty(false)) setDirty();
				}
			}
		}
		return isDirty;
	}
	
	protected Map<String,String> getResolvedVariablesAssignment(IComponent component) throws ConfigurationException {
		Map<String,String> variablesAssignment = new HashMap<String,String>();
		for (String s : component.getConfigurator().getResolvedVariables()) {
			String value = component.getConfigurator().getVariables().getProperty(s);
			if (value == null) value = component.getConfigurator().getParameter().getProperty(s);
			if (value == null) throw new ConfigurationException("Variable "+s+" not found in component "+component.getName());
			variablesAssignment.put(s, value);
		}
		return variablesAssignment;
	}
	
	protected String getPersistentSignature() throws ConfigurationException {
		List<IComponent> dependencies = getDependencies();
		Map<String,String> variablesAssignment = getResolvedVariablesAssignment(this);
		for (IComponent c : dependencies) {
			variablesAssignment.putAll(getResolvedVariablesAssignment(c));
		}
		StringBuffer buffer = new StringBuffer();
		for (String s : variablesAssignment.keySet()) {
			String value = variablesAssignment.get(s);
			buffer.append(s+"="+value+";");
		}
		if (buffer.length() > 0) {
			int hashCode = buffer.toString().hashCode();
			long varSignature = Long.valueOf(Integer.MAX_VALUE) + Long.valueOf(hashCode);
			return getName()+"_"+String.valueOf(varSignature);
		} else return getName();
	}
	
	protected IContext getRootExecutionContext() {
		IContext executionRootContext = getContext();
		if (executionRootContext != null) {
			while (!executionRootContext.getBaseContextName().equals(IContext.defaultName)) {
				executionRootContext = ContextManager.getInstance().get(executionRootContext.getBaseContextName());
			}
		}
		return executionRootContext;
	}
	
	protected void init() throws InitializationException {
		try {
			clearManagers();
			getConfigurator().configure();
			setName(getConfigurator().getName());
			setLocator(getConfigurator().getLocator().clone().add(getName()),getConfigurator().getContext());
		} catch (Exception e) {
			throw new InitializationException(e);
		}
		finally { //ensure that name is set, even if something bad happens in configure.
			setName(getConfigurator().getName());
		}
	}
	
	protected void postInit() throws InitializationException {
		try {
			if (isOptimizePersistence()) {
				if (getContext() != null) {//cschw: Contexts and Projects have no contexts! 
					getLocator().setPersistentSchema(getLocator().getRootName()+"_"+getRootExecutionContext().getName());
					getLocator().setPersistentTable(getPersistentSignature());
				}
			}
		} catch (Exception e) {
			throw new InitializationException(e);
		}
		finally {
		}
	}

	public void initialize() throws InitializationException {
		init();
		postInit();
	}
	
	protected boolean isOptimizePersistence() {
		String setting = Settings.getInstance().getContext(Settings.PersistenceCtx).getProperty("optimize", "false");
		try {
			return setting.equalsIgnoreCase("true");
		} catch (Exception e) {
			return false;
		}
	}

	public void test() throws RuntimeException {
		//do nothing here
	}

	public Row getOutputDescription() throws RuntimeException {
		//return an empty output description by default.
		return new Row();
	}
	
	public Element getComponentDescription() throws ConfigurationException {
		Element root = new Element("invalid");
		try {
			root.setName(getLocator().getType());
			root.setAttribute("type", getConfigurator().getXML().getAttributeValue("type"));
			root.setAttribute("name",getName());
			Element description = new Element("description");
			description.setText(getConfigurator().getXML().getChildTextTrim("comment"));
			root.addContent(description);
			
			IProject project =  ((IProject)getContext().getComponent(getLocator().getRootLocator()));
			if(this.getLocator().getType().equals("function")){
			List<IComponent> usesList = this.getDependencies(new ArrayList<IComponent>(), false);
			if (!usesList.isEmpty()) {
				Element dependencies = new Element("uses");
				for (IComponent c : usesList) {
					Element uses = new Element(c.getLocator().getType());
					uses.setText(c.getLocator().getName());
					dependencies.addContent(uses);
				}
				root.addContent(dependencies);
			}
			}else{
		
				List<String> directDeps = project.getDirectDependencies(this.getLocator().toString(), true);
				
				if (!directDeps.isEmpty()) {
					Element dependencies = new Element("uses");
					for (String dep : directDeps) {
						Locator depLoc = Locator.parse(dep);
						Element uses = new Element(depLoc.getType());
						uses.setText(depLoc.getName());
						dependencies.addContent(uses);
					}
					root.addContent(dependencies);
				}
			}
			if (!getLocator().isRoot()) {
				List<String> usedByList =project.getDirectDependents(getLocator().toString(),true);
				if (usedByList != null && !usedByList.isEmpty()) {
					Element dependants = new Element("usedby");
					for (String c : usedByList) {
						Element usedBy = new Element(Locator.parse(c).getType());
						usedBy.setText(Locator.parse(c).getName());
						dependants.addContent(usedBy);
					}
					root.addContent(dependants);
				}
			}
			Element rawXML = new Element("xml");
			rawXML.addContent(getConfigurator().getXML().cloneContent());
			root.addContent(rawXML);
		}
		catch (Exception e) {
			throw new ConfigurationException(e);
		}
		return root;
	}
	
   public int compareTo(IComponent argument) {		   
	  return getName().compareTo(argument.getName());
   }
   
   public ITreeProcessor initTreeProcessor(ITreeProcessor processor, IProcessor.Facets facet) throws RuntimeException {
	   initProcessor(processor,facet);
	   return processor;
   }
   
   public IProcessor initProcessor(IProcessor processor, IProcessor.Facets facet) throws RuntimeException {
	   if (!processor.isInitialized() || processor.getFacet() == null) {
		   processor.setOwner(this);
		   processor.setFacet(facet);
		   if (getContext() != null && !processor.getFacet().equals(Facets.CONNECTION)) {
			   getContext().registerProcessor(processor);
		   }
		   if (!processor.isInitialized()) processor.initialize();
	   } else {
		   if (processor.getFacet().equals(Facets.CONNECTION) && !this.equals(processor.getOwner())) {
			   processor.setOwner(this);
			   processor.setFacet(facet);
			   if (getContext() != null) {
				   getContext().registerProcessor(processor);
			   }
		   } else {
			    return initProcessor(new IdProcessor(processor),facet);
		   }
	   }
	   return processor;
   }

}
