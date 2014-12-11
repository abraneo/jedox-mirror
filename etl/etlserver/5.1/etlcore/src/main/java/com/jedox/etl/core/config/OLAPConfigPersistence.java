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

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.tree.Consolidation;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.palojlib.premium.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICube.SplashMode;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.CellLoadContext;
import com.jedox.palojlib.premium.main.ConnectionManager;

/**
 * @author khaddadin
 *
 */
public class OLAPConfigPersistence implements IConfigPersistence {

	private static final Log log = LogFactory.getLog(OLAPConfigPersistence.class);
	private boolean needMigration=false;
	

	/* loads the project from OLAP.
	 * @see com.jedox.etl.core.config.AbstractConfigPersistence#load()
	 */

	@Override
	public void load() throws Exception {

		IConnection palosuiteConnection = null;
		try{
			IConnectionConfiguration connectionConfiguration = Settings.getInstance().getSuiteConnectionConfiguration();
			palosuiteConnection = ConnectionManager.getInstance().getConnection(connectionConfiguration);
			palosuiteConnection.openInternal();
			log.debug("Connection to Jedox OLAP Server on "+connectionConfiguration.getHost()+" on port "+connectionConfiguration.getPort()+" is open.");

			IDatabase configdb = palosuiteConnection.getDatabaseByName("Config");
			if (configdb == null)
				throw new RuntimeException("Database Config not found.");

			//boolean isEmpty=false;

			IDimension configDim = configdb.getDimensionByName("config");		
			if(configDim ==null)
				throw new RuntimeException("Dimension config in Database Config is not found.");
			
			IDimension etlsDim = configdb.getDimensionByName("etls");		
			/*if(etlsDim ==null){
				etlsDim = configdb.addDimension("etls");
				needMigration=true;
				isEmpty=true;
			}*/

			if(configDim.getElementByName("etls_seq",false) ==null){
				//configDim.addBaseElement("etls_seq", ElementType.ELEMENT_STRING);
				//isEmpty=true;
			}

			IDimension etlpropsDim = configdb.getDimensionByName("etlprops");
			/*if(etlpropsDim ==null){
				etlpropsDim = configdb.addDimension("etlprops");
				etlpropsDim.addElements(new String[]{"name","definition"},new ElementType[]{ElementType.ELEMENT_STRING,ElementType.ELEMENT_STRING});
				isEmpty=true;
			}*/

			ICube etlsCube = configdb.getCubeByName("etls");
			/*if(etlsCube == null){
				etlsCube = configdb.addCube("etls", new IDimension[]{etlsDim,etlpropsDim});
				isEmpty=true;
			}*/

			/*if(isEmpty==true){
				log.info("Creating needed olap cube and dimensions.");
				return;
			}*/

			if(etlsDim.getElements(true).length==0){
				needMigration = true;
			}
			
			log.debug("Loading project repository from OLAP server.");
			IElement[] etlprojects = etlsDim.getElements(false);
			IElement definition = etlpropsDim.getElementByName("definition", false);
			IElement name = etlpropsDim.getElementByName("name", false);
			ConfigConverter converter = new ConfigConverter();
			
			for(IElement etlproject:etlprojects){
				String projectName = etlsCube.getCell(new IElement[]{etlproject,name}).getValue().toString();
				Element projectElement = XMLUtil.stringTojdom(etlsCube.getCell(new IElement[]{etlproject,definition}).getValue().toString());
				if (etlproject.getName().startsWith(NamingUtil.group_prefix))
					ConfigPersistor.getInstance().addGroup(projectName, projectElement);
				else if (etlproject.getName().startsWith(NamingUtil.project_prefix)) {
					converter.convert(projectElement);					
					ConfigPersistor.getInstance().addProject(projectName, projectElement);
				}	
				else
					throw new RuntimeException("Wrong project prefix in "+etlproject.getName());
			}
			return;

		}catch(Exception e){
			throw new RuntimeException("Error while reading ETL repository: " + e.getMessage());
		}finally{
			if(palosuiteConnection!= null && palosuiteConnection.isConnected())
				palosuiteConnection.close();
		}
	}



	@Override
	public synchronized void  save(Locator locator, Element config) throws Exception,IOException {
		try{
			Locator loc = (locator.getOrigin()==null?locator:locator.getOrigin());
			String locName = loc.getRootName();
			com.jedox.palojlib.interfaces.IConnection conn = OLAPAuthenticator.getInstance().getSessionConnection(loc.getSessioncontext());
			IDatabase configDb = conn.getDatabaseByName("Config");
			IDimension configDim = configDb.getDimensionByName("config");
			IDimension etlpropsDim = configDb.getDimensionByName("etlprops");
			IDimension etlsDim = configDb.getDimensionByName("etls");
			IElement[] etlprojects = etlsDim.getElements(false);
			ICube etlprojectsCube = configDb.getCubeByName("etls");
			IElement name = etlpropsDim.getElementByName("name", false);

			CellLoadContext loadContext = new CellLoadContext(SplashMode.SPLASH_DEFAULT, 100, false, true);
			String prefix = loc.getManager().equals(NamingUtil.group_manager) ? NamingUtil.group_prefix : NamingUtil.project_prefix;

			if(config==null){
				IElement element = findElement(etlprojectsCube, locName, name, etlprojects, prefix);
				if(element!=null) {
					etlsDim.removeElements(new IElement[]{element});
				}
			}
			else {
				addETLNode(config, etlprojectsCube, etlpropsDim,configDim, etlsDim ,loadContext, locName,prefix);
			}

			if (!prefix.equals(NamingUtil.group_prefix)) {
				String changedGroup = ConfigPersistor.getInstance().updateProjectInGroup(loc,(config==null?null:config.getAttributeValue("name")));
				//update the group information in olap
				if (changedGroup!=null) {				
					updateGroup(changedGroup);
				}	
			}
			
			etlprojectsCube.save();
			
			//update Maps
			ConfigPersistor.getInstance().updateMaps(loc,config);			
			
		}catch(Exception e){
			throw new RuntimeException(e.getMessage());
		}
	}

	private void updateGroup(String groupName) throws ConfigurationException, RuntimeException {		
		//update the group information in olap using the Suite ConnectionSession
		IConnectionConfiguration connectionConfiguration = Settings.getInstance().getSuiteConnectionConfiguration();
		IConnection palosuiteConnection = null;
		try {
			palosuiteConnection = ConnectionManager.getInstance().getConnection(connectionConfiguration);
			palosuiteConnection.openInternal();
			log.debug("Connection to Jedox OLAP Server on "+connectionConfiguration.getHost()+" on port "+connectionConfiguration.getPort()+" is open.");
			IDatabase configDb = palosuiteConnection.getDatabaseByName("Config");
			IDimension configDim = configDb.getDimensionByName("config");
			IDimension etlpropsDim = configDb.getDimensionByName("etlprops");
			IDimension etlsDim = configDb.getDimensionByName("etls");
			ICube etlprojectsCube = configDb.getCubeByName("etls");
			CellLoadContext loadContext = new CellLoadContext(SplashMode.SPLASH_DEFAULT, 100, false, true);
			addETLNode(ConfigPersistor.getInstance().getGroup(groupName), etlprojectsCube, etlpropsDim, configDim, etlsDim, loadContext, groupName, NamingUtil.group_prefix);
		}catch(Exception e){
			throw new RuntimeException(e.getMessage());
		}finally{
			if(palosuiteConnection!= null && palosuiteConnection.isConnected())
				palosuiteConnection.close();
		} 		
	}



	/**
	 * @param project
	 * @param etlprojectsCube
	 * @param name
	 * @param changeto
	 * @param definition
	 * @param changeby
	 * @param loadContext
	 * @param elementName
	 * @throws IOException
	 */
	private IElement addETLNode(Element project, ICube etlprojectsCube,
			IDimension etlpropsDim,IDimension configDim,IDimension etlsDim ,CellLoadContext loadContext, String projectName, String prefix)
	throws IOException {

		IElement definition = etlpropsDim.getElementByName("definition", false);
		IElement name = etlpropsDim.getElementByName("name", false);
		IElement[] projectsList = etlsDim.getElements(false);

		IElement projElement = findElement(etlprojectsCube, projectName, name, projectsList, prefix);

		if(projElement==null)
			projElement = getNewElement(projectName, configDim, etlsDim,prefix);

		IElement[][] paths ={{projElement,name},
				{projElement,definition}};
		Object[] values = {project.getAttributeValue("name"),XMLUtil.jdomToString(project)};
		etlprojectsCube.loadCells(paths, values, loadContext,null);
		
		//update the consolidations for the group
		if(prefix.equals(NamingUtil.group_prefix)){
			List<?> members = project.getChild("members").getChildren();
			if(members.size()==0){
				etlsDim.removeConsolidations(new IElement[]{projElement});
			}else{
				ArrayList<IConsolidation> cons = new ArrayList<IConsolidation>();
				for(Object e:members){
					cons.add(new Consolidation(projElement, findElement(etlprojectsCube, ((Element)e).getAttributeValue("name"), name, projectsList, NamingUtil.project_prefix), 1));
				}
				etlsDim.updateConsolidations(cons.toArray(new IConsolidation[0]));
			}
		}
		
		return projElement;
	}



	/**
	 * @param etlprojectsCube
	 * @param projectName
	 * @param name
	 * @param projectsList
	 * @param projElement
	 * @return
	 */
	private IElement findElement(ICube etlprojectsCube, String projectName,
			IElement name, IElement[] projectsList, String prefix) {
		for(IElement etlproject:projectsList){
			if(etlproject.getName().startsWith(prefix)){
				if(etlprojectsCube.getCell(new IElement[]{etlproject,name}).getValue().toString().equals(projectName)){
					return etlproject;
				}
			}
		}
		return null;
	}

	private IElement getNewElement(String project,IDimension configDim, IDimension etls,String prefix){

		// element does not exist and there for has to be added
		int index = 0;
		IElement etl_seq = configDim.getElementByName("etls_seq", true);
		Object v = etl_seq.getAttributeValue("value");
		try{
			index = Integer.parseInt(v.toString());
		}catch(Exception e){}

		String newName = prefix+index;
		configDim.addAttributeValues(configDim.getAttributeByName("value"), new IElement[]{etl_seq}, new Object[]{++index});
		return  etls.addBaseElement(newName, ElementType.ELEMENT_STRING);

	}

	public boolean needMigration() {
		return needMigration;
	}
	
	public void migrate (IConfigPersistence oldPersistence) throws Exception {
		log.info("Projects will be copied to the olap persistence");
		oldPersistence.load();
		String[] oldprojects = ConfigPersistor.getInstance().getProjects().keySet().toArray(new String[0]);
		if(oldprojects.length>0){
			IConnectionConfiguration connectionConfiguration = Settings.getInstance().getSuiteConnectionConfiguration();
			com.jedox.palojlib.interfaces.IConnection suiteConnection = null;
			try {
				suiteConnection = com.jedox.palojlib.main.ConnectionManager.getInstance().getConnection(connectionConfiguration);
				String session = suiteConnection.open();
				for(String project : oldprojects){
					Locator loc = Locator.parse(project);
					loc.setSessioncontext(session);					
					Element projectElement = ConfigPersistor.getInstance().getProject(project);
					ConfigManager.getInstance().updateChangedInfo(projectElement, null,session,false);
					save(loc, projectElement);
				}	
			}finally{
				if(suiteConnection!=null && suiteConnection.isConnected())
					suiteConnection.close();
			}
		}
	}	

}
