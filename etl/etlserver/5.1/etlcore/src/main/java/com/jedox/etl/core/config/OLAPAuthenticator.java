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
 */
package com.jedox.etl.core.config;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Properties;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementPermission;
import com.jedox.palojlib.interfaces.IUserInfo.Predefined_OLAP_Right_Object_Type;
import com.jedox.palojlib.main.SessionConfiguration;
import com.jedox.palojlib.main.UserInfo;

/**
 * @author khaddadin
 *
 */

public class OLAPAuthenticator {
	
	public enum ETL_Element_Types{
		G,P
	}
		
	private static OLAPAuthenticator authenticator = null;
	private HashMap<String, IDatabase> configs = null;
	private static boolean active = true;
	
	// the sequence should be kept the same 
	// if new right object comes up then it should be added to the end
	private final Predefined_OLAP_Right_Object_Type [] etl_Relevant_Types = new Predefined_OLAP_Right_Object_Type[]{
			Predefined_OLAP_Right_Object_Type.database,
			Predefined_OLAP_Right_Object_Type.cube,
			Predefined_OLAP_Right_Object_Type.dimension,
			Predefined_OLAP_Right_Object_Type.dimension_element,
			Predefined_OLAP_Right_Object_Type.cell_data,
			Predefined_OLAP_Right_Object_Type.ste_etl,
			Predefined_OLAP_Right_Object_Type.drillthrough
	};
	
	/* permission sets*/
	/**
	 * only for read operations: read project, read execution log, ... etc.
	 */
	public static final ElementPermission[] roPermSet1 = new ElementPermission[]{
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.N
			
	};
	
	/**
	 * for executing: runExecution, getComponentOutput,.. etc.
	 */
	public static final ElementPermission[] roPermSet2 = new ElementPermission[]{
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.W,
			ElementPermission.N
			
	};
	
	/**
	 * for adding new components and groups (but not adding projects to a group)
	 */
	public static final ElementPermission[] roPermSet3 = new ElementPermission[]{
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.W,// because we write to etl_seq as well
			ElementPermission.W,
			ElementPermission.W,
			ElementPermission.W,
			ElementPermission.N
			
	};
	
	/**
	 * for removing components and groups, remove executions,remove project from group
	 */
	public static final ElementPermission[] roPermSet4 = new ElementPermission[]{
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.R,
			ElementPermission.D,
			ElementPermission.N,
			ElementPermission.D,
			ElementPermission.N
			
	};
	
	/**
	 * only upload file
	 */
	public static final ElementPermission[] roPermSet5 = new ElementPermission[]{
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.W,
			ElementPermission.N
			
	};
	
	/**
	 * drillthough, drillthroughInfo
	 */
	public static final ElementPermission[] roPermSet6 = new ElementPermission[]{
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.N,
			ElementPermission.D
			
	};
	
	/**
	 * rename components, rename groups,addProjecttoGroup
	 */
	public static final ElementPermission[] roPermSet7 = new ElementPermission[]{
		ElementPermission.R,
		ElementPermission.R,
		ElementPermission.R,
		ElementPermission.W,
		ElementPermission.W,
		ElementPermission.W,
		ElementPermission.N
		
	};

	
	private OLAPAuthenticator(){
		configs = new HashMap<String, IDatabase>();
	}
	
	public static OLAPAuthenticator getInstance(){
		
		if(authenticator==null){
			authenticator = new OLAPAuthenticator();
			active = Settings.getInstance().getProjectsPersistence().equalsIgnoreCase("olap");
		}
		
		return authenticator;
	}
	
	public synchronized void authenticateComponent(Locator locator, Properties headerProps, ElementPermission[] rightObjectsPermissions) throws ConfigurationException{
		
		if(!active)
			return;
		
		String session = headerProps.getProperty(NamingUtil.session);
		if(session==null){
			throw new RuntimeException("This opertion requires a username and a password.");
		}
		IDatabase configdb = getConfigDB(session);
		checkRightObjects(headerProps,rightObjectsPermissions);
		
		ElementPermission right = rightObjectsPermissions[5];
		if(right.equals(ElementPermission.N))
			return;
		
		//ElementPermission projectGroupPermission = rightObjectsPermissions[7];
		
		IDimension etlsDim = configdb.getDimensionByName("etls");	
		etlsDim.setWithElementPermission(true);
		IDimension etlpropsDim = configdb.getDimensionByName("etlprops");
		ICube etlsCube = configdb.getCubeByName("etls");
		IElement[] etlElements = etlsDim.getElements(false);
		IElement name = etlpropsDim.getElementByName("name", false);
		
		boolean isGroup = locator.getManager()!=null && locator.getManager().equals(NamingUtil.group_manager);
		
		IElement e = findElement(locator.getRootName(), etlElements, isGroup, etlsCube, name);
		if(e==null)
			throw new RuntimeException("The "+ (isGroup?"group":"project")  + " "+ locator.getRootName() + " does not exist or the user has no sufficient rights to access it.");
					
		checkSinglePermission(e, right, isGroup, locator.getRootName());
		//do an extra check for the project's group (in project case)
		//if(!isGroup){
		//	checkProjectGroup(locator,etlElements,etlsCube,name,projectGroupPermission,locator.getRootName());					
		//}
	}
	
	private void checkSinglePermission(IElement e,ElementPermission right,boolean isGroup,String elementName){
		if(e.getPermission().higherOrEqual(right)){
				return;
			}else{
				throw new RuntimeException("The User has permission " + e.getPermission() + " on "+ (isGroup?"group":"project") + " "+ elementName + ", at least permission " + right + " is needed to perform this operation.");
			}
	}
	
	private IElement findElement(String componentName,IElement[] etlElements,boolean isGroup, ICube etlsCube,IElement name){
		
		for(IElement e:etlElements){
			if((!isGroup && e.getName().startsWith(NamingUtil.project_prefix)) ||
					(isGroup && e.getName().startsWith(NamingUtil.group_prefix))){	
				String elementName=null;
				try {
					elementName = etlsCube.getCell(new IElement[]{e,name}).getValue().toString();
				} catch (Exception e1) {
					// if HideElement=N and the permission on element is N then we can not know the name of the project
					continue;
				}
				if(elementName!=null && elementName.equals(componentName)){
					return e;
				}
			}			
		}
		
		return null;
	}
	
	/*private void checkProjectGroup(Locator loc,
			IElement[] etlElements, ICube etlsCube, IElement name, ElementPermission projectGroupPermission, String projectName) {
		
		try {
			if(projectGroupPermission.equals(ElementPermission.N)){
				return;
			}
			
			String groupName = ConfigPersistor.getInstance().getProjectGroup(loc);
			if(groupName==null)
				return;
			else{
				IElement e = findElement(groupName, etlElements, true, etlsCube, name);
				if(e==null)
					throw new RuntimeException("The User has N permission on group "+ groupName + ". At least permission " + projectGroupPermission + " is needed to perform this operation on the needed project, since this project is a member of this group.");
				
				checkSinglePermission(e, projectGroupPermission, true, groupName);
			}
		} catch (Exception e) {
			throw new RuntimeException("The user does not have enough permission on the project's group:" + e.getMessage());
		}
	}*/

	public synchronized ArrayList<String> getAuthenticatedComponents(ETL_Element_Types type,Properties headerProps, ElementPermission[] rightObjectsPermissions) throws ConfigurationException{
		
		if(!active)
			return null;
		
		String session = headerProps.getProperty(NamingUtil.session);
		if(session==null){
			throw new RuntimeException("This opertion requires a username and a password.");
		}
		
		checkRightObjects(headerProps,rightObjectsPermissions);
		ElementPermission right = rightObjectsPermissions[5];
		
		IDatabase configdb = getConfigDB(session);
		IDimension etlsDim = configdb.getDimensionByName("etls");
		etlsDim.setWithElementPermission(true);
		IDimension etlpropsDim = configdb.getDimensionByName("etlprops");
		ICube etlsCube = configdb.getCubeByName("etls");
		IElement[] elements = etlsDim.getElements(false);
		IElement name = etlpropsDim.getElementByName("name", false);
		ArrayList<String> foundList = new ArrayList<String>();
		for(IElement e:elements){
			if((type.equals(ETL_Element_Types.P) && e.getName().startsWith(NamingUtil.project_prefix)) ||
			   (type.equals(ETL_Element_Types.G) && e.getName().startsWith(NamingUtil.group_prefix))){
				// TODO the or condition should be removed as soon as olap 4.1 is used
				if(e.getPermission().higherOrEqual(right)){
					
					String elementName = etlsCube.getCell(new IElement[]{e,name}).getValue().toString();
					foundList.add(elementName);
				}
			}
		}
		
		return foundList;
	}
	
	public synchronized void drillthoughCheck(String databaseName,String cubeName ,Properties headerProps) throws ConfigurationException{
		
		if(!active)
			return;
		
		String session = headerProps.getProperty(NamingUtil.session);
		if(session==null){
			throw new RuntimeException("This operation requires a username and a password.");
		}
		
		IConnection conn  = getSessionConnection(session);
		IDatabase db = conn.getDatabaseByName(databaseName);
			
		if(db==null)
			throw new RuntimeException("Database " + databaseName + " does not exist or the user has no sufficient rights.");
		
		ICube cube = db.getCubeByName(cubeName);
		if(cube==null)
			throw new RuntimeException("Cube " + cubeName + " does not exist or the user has no sufficient rights.");

	}
	
	public void checkRightObjects(Properties headerProps,ElementPermission[] rightObjectsPermissions) throws ConfigurationException {	

		if(!active)
			return;
		
		String session = headerProps.getProperty(NamingUtil.session);
		if(session==null){
			throw new RuntimeException("This opertion requires a username and a password.");
		}
		IConnection conn  = getSessionConnection(session);
		HashMap<Predefined_OLAP_Right_Object_Type, ElementPermission> olaprights = conn.getUserInfo(true).getRightObjectsMap();
		if (olaprights==null){
			throw new RuntimeException("The right objects of the user could not be determined. Check the version of the Olap Server.");
		}
			
		for(int i=0;i<this.etl_Relevant_Types.length;i++){
			if(!(olaprights.get(etl_Relevant_Types[i]).higherOrEqual(rightObjectsPermissions[i]))){
				throw new RuntimeException("User has only permission " + olaprights.get(etl_Relevant_Types[i]) + " on right object " + etl_Relevant_Types[i]+ ", at least " + rightObjectsPermissions[i] + " is needed.");
			}
		}	
	}
	
	public IConnection getSessionConnection(String session){
		
		SessionConfiguration config = new SessionConfiguration();
		IConnectionConfiguration suiteConfig = Settings.getInstance().getSuiteConnectionConfiguration();
		config.setHost(suiteConfig.getHost());
		config.setPort(suiteConfig.getPort());
		config.setSession(session);
		config.setTimeout(suiteConfig.getTimeout());
		config.setSslPreferred(suiteConfig.isSslPreferred());
		
		IConnection conn = com.jedox.palojlib.main.ConnectionManager.getInstance().getConnection(config);
		conn.open();
		return conn;
	}
	
	public UserInfo getUserInfo(String session){
		
		if(session==null || session.isEmpty())//in case no session
			return new UserInfo("0", "", new String[]{}, new String[]{},null);
		
		IConnection conn  = getSessionConnection(session);
		return conn.getUserInfo(false);
	}
	
	public void remove(String session){
		configs.remove(session);	
	}
	
	private IDatabase getConfigDB(String session) throws ConfigurationException {

		IDatabase configdb = configs.get(session);
		if(configdb==null) {		

			IConnection conn  = getSessionConnection(session);
			configdb = conn.getDatabaseByName("Config");
			if(configdb==null)
				throw new RuntimeException("Config database does not exist.");
			configs.put(session, configdb);
		}
		return configdb;
	}	
		
}
