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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 *   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.config;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.util.NamingUtil;

/**
 * @author afroehlich
 *
 */
public class ConfigPersistor {

	private static ConfigPersistor instance;	
	private IConfigPersistence persistence;	
	private HashMap<String,Element> projects = new HashMap<String,Element>();
	private HashMap<String,Element> groups = new HashMap<String,Element>();
	private boolean isValid = true;
	
	private static final Log log = LogFactory.getLog(ConfigPersistor.class);

	
	protected ConfigPersistor(boolean readOnly) {
		if (!readOnly) {
			if(Settings.getInstance().getProjectsPersistence().equalsIgnoreCase("file"))
				persistence = new FileConfigPersistence();
			else{
				persistence = new OLAPConfigPersistence();
			}		
		} else {
			persistence = new MockConfigPersistence();
		}
	}
		
	/**
	 * gets the instance of this persistence backend.
	 * @return this persistence backend
	 */
	public synchronized static ConfigPersistor getInstance() {
		if (instance == null) {
			instance = new ConfigPersistor(false);
		}
		return instance;
	}	
	
	protected void addProject(String name,Element config){
		projects.put(name,config);
	}
	
	protected void removeProject(String name){
		projects.remove(name);
	}
	
	protected Element getProject(String name){
		return projects.get(name);
	}
	
	protected HashMap<String,Element> getProjects(){
		return projects;
	}

	protected void addGroup(String name,Element config){
		groups.put(name,config);
	}
	
	protected void removeGroup(String name){
		groups.remove(name);
	}
	
	protected Element getGroup(String name){
		return groups.get(name);
	}
	
	protected HashMap<String,Element> getGroups(){
		return groups;
	}	
	
	protected void updateMaps(Locator loc, Element project){
		if(loc.getManager().equals(NamingUtil.group_manager)) {
			if(project==null) {
				removeGroup(loc.getRootName());
			} else {
				if(project.getAttributeValue("name")!=loc.getRootName())
					removeGroup(loc.getRootName());
				addGroup(project.getAttributeValue("name"), project);
			}
		} 
		else {
			if(project==null) {
				removeProject(loc.getRootName());
			} else {
				if(loc.getOrigin()!=null)
					removeProject(loc.getOrigin().getRootName());
				
				removeProject(loc.getRootName());
				addProject(project.getAttributeValue("name"), project);
			}
		}	
	}
	
	@SuppressWarnings("unchecked")
	protected String updateProjectInGroup(Locator newNameLocator,String newName) throws ConfigurationException {
		Locator loc = (newNameLocator.getOrigin()==null?newNameLocator:newNameLocator.getOrigin());
		if(loc.isRoot() && (newName==null || !loc.getName().equals(newName))){
			for(Element group : getGroups().values()){
				List<Element> memberslist = new ArrayList<Element>();
				memberslist.addAll(group.getChild("members").getChildren())	;
				for(Element member:memberslist){
					if(member.getAttributeValue("name").equals(loc.getRootName())){
						if(newName==null){
							member.detach();
							group.getChild("members").removeContent(member);
						}else{
							member.setAttribute("name", newName);
						}
						ConfigManager.getInstance().updateChangedInfo(group,null,loc.getSessioncontext(),false);
						String groupName=group.getAttributeValue("name");
						addGroup(groupName, group);
						return groupName;
					}
				}
			}
		}
		return null;
	}
	
	@SuppressWarnings("unchecked")
	public String getProjectGroup(Locator loc){

			for(Element group : getGroups().values()){
				List<Element> memberslist = new ArrayList<Element>();
				memberslist.addAll(group.getChild("members").getChildren())	;
				for(Element member:memberslist){
					if(member.getAttributeValue("name").equals(loc.getRootName())){
						
						return group.getAttributeValue("name");
						
					}
				}
			}
		
		return null;
	}
	
	protected void load() throws Exception {
		persistence.load();
		if (persistence.needMigration()) {
			// load the projects from file if exists				
			persistence.migrate(FileConfigPersistence.getInstance());
			FileConfigPersistence.getInstance().switchToFallBack();
		}		
	}
	
	protected void save(Locator loc, Element project) throws Exception {
		if (Settings.getInstance().getAutoSave()) {
			persistence.save(loc,project);
			if (persistence instanceof OLAPConfigPersistence) {
				// Save projects also as File for Backup as temporary solution		
				try {
					((FileConfigPersistence)FileConfigPersistence.getInstance()).backupToFile();
				} catch (Exception e) {
					log.error("Error in Backup to File :"+e.getMessage());
				}
			}
		}	
		else { // for Standalone client
			updateMaps(loc, project);			
		}
	}
	
	public void setValid(boolean isValid) {
		this.isValid = isValid;
	}
	
	public boolean isValid() {
		return isValid;
	}
	
	
}
