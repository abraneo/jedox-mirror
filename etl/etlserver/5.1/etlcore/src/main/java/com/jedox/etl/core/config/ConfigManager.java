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

package com.jedox.etl.core.config;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;
import org.jdom.Document;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import com.jedox.etl.core.component.EventSender;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.ITypes.Components;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.project.ProjectManager;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;

import java.io.IOException;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Vector;
import java.util.List;
import java.util.HashSet;
import java.util.Iterator;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Manager class for project definitions.
 * Entire {@link com.jedox.etl.core.IProject projects} may be added or removed as well as single {@link com.jedox.etl.core.component.IComponent components} within existing projects.
 * <p>
 * The ConfigManager is responsible for delivering up-to-date components to the application. If any changes in the underlying configuration of a component are made, this component is recreated with the current configuration on the next request.
 * </p>
 * Validation of the configurations is also supported.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ConfigManager extends EventSender {

	private static ConfigManager instance = null;
	private static final Log log = LogFactory.getLog(ConfigManager.class);
	private ConfigPersistor persistor;
	private HashSet<String> dirtySet = new HashSet<String>();
	private ReentrantReadWriteLock lock = new ReentrantReadWriteLock();
	private ProjectManager projectManager;
	private HashSet<String> cycleDetectionSet = new HashSet<String>();
	private boolean writeable = true;
	private HashMap<String,String> sessionToUserMap = new HashMap<String,String>();
	

	private ConfigManager() {
		projectManager = new ProjectManager();
	}

	/**
	 * gets the singleton instance of this ConfigManager
	 * @return the productive instance
	 */
	public synchronized static final ConfigManager getInstance() throws ConfigurationException {
		if (instance == null) {
			instance = new ConfigManager();
			instance.load();
		}
		return instance;
	}
	
	public ConfigManager spawnReadOnlyInstance(Locator rootConfig) {
		ConfigManager contextInstance = new ConfigManager();
		try {
			lockDocument(true);
			IProject projectInDefaultContext = this.projectManager.get(rootConfig.getRootName());
			if (projectInDefaultContext != null) contextInstance.projectManager.add(projectInDefaultContext);
			contextInstance.persistor = new ConfigPersistor(true);
			contextInstance.writeable = false;
			//add read only clone of local project configuration
			if(rootConfig.getManager().equals(NamingUtil.group_manager)){
				Element group = this.persistor.getGroup(rootConfig.getRootName());
				if(group!=null)
					contextInstance.persistor.addGroup(rootConfig.getRootName(), group);
			}else{
				Element project = this.persistor.getProject(rootConfig.getRootName());
				if (project != null) {
					contextInstance.persistor.addProject(rootConfig.getRootName(), (Element)project.clone());
				}
			}
		} catch (Exception e) {
			log.error("Error spawning read only instance of config manager: "+e.getMessage());
		}
		finally {
			unlockDocument(true);
		}
		return contextInstance;
	}
	

	public void copyProject(String source, String target) throws ConfigurationException {
		Element project = get(Locator.parse(source));
		if (project != null) {
			project.setAttribute("name", target);
			add(Locator.parse(target),project);
		}
	}

	private synchronized void lockDocument(boolean readOnly) {
		if (readOnly) {
			lock.readLock().lock();
			//log.info("Read lock.");
		}
		else {
			lock.writeLock().lock();
			//log.info("Write lock.");
		}
	}

	private synchronized void unlockDocument(boolean readOnly) {
		if (readOnly) {
			lock.readLock().unlock();
			//log.info("Release read lock.");
		} else {
			lock.writeLock().unlock();
			//log.info("Release write lock.");
		}
	}


	protected void setDirty(Locator locator) throws ConfigurationException {
		try {
			Locator loc = locator.clone();
			//set project dirty only if project config is changed.
			if (loc.isRoot()) {
				dirtySet.add(loc.toString());
			}
			//set whole tree dirty up to project.
			else while (!loc.isRoot()) {
				if (loc.isComponent())
					dirtySet.add(loc.toString());
				loc.reduce();
			}
			//set project object dirty to force new dependency calculation
			IProject project = getProjectManager().get(locator.getRootName());
			if (project != null) project.invalidate();
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to set locator dirty: "+e.getMessage());
		}
	}
	
	private void save(Locator loc, Element rootConfig, boolean isDelete) throws Exception {
		if (writeable) {
			if (rootConfig!=null) {
				// this temporary ConfigManager will help us use some methods like findElement
				ConfigManager manager = this.spawnReadOnlyInstance(loc);
				if(!loc.getManager().equals(NamingUtil.group_manager)){
					manager.addElement(loc.getRootLocator(), rootConfig);			
					manager.updateChangedInfo(manager.findElement(loc.getRootLocator()),manager.findElement(loc),loc.getSessioncontext(), isDelete);
				}else{
					manager.updateChangedInfo(rootConfig,null,loc.getSessioncontext(),false);
				}
			}	
			if (!loc.getManager().equals(NamingUtil.group_manager)) {
				setDirty(loc);
			}					
			persistor.save(loc,rootConfig);
		} else {
			persistor.updateMaps(loc, rootConfig);
		}
		
	}

	private void setTidy(Locator locator) {
		Iterator<String> i = dirtySet.iterator();
		Vector<String> tidy = new Vector<String>();
		while (i.hasNext()) {
			String s = i.next();
			if (s.startsWith(locator.toString())) tidy.add(s);
		}
		dirtySet.removeAll(tidy);
	}

	private boolean isDirty(Locator locator) {
		return dirtySet.contains(locator.toString());
	}
	

	private Element getByName(Element parent, List<?> list, String name) {
		Element element = null;
		for (int j=0; j<list.size(); j++) {
			Element e = (Element) list.get(j);
			if (e.getAttributeValue("name","default").trim().equals(name)) {
				element = e;
				break;
			}
		}
		return element;
	}

	private String getManagerName(Locator locator) {
		String name = locator.clone().reduce().getName();
		return name;
	}

	private String getComponentName(String manager) {
		return manager.substring(0, manager.length()-1);
	}
	
	public Element findGroup(Locator group) throws ConfigurationException {
		try {	
			return persistor.getGroup(group.getRootName());			
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to find group: "+e.getMessage());
		}
	}
	
	public String[] getGroups(String project,boolean returnConfig) throws ConfigurationException {
		if (!persistor.isValid())
			throw new ConfigurationException("ETL project persistence is corrupt");

		try {
			ArrayList<String> groups = new ArrayList<String>();
			for(Element groupElement: persistor.getGroups().values()){
				boolean toAdd = false;
				if(project==null || project.isEmpty()){
					toAdd=true;
				}else{
					for(Object member: groupElement.getChild("members").getChildren("member")){
						Element memberElement = (Element) member;
						if(project.equals(memberElement.getAttributeValue("name"))){
							toAdd=true;
							break;
						}
					}
				}				
				if(toAdd){
					if(returnConfig)
						groups.add(XMLUtil.jdomToString(groupElement));
					else
						groups.add(groupElement.getAttributeValue("name"));
				}				
			}
			
			return groups.toArray(new String[0]);
			
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to get Group info: "+e.getMessage());
		}
	}


	public Element findElement(Locator locator) throws ConfigurationException {
		try {
			Vector<String> path = locator.getPath();
			List<?> list = null;
			if (path.size() == 0) {
				throw new ConfigurationException("Locator cannot be empty.");
			}	
			Element element = persistor.getProject((locator.getRootName()));
			if (element==null && path.size() > 1) {
				throw new ConfigurationException("Project "+locator.getRootName()+" not found");				
			}
			list = null;
			String component = null;
			String manager = null;
			for (int i=1; i<path.size(); i++) {
				if (i % 2 == 1) {
					manager = path.get(i);
					Element e = element.getChild(manager);
					if (e != null) { //manager exists
						list = e.getChildren(getComponentName(manager));
						element = e;
					}
					else {
						//list = element.getChildren(getComponentName(manager));
						if(!ITypes.MainManagers.contains(manager))
							throw new ConfigurationException("Manager " + manager + " is not a valid manager name, only values " + java.util.Arrays.asList(ITypes.MainManagers.values()) + " are allowed.");
						
						Element managerElement = new Element(manager);
						element.addContent(managerElement);
						element = managerElement;
						list = new ArrayList<Element>();
					}
				}
				else {
					component = path.get(i);
					element = getByName(element, list,component);
					if (element == null) {
						return null;
					}
				}
			}
			return element;
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to find element: "+e.getMessage());
		}
	}

	private void updateChangedInfoElement(Element config, String changedby) {
		config.setAttribute("modified",String.valueOf((new Date()).getTime()));
		config.setAttribute("modifiedBy",changedby);		
	}
	
	
	protected void updateChangedInfo(Element rootConfig, Element componentConfig, String session, boolean rootOnly){
		if(sessionToUserMap.get(session)==null){
			sessionToUserMap.put(session,OLAPAuthenticator.getInstance().getUserInfo(session).getName());
		}
		String changedby = sessionToUserMap.get(session);
		if (!rootOnly) {
			if (componentConfig == null || rootConfig.equals(componentConfig)) { //recurse to all components
				if(rootConfig.getName().equals("project")){
					for (ITypes.Managers m : ITypes.Managers.values()) {
						Element managerElement = rootConfig.getChild(m.toString());
						if (managerElement != null) {
							for (Object component : managerElement.getChildren()) {
								updateChangedInfoElement((Element)component,changedby);
//								updateChangedInfo(rootConfig,(Element)component,session,rootOnly);
							}
						}
					}
				}
			}else if(!componentConfig.getName().equals("function")){
				updateChangedInfoElement(componentConfig,changedby);
				log.debug("Updating component access attributes: "+componentConfig.getName()+":"+componentConfig.getAttributeValue("name"));
			}else{}
		}		
		log.debug("Updating access attributes: "+rootConfig.getName()+":"+rootConfig.getAttributeValue("name"));
		updateChangedInfoElement(rootConfig,changedby);

	}
	
	private void addElement(Locator locator, Element config) throws ConfigurationException {
		try {	
			lockDocument(false);
			// check root tag of config
			if (!config.getName().equals(locator.getType()))
				throw new ConfigurationException("Root element "+config.getName()+" does not match to locator "+locator.getType());			
			Element element = get(locator);			
			Element project=null;
			if (locator.isRoot()) {
				project=config;
			} else {
				Element parent = findElement(locator.getParentLocator());
				Element e = findElement(locator);
				int index = -1;
				if(e!=null){
					index = e.getParentElement().indexOf(e);
					parent.removeContent(e);
				}
				if (parent != null) {
					config.detach();
					if(index==-1)
						parent.addContent(config);
					else 
						parent.addContent(index, config);
					project = findElement(locator.getRootLocator());					
				}
				else
					throw new ConfigurationException("Failed to locate config for: "+locator.getParentLocator().toString());				
			}
			save(locator,project,false);			
			fireConfigChanged(locator,element,config);
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to add element: "+e.getMessage());
		}
		finally {
			unlockDocument(false);
		}
	}


	private String getVersion(Locator locator, Element config) throws ConfigurationException {
		if (locator.isRoot())
			return config.getAttributeValue("version","1.0");
		else
			return findElement(locator.getRootLocator()).getAttributeValue("version", "1.0");
	}

	private IProject.Declaration getFormat(Locator locator, Element config) {
		try {
			if (locator.isRoot())
				return IProject.Declaration.valueOf(config.getAttributeValue("format",IProject.Declaration.lazy.toString()).toLowerCase());
			else
				return IProject.Declaration.valueOf(findElement(locator.getRootLocator()).getAttributeValue("format", IProject.Declaration.lazy.toString()).toLowerCase());
		}
		catch (Exception e) {
			return IProject.Declaration.lazy;
		}
	}

	/**
	 * Adds a configuration entity to this manager at the address denoted by the locator.
	 * @param locator the address in project space, where the new configuration element should be added.
	 * @param config the configuration element
	 * @return the old configuration element at the address of the locator or null, if there was no configuration at this address
	 */
	public synchronized Element add(Locator locator, Element config) throws ConfigurationException {
		if (!persistor.isValid())
			throw new ConfigurationException("ETL project persistence is corrupt and therefor no components can be added.");
		
		ConfigConverter converter = new ConfigConverter();		
		Element element = findElement(locator);
		addElement(locator, converter.convert(config, getVersion(locator,config), getFormat(locator, config)));
		return element;
	}
	
	/**
	 * Adds a group.
	 * @param group group name.
	 * @param config the configuration element
	 * @return the old configuration element at the address of the locator or null, if there was no configuration at this address
	 */
	public synchronized Element addGroup(Locator group, Element config) throws ConfigurationException {
		try {	
			if (!persistor.isValid())
				throw new ConfigurationException("ETL project persistence is corrupt and therefor no groups can be added.");
			lockDocument(false);
			Element element = findGroup(group);
			save(group,config,false);
			return element;
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to add group: "+e.getMessage());
		}
		finally {
			unlockDocument(false);
		}		
	}

	/**
	 * Removes the configuration entity at the address denoted by the locator.
	 * @param locator the address in project space, where the configuration should be removed.
	 * @return the old configuration element at the address of the locator or null, if there was no configuration at this address.
	 */
	public synchronized Element removeElement(Locator locator) throws ConfigurationException {
		try {
			lockDocument(false);			
			Element element = findElement(locator);
//				if ((element != null) && (!element.equals(getDocument().getRootElement()))) {
			if (element != null) {
				Element project=null;				
				if (!locator.isRoot()) {
					element.getParentElement().removeContent(element);					
					project = findElement(locator.getRootLocator());
				}	
				save(locator,project,true);				
				fireConfigChanged(locator,element,null);
				return element;
			}
			return null;
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to remove element: "+e.getMessage());
		}
		finally {
			unlockDocument(false);
		}		
	}
	
	/**
	 * Removes group
	 * @param group group name
	 * @return the old configuration element at the address of the locator or null, if there was no configuration at this address.
	 */
	public synchronized Element removeGroup(Locator group) throws ConfigurationException {
		try {
			lockDocument(false);
			Element element = findGroup(group);
			if (element != null) {
				element.getParentElement().removeContent(element);
				save(group, null, true);
				return element;
			}else{
				throw new ConfigurationException("Failed to remove group "+group.getRootName() + ": the group does not exist."  );
			}
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to remove group: "+e.getMessage());
		}
		finally {
			unlockDocument(false);
		}
	}
	
	private void renameUpdateDependant(Element projectConfig, IProject project, Locator loc, String dependent, String newName) throws ConfigurationException {
		Locator depLoc = Locator.parse(dependent);
		Element component = findElement(depLoc);
		Element componentOld = (Element) component.clone();
		UpdateReferencesConfigUtil.getInstance().renameReference(component, loc, depLoc, newName);
		try {
			if(!XMLUtil.compare(component, componentOld))
				updateChangedInfo(projectConfig, component, loc.getSessioncontext(),false);
		} catch (IOException e) {
			throw new ConfigurationException("Error while comparing the new/old versions: " + e.getMessage());
		}
		setDirty(depLoc);
	}

	public synchronized int renameComponent(Locator loc, String newName,boolean updateReferences) throws ConfigurationException {
		int dependentCount=0;
		Locator originalLoc = loc.clone();
		/*
		 * backup old project configuration, because renameComponent (unlike the other 
		 * methods) changes the in-memory variant directly a backup should be kept to
		 * restore if the rename fails e.g. no OLAP connection
		 */
		Element projectConfig = findElement(loc.getRootLocator());
		Element backUpConfig = (Element) projectConfig.clone();
		
		try {
			ConfigManager.getInstance().lockDocument(false);
			Element element = ConfigManager.getInstance().findElement(loc);
			if (loc.isRoot()) {
				loc.setRootName(newName);
			} else {
				if (updateReferences) {
					//if the component is not a project, the dependencies should be changes 
					// CAUTION: the dependencies will be changed directly in memory. 
					// So no authentication error should come afterward
					IProject project = (IProject) getComponent(loc.getRootLocator(), IContext.defaultName);				
					if (loc.getManager().equals(ITypes.Managers.variables.toString())) {//variables are used in jobs
						ArrayList<Locator> locList = new ArrayList<Locator>();
						locList.add(loc);
						HashMap<String, String> map = new HashMap<String, String>();
						map.put(loc.getName(), newName);
						new VariablesDependencyRename(locList).rename(map);	
					} else if (loc.getManager().equals(ITypes.Managers.functions.toString())) { //functions
						Locator transformLoc = loc.clone().reduce().reduce();
						renameUpdateDependant(projectConfig, project, loc, transformLoc.toString(), newName); //fix only enclosing transform
						dependentCount = 1;
					/*boolean hasTarget = findElement(transformLoc).getChild("target") != null;
					if (!hasTarget) { //fix dependents of enclosing transform
						List<String> dependents = project.getDirectDependents(transformLoc.toString(),false);
						dependentCount+=dependents.size();
						for(String dependent : dependents){
							renameUpdateDependant(projectConfig, project, loc, dependent,newName);
						}
					}*/
					} else {//get dependents for components
						//for connections,extracts,transforms,loads,jobs	
						List<String> dependents = project.getDirectDependents(loc.toString(),false);
						dependentCount=dependents.size();
						for(String dependent : dependents){
							renameUpdateDependant(projectConfig, project, loc, dependent, newName);
						}
					}
				}	
				loc.reduce();
				loc.add(newName);
			}			
			element.setAttribute("name", newName);
			loc.setOrigin(originalLoc);
			add(loc, element);
			setDirty(loc);
			//removeElement(originalLoc);
		}catch(Exception e){
			ConfigPersistor.getInstance().updateMaps(originalLoc.getRootLocator(), backUpConfig);
			throw new ConfigurationException(e.getMessage());
		}
		finally {
			try {
				ConfigManager.getInstance().unlockDocument(false);
			} catch (ConfigurationException e) {}
		}
		return dependentCount;
	}
	
	
	/**
	 * tries to get the configuration denoted by the locator and its parent locators and raises Exception if not found
	 * @param locator the address in project space, where the configuration should be fetched from
	 * @throws ConfigurationException Readable Exception for the component which hasn't been found
	 */
	public void checkComponent(Locator locator) throws ConfigurationException {
		if (!locator.isRoot())
			checkComponent(locator.getParentLocator());
		if (locator.isComponent() && get(locator)==null)
			throw new ConfigurationException(locator.getDisplayName() + " not found.");
	}	
	
	/**
	 * gets the configuration entity denoted by the locator.
	 * @param locator the address in project space, where the configuration should be fetched from
	 * @return the configuration entity or null, if there is no configuration at this address.
	 */
	public Element get(Locator locator) throws ConfigurationException {
		Element result;
		result = findElement(locator);
		if (result != null) {
			result = (Element) result.clone();
		}
		return result;
	}

	/**
	 * gets the configuration entity denoted by the locator in a pretty printed human readable string representation.
	 * @param locator the address in project space, where the configuration should be fetched from
	 * @return the configuration entity in string representation.
	 */
	public String getConfigurationString(Locator locator) throws ConfigurationException {
		StringWriter writer = new StringWriter();
		try {
			Document document = new Document();
			Element root = get(locator);
			if (root != null) {
				if (locator.isRoot()) //remove format tag of projects
					root.removeAttribute("format");
				document.setRootElement(root);
				XMLOutputter outputter = new XMLOutputter(Format.getPrettyFormat());
				outputter.output(document, writer);
			}
			else
				throw new ConfigurationException("Failed to get Component configuration: Component "+locator.toString()+" not found.");
		} catch (Exception e) {
			throw new ConfigurationException(e);
		}
		return writer.toString();
	}

	/**
	 * gets a specific {@link com.jedox.etl.core.context.IContext context} from an existing project
	 * @param project the name of the project
	 * @param context the name of the context within the project
	 * @return the context object or the default context if no such context is found in the project
	 * @throws ConfigurationException, if configuration is faulty
	 */
	public synchronized IContext getContext(String project, String context) throws ConfigurationException {
		try {
			//CSCHW projects manage their default contexts only. All runtime execution contexts (having unique ids) are managed by global context-manager, since projects may be updated during executions.
			IContext c = (context == null || IContext.defaultName.equals(context)) ? (IContext)getProject(project).getManager(ITypes.Contexts).get(context) : ContextManager.getInstance().get(context);
			if (c == null) {
				log.warn("Context "+context+" not found. Using default context instead.");
				c = (IContext)getProjectManager().get(project).getManager(ITypes.Contexts).get(IContext.defaultName);
			}
			return (IContext) c;
		}
		catch (Exception e) {
			throw new ConfigurationException(e);
		}
	}

	/**
	 * gets a {@link com.jedox.etl.core.project.IProject project} by name. The sub-components of the project are not initialized.
	 * @param project the name of the project
	 * @return the project object
	 * @throws ConfigurationException, if there is no such project
	 */
	public synchronized IProject getProject(String project) throws ConfigurationException {
		try {
			IComponent component = getProjectManager().get(project);
			Locator pl = Locator.parse(project);
			if ((component == null)  || isDirty(pl)) {
				Element element = get(pl);
				if (element != null) {
					component = getProjectManager().add(element);
					setTidy(pl);
				} else
					throw new ConfigurationException("Project "+project+" not found.");
			}
			return (IProject) component;
		}
		catch (Exception e) {
			throw new ConfigurationException(e);
		}
	}

	private IComponent createComponent(Locator locator, IContext context, String managerName) throws ConfigurationException {
		String uname = locator.toString()+"@"+context.getName();
		if (!cycleDetectionSet.contains(uname)) {
			try {
				IComponent component = null;
				if (managerName.equals(ITypes.Sources) && locator.isBasicComponent()) { //dispatch generic source request to supported components
					IComponent result = createComponent(locator.clone().setManager(ITypes.Extracts),context,ITypes.Extracts);
					if (result == null)
						result = createComponent(locator.clone().setManager(ITypes.Transforms),context,ITypes.Transforms);
					return result;
				}
				Element element = get(locator);
				if (element != null) { //recreate from config
					IManager parentManager = context.getManager(managerName);
					//reset manager locator for embedded components.
					Locator loc = locator.clone().reduce();
					loc.setPersistentSchema(null);
					loc.setPersistentTable(null);
					parentManager.setLocator(loc, context);
					cycleDetectionSet.add(uname);
					component = parentManager.add(element);
					setTidy(locator);
				}
				return component;
			}
			catch (Exception e) {
				// Raise message with component type/name but not repeated for dependent components
				// Type/name not required if error occurred in validation, class ConfigValidator method validate
				if (e.getMessage()!=null && (e.getMessage().startsWith("Configuration Error in") || e.getMessage().startsWith("Failed to validate")))
					throw new ConfigurationException(e.getMessage());
				else
					throw new ConfigurationException("Configuration Error in "+locator.getDisplayName()+": "+e.getMessage());
			}
			finally {
				cycleDetectionSet.remove(uname);
			}
		}
		else throw new ConfigurationException("Cyclic Dependency detected for "+locator.getDisplayName());
	}

	public synchronized List<Locator> initProjectComponents(IProject project, String context, boolean ignoreInvalidComponents) throws ConfigurationException {
		//check all components and their dependencies
		List<Locator> invalidComponents = new ArrayList<Locator>();
		Locator loc = project.getLocator();
		//set context of project when used as component (e.g. for testing)
		project.setLocator(loc, getContext(project.getName(),context));
		Element e = findElement(loc);
		List<?> managers = e.getChildren();
		for (int i=0; i<managers.size(); i++) {
			Element m = (Element) managers.get(i);
			List<?> components = m.getChildren();
			for (int j=0; j<components.size(); j++) {
				Element c = (Element) components.get(j);
				Locator subloc = null;
				try {
					//test if element specifies a component
					Components.valueOf(c.getName());
					//instantiate this component
					subloc = loc.clone().add(m.getName()).add(c.getAttributeValue("name"));
					getComponent(subloc,context, ignoreInvalidComponents);
				}
				catch (IllegalArgumentException ex) {}
				catch (ConfigurationException ce) {
					if (ignoreInvalidComponents) {
						invalidComponents.add(subloc);
					} else throw ce;
				}
			}
		}
		return invalidComponents;
	}


	public synchronized IComponent getComponent(Locator locator, String context) throws ConfigurationException {
		return getComponent(locator, context, false);
	}	
	
	/**
	 * gets / creates a {@link com.jedox.etl.core.component.IComponent component} by its address in project space. If a non-dirty component already exists, this component is returned. Else a new component matching the config at the locator address is created. All dependent sub-components are also created.
	 * @param locator the address of the component in project space
	 * @param context the context to get / create the component in
	 * @param ignoreInvalidComponents if set configuration errors are ignored
	 * @return the component at this address.
	 * @throws ConfigurationException, if no component can be found and created.
	 */
	public synchronized IComponent getComponent(Locator locator, String context, boolean ignoreInvalidComponents) throws ConfigurationException {
		if (locator == null) {
			throw new ConfigurationException("Cannot instatiate component from null-locator.");
		}
		String project = locator.getRootName();
		IComponent component = null;
		//special case project
		if (locator.isRoot()) {
			IProject p = getProject(project);
			log.debug("Initializing project in context "+context);
			if (p.getModelComponents().isEmpty()) initProjectComponents(p,context,ignoreInvalidComponents);
			return p;
		}
		String managerName = getManagerName(locator);
		IContext c = getContext(project,context);
		IManager manager = c.getManager(managerName);
		if (manager == null) {
			throw new ConfigurationException("Manager name '"+managerName+"' not valid in locator "+locator.toString());
		}
		component = manager.get(locator.toString());
		if ((component == null) || (isDirty(locator) || (component.isDirty(true)))) { // it does not exist or is dirty
				component = createComponent(locator,c,managerName);
		}
		if (component == null) {
			throw new ConfigurationException(locator.getType() + " "+ locator.getName() + " not found.");
		}
		return component;
	}

	private Vector<String> getComponentNames(IComponent[] components) {
		Vector<String> names = new Vector<String>();
		for (IComponent c : components) {
			names.add(c.getName());
		}
		return names;
	}

	private Vector<String> getComponentNames(Locator locator) throws ConfigurationException {
		if (!persistor.isValid())
			throw new ConfigurationException("ETL project persistence is corrupt");
		
		Vector<String> names = new Vector<String>();
		if (locator.isEmpty()) { //get available projects from config space			
			for (String projectName : persistor.getProjects().keySet()) {
				names.add(projectName);
			}
/*			
			Element element = get(locator);
			if (element != null) {
				List<?> list = element.getChildren(getComponentName(element.getName()));
				for (int i=0; i<list.size(); i++) {
					String name = ((Element)list.get(i)).getAttributeValue("name","default");
					names.add(name);
				}
*/								
			
		} else {
			//ensure that all components are initialized by forcing doing a project validation.
			//validate(locator.getRootLocator());
			IComponent component = getComponent(locator.clone().reduce(),IContext.defaultName, true);
			IManager manager = null;
			if (component instanceof IProject) { //get manager from context default
				manager = component.getManager(ITypes.Contexts).get(IContext.defaultName).getManager(locator.getName());
			}
			else //directly get manager
				manager = component.getManager(locator.getName());
			if (manager != null) {
				names.addAll(getComponentNames(manager.getAll()));
			} else
				throw new ConfigurationException(locator.getType() + " "+ locator.getName() + " not found.");
		}
		return names;
	}


	private Vector<String> getManagerNames(IManager[] managers) {
		Vector<String> names = new Vector<String>();
		for (IManager m : managers) {
			if (m.getName().equals(ITypes.Contexts) && managers.length == 1)
				return getManagerNames(m.get(IContext.defaultName).getManagers());
			if (!m.isEmpty())
				names.add(m.getName());
		}
		return names;
	}

	private Vector<String> getManagerNames(Locator locator) throws ConfigurationException {
		IManager[] managers = null;
		IComponent component = null;
		try {
			component = getComponent(locator,IContext.defaultName, true);
			managers = component.getManagers();
		}
		catch (ConfigurationException e) { //may be a view. get by enclosing component manager.
			return new Vector<String>();
		}
		return getManagerNames(managers);
	}

	/**
	 * gets the names of all children hosted by the component or manager denoted by the locator
	 * @param locator the address of a component or manager: e.g: "myProject.connections", "myProject.connections.myConnection". If null the root project manager is selected.
	 * @return an array of names
	 */
	public List<String> getNames(Locator locator) throws ConfigurationException {
		ArrayList<String> names = new ArrayList<String>();
		if (locator.isManager()) {
			//components of managers required:
			names.addAll(getComponentNames(locator));
		}
		else {
			//managers of component required:
			names.addAll(getManagerNames(locator));
		}
	    Collections.sort(names);
//		return names.toArray(new String[names.size()]);
	    return names;
	}

	/**
	 * gets the locators of all children hosted by the component or manager denoted by the locator
	 * @param locator the path to a component or manager: e.g: "myProject.connections", "myProject.connections.myConnection". If null the root project manager is selected.
	 * @return an array of locators
	 */
	public Locator[] getLocators(Locator locator) throws ConfigurationException {
		List<String> names = getNames(locator);
		Locator[] locators = new Locator[names.size()];
		for (int i=0; i<names.size(); i++) {
			locators[i] = locator.clone().add(names.get(i));
		}
		return locators;
	}

	/**
	 * invalidates all internally cached objects, which forces their recreation on their next request.
	 */
	public void invalidate() {
		//do a simple refresh strategy. Remove all existing components, which forces them to be re-created on next demand.
		getProjectManager().clear();
	}

	/**
	 * validates a configuration with respect to its xsd and instantiates all of its managers and components to check the dynamic references.
	 * @param locator the address of of the configuration in project space.
	 * @throws ConfigurationException, if validation fails.
	 */
	public synchronized void validate(Locator locator) throws ConfigurationException {
		Locator loc = locator.clone();
		if (loc.isManager())
			loc.reduce();
		if (!loc.isEmpty())
			getComponent(loc, IContext.defaultName);
	}

	protected void load() throws ConfigurationException {
		try {
			lockDocument(false);
			persistor = ConfigPersistor.getInstance();			
			persistor.load();
			persistor.setValid(true);
		}
		catch (Exception e) {
			persistor.setValid(false);
			throw new ConfigurationException("Failed to load repository: "+e.getMessage());
		}
		finally {
			unlockDocument(false);
		}
	}	

	public ProjectManager getProjectManager() {
		return projectManager;
	}

}
