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
package com.jedox.etl.service;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.io.FileOutputStream;
import java.io.File;

import org.jasypt.util.text.BasicTextEncryptor;
import org.jdom.Element;
import org.jdom.Document;
import org.apache.axiom.om.OMElement;
import org.apache.axiom.soap.SOAPHeader;
import org.apache.axis2.AxisFault;
import org.apache.axis2.context.MessageContext;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.ITypes.Managers;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.OLAPAuthenticator;
import com.jedox.etl.core.config.OLAPAuthenticator.ETL_Element_Types;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.ConfigConverter;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.ConfigValidator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.ExecutionDetail;
import com.jedox.etl.core.execution.ExecutionException;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.execution.ResultCodes;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.StateManager;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ResultCodes.DataTargets;
import com.jedox.etl.core.execution.StateManager.SortDirection;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.DatastoreManager;
import com.jedox.etl.core.persistence.hibernate.HibernateUtil;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.prototype.PrototypeGenerator;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.util.ClassUtil;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.docu.DocuUtil;
import com.jedox.etl.core.util.svg.GraphManager;
import com.jedox.etl.core.writer.CSVWriter;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.main.SessionConfiguration;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.main.ConnectionManager;
import com.jedox.palojlib.main.ClientInfo;

/**
 * The Main ETLService class providing the API to the ETLServer.
 * @author chris
 *
 */
public class ETLService {

	//throw new AxisFault("Project " + name + " not found");
	// QName .... soap 1.1 fault code... must be namespace qualified
	// fault reasong .. .human readable message
	// fault actor ... not necessary for fault generating entity, but may
	// fault detail ... only allowed, when error during processing of body element
	// Soap 1.2: see http://www.w3.org/TR/2003/REC-soap12-part1-20030624/#soapfault
	//throw new AxisFault(new QName("http://test.org", "FaultCode", "test"), "FaultReason", new Exception("This is a test Exception"));

	private static Log log = LogFactory.getLog(ETLService.class);
	//private MetadataUtil meta = new MetadataUtil();

	public ETLService() {
		try {
			ConfigManager.getInstance();
			Settings.getInstance().getContext(Settings.PersistenceCtx).setProperty("mode", "server");
			new HibernateUtil().close();
			StateManager.getInstance();
		}
		catch (Exception e) {
			log.error("Error reading files from project repository: ",e);
		}
	}
	
	
	//******************* P R I V A T E  M E T H O D S ********************************

	private Properties getProperties(Variable[] variables) {
		Properties properties = new Properties();
		if (variables != null)
			for (Variable v : variables) {
				if ((v.getName() != null) && (v.getValue() != null))
					properties.setProperty(v.getName(), v.getValue());
			}
		return properties;
	}

	private String getNamesAsString(String[] names) {
		if (names == null) return "";
		StringBuffer b = new StringBuffer();
		for (String name: names) {
			b.append(name+" ");
		}
		return b.toString();
	}

	private String getPrintable(String locator) {
		if (locator == null) return "projects";
		return locator;
	}

	private String getPrintable(String[] values, String defvalue) {
		StringBuffer result = new StringBuffer();
		if (values == null) return "";
		for (String value : values) {
			if (value == null)
				result.append(defvalue);
			else
				result.append(value+" ");
		}
		return result.toString();
	}	
	
	private String getPrintable(String[] locators) {
		return getPrintable(locators,"projects ");
	}

	private void logDescriptor(ResultDescriptor d, boolean errorsOnly) {
		if (d.getValid()) {
			if (!errorsOnly && d.getResult()!=null)
				log.info(d.getResult());
		}
		else
			log.error(d.getErrorMessage());
	}

	private void logDescriptor(ExecutionDescriptor d, boolean errorsOnly) {
		if (d.getValid()) {
			if (!errorsOnly && d.getResult()!=null)
				log.info(d.getResult());
		}
		else
			log.error(d.getErrorMessage());
	}
	
	
	private ComponentOutputDescriptor[] getOutputDescription(Row row) {
		if (row != null) {
			ArrayList<ComponentOutputDescriptor> result = new ArrayList<ComponentOutputDescriptor>();
			for (IColumn column : row.getColumns()) {
				ComponentOutputDescriptor d = new ComponentOutputDescriptor();
				d.setName(column.getName());
				d.setDefaultValue(null);
				d.setPosition(row.indexOf(column)+1);
				d.setType(column.getValueType().getCanonicalName());
				d.setOriginalName((column.getAliasElement() != null) ? column.getAliasElement().getOrigin() : column.getName());
				result.add(d);
			}
			return result.toArray(new ComponentOutputDescriptor[result.size()]);
		}
		return null;
	}

	private ExecutionDescriptor getDescriptor(ExecutionState state) {
		ExecutionDescriptor d = new ExecutionDescriptor();
		d.setId(state.getId());
		d.setProject(state.getProject());
		d.setType(state.getType());
		d.setName(state.getName());
		d.setExecutionType(state.getExecutionType().toString());
		d.setErrors(state.getErrors());
		d.setWarnings(state.getWarnings());
		d.setStartDate((state.getStartDate() == null) ? 0 : state.getStartDate().getTime());
		d.setStopDate((state.getStopDate() == null) ? 0 : state.getStopDate().getTime());
		d.setStatus(state.getString(state.getStatus()));
		d.setStatusCode(state.getNumeric(state.getStatus()));
		d.setErrorMessage(state.getFirstErrorMessage());
		d.setMetadata(getOutputDescription(state.getMetadata()));
		d.setUserName(state.getUserName());
		switch (state.getDataTarget()) {
		case CSV_INLINE: 
		case XML_INLINE: 
		case CSV_PERSISTENCE: 
			d.setResult(state.getData()); break;
		default: d.setResult(null);
		}
		if (!Codes.statusInvalid.equals(state.getStatus()))
			d.setValid(true);
		return d;
	}
	
	private ExecutionDetailEntry getDetailDescriptor(ExecutionDetail detail) {
		ExecutionDetailEntry d = new ExecutionDetailEntry();
		d.setLocator(detail.getLocator());
		d.setProcessedInputRows(detail.getProcessedInputRows());
		d.setProcessedOutputRows(detail.getProcessedOutputRows());
		d.setType(detail.getType());
		d.setScope(detail.getScope() != null ? detail.getScope().toString() : "");
		d.setInputCalls(detail.getInputCalls());
		d.setOutputCalls(detail.getOutputCalls());
		d.setRuntime(detail.getRuntime());
		return d;
	}

	private ExecutionDescriptor getErrorDescriptor(String message) {
		log.error(message);
		ExecutionDescriptor d = new ExecutionDescriptor();
		d.setErrorMessage(message);
		d.setErrors(1);
		ResultCodes codes = new ResultCodes();
		d.setStatus(codes.getString(Codes.statusFailed));
		d.setStatusCode(codes.getNumeric(Codes.statusFailed));
		return d;
	}

	private ComponentDependencyDescriptor getErrorDependencyDescriptor(String locator, String message) {
		log.error(message);
		ComponentDependencyDescriptor notValidCD= new ComponentDependencyDescriptor();
		notValidCD.setName(locator);
		notValidCD.setComponents(new String[0]);
		notValidCD.setErrorMessage(message);
		notValidCD.setValid(false);
		return notValidCD;
	}	
	
	

	private Element getGroupXML(GroupDescriptor desc) {

		Document doc = new Document();
		Element groupElement = new Element("group");
		Element commentElement = new Element("comment");
		commentElement.addContent(desc.getDescription());
/*		
		Element changedByElement = new Element("changedby");
		changedByElement.addContent(desc.getChangedUser());
		Element changedAtElement = new Element("changeddate");
		changedAtElement.addContent(String.valueOf(desc.getChangedDate()));
*/		
		Element membersElement = new Element("members");
		groupElement.setAttribute("name", desc.getName());
		groupElement.addContent(commentElement);
//		groupElement.addContent(changedByElement);
//		groupElement.addContent(changedAtElement);
		groupElement.addContent(membersElement);
		doc.addContent(groupElement);
		for(String proj:desc.getProjects()){
			Element projElement = new Element("member");
			projElement.setAttribute("name", proj);
			membersElement.addContent(projElement);
		}
		return doc.getRootElement();
	}

	private ArrayList<String> getListFromArray(String[] array){
		ArrayList<String> list = new ArrayList<String>();
		for(String s:array)
			list.add(s);

		return list;
	}
	
	/**
	 * @param properties
	 * @param header
	 */
	private void addLoginParameters(Properties properties, Properties header) {
		if(header.getProperty(NamingUtil.username)!=null)properties.setProperty(NamingUtil.hiddenInternalPrefix() + NamingUtil.username, header.getProperty(NamingUtil.username));
		if(header.getProperty(NamingUtil.password)!=null)properties.setProperty(NamingUtil.hiddenInternalPrefix() + NamingUtil.password, header.getProperty(NamingUtil.password));
	}

	/**
	 * needed for data preview methods
	 * @param offset starting from 1
	 * @return real offset starting from 0
	 * @throws Exception
	 */
	private int handleOffset(int offset) throws Exception {
		if(offset<0)
			throw new Exception("Minimum value of offset is 1.");
		if(offset>0)
			offset--;
		return offset;
	}
	
	private List<String> getProjectNames() throws ConfigurationException {
		
		ArrayList<String> filteredNames = OLAPAuthenticator.getInstance().getAuthenticatedComponents(ETL_Element_Types.P,getHeaderInfo(),OLAPAuthenticator.roPermSet1);
		if(filteredNames!= null)
			return filteredNames;
		else
			return ConfigManager.getInstance().getNames(Locator.parse(""));
	}	
	
	// *********************** G E T  M E T H O D E S ********************


	public ResultDescriptor login(String user,String password, String olapSession){
		log.debug("Login request");
		ResultDescriptor desc = new ResultDescriptor();
		try{
			boolean newSessionCreated = false;
			if(olapSession==null){
				IConnectionConfiguration suiteconfig = Settings.getInstance().getSuiteConnectionConfiguration();
				IConnectionConfiguration userconfig = new ConnectionConfiguration();
				//same as suite connection
				userconfig.setHost(suiteconfig.getHost());
				userconfig.setPort(suiteconfig.getPort());
				userconfig.setSslPreferred(suiteconfig.isSslPreferred());
				userconfig.setTimeout(suiteconfig.getTimeout());
				userconfig.setClientInfo(new ClientInfo("Jedox ETL","etl login"));
				//only different user and password
				userconfig.setUsername(user);
				userconfig.setPassword(password);
				IConnection conn = ConnectionManager.getInstance().getConnection(userconfig);
				olapSession = conn.open();
				if(user.indexOf('\t')!=-1)
					user = user.substring(user.indexOf('\t')+1);
				newSessionCreated = true;
			}else{
				IConnection conn = OLAPAuthenticator.getInstance().getSessionConnection(olapSession);
				user = conn.getUserInfo(false).getName();
			}
			BasicTextEncryptor crypt = new BasicTextEncryptor();
			crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));		
			desc.setResult(crypt.encrypt(user+"\n"+password+"\n"+olapSession + "\n" + newSessionCreated));
			desc.setValid(true);

		}catch(Exception e){
			desc.setErrorMessage(e.getMessage());
			log.error(e.getMessage());
			desc.setValid(false);
		}
		log.debug("finished");
		return desc;
	}

	public ResultDescriptor logout(String etlsession){
		log.debug("Logout request");		
		ResultDescriptor desc = new ResultDescriptor();
		try{
			String[] results =  decrypt(etlsession);
			if(results[3].equals("true")){
				String session = results[2];
				OLAPAuthenticator.getInstance().remove(session);
				IConnectionConfiguration config = Settings.getInstance().getSuiteConnectionConfiguration();
				SessionConfiguration sessionconfig = new SessionConfiguration();
				sessionconfig.setHost(config.getHost());
				sessionconfig.setPort(config.getPort());
				sessionconfig.setSession(session);
				sessionconfig.setSslPreferred(true);
				sessionconfig.setTimeout(30000);
				IConnection conn = ConnectionManager.getInstance().getConnection(sessionconfig);
				conn.close();
			}
			desc.setResult("logging out ..");
			log.info("logging out the session.");
			desc.setValid(true);

		}catch(Exception e){
			//logout should never gives an error, the goal of this method is to logout the olap session, 
			//if it is already logged out then there is nothing to do 
			
			//desc.setErrorMessage(e.getMessage());
			log.debug(e.getMessage());
			//desc.setValid(false);
			desc.setValid(true);
		}
		log.debug("finished");
		return desc;
	}

	public ResultDescriptor addGroup(String name, String description){
		ResultDescriptor d = new ResultDescriptor();
		try{
			Properties header = getHeaderInfo();
			GroupDescriptor desc = new GroupDescriptor();
			desc.setName(name);
			desc.setDescription(description);
			Element groupXml = getGroupXML(desc);

			Locator loc = Locator.parseGroup(name);	
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			
			Element config = ConfigManager.getInstance().findGroup(loc);
			if(config !=null){
				config = (Element)config.clone();
				OLAPAuthenticator.getInstance().authenticateComponent(loc, header, OLAPAuthenticator.roPermSet3);
				groupXml.removeChild("members");
				groupXml.addContent(config.getChild("members").detach());
			}
			Element oldGroup = ConfigManager.getInstance().addGroup(loc, groupXml);
			d.setValid(true);
			if(oldGroup==null)
				d.setResult("added group " + name);
			else
				d.setResult("replaced group " + name);
		}catch(Exception e){
			String errorMessage = "Error while trying to add a group: " + e.getMessage();
			log.error(errorMessage);
			d.setErrorMessage(errorMessage);
			d.setValid(false);
		}
		return d;
	}
	
	public ResultDescriptor renameGroup(String name, String newName){
		ResultDescriptor d = new ResultDescriptor();
		try{
			Properties header = getHeaderInfo();
			
			if(newName!=null && (newName.isEmpty() || newName.contains(".")))
				throw new RuntimeException("newName \"" + newName + "\" is not a valid value for the new group name.");

			
			Locator newloc = Locator.parseGroup(newName);
			if(ConfigManager.getInstance().findGroup(newloc)!=null)
				throw new RuntimeException("A Group with name " + newName + " already exists.");
			
			Locator loc = Locator.parseGroup(name);	
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			
			Element config = ConfigManager.getInstance().findGroup(loc);
			if(config !=null){
				OLAPAuthenticator.getInstance().authenticateComponent(loc, header, OLAPAuthenticator.roPermSet7);
				config = (Element)config.clone();
				config.setAttribute("name", newName);
			}else{
				throw new RuntimeException("Group " + name + " does not exist.");
			}


			ConfigManager.getInstance().addGroup(loc, config);
			d.setValid(true);
			d.setResult("rename group " + name);

		}catch(Exception e){
			String errorMessage = "Error while trying to add a group: " + e.getMessage();
			log.error(errorMessage);
			d.setErrorMessage(errorMessage);
			d.setValid(false);
		}
		return d;
	}


	public ResultDescriptor addProjectsToGroup(String group,String[] projects){
		ResultDescriptor d = new ResultDescriptor();
		try{
			Locator loc = Locator.parseGroup(group);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			OLAPAuthenticator.getInstance().authenticateComponent(loc, header, OLAPAuthenticator.roPermSet7);
					
			Element c = ConfigManager.getInstance().findGroup(loc);
			if(c==null)
				throw new Exception("Group " + group + " does not exist.");
			
			Element config = (Element) c.clone();

			ArrayList<String> visibleProjectsList = getListFromArray(getNames(null));
			ArrayList<String> toAddList = getListFromArray(projects);

			ArrayList<String> currentProjectsList = new ArrayList<String>();		
			for(Object member: config.getChild("members").getChildren()){
				String memberProj = ((Element)member).getAttributeValue("name");
				currentProjectsList.add(memberProj);
			}

			for(String newProject:toAddList){
				if(!visibleProjectsList.contains(newProject)){
					throw new Exception("Project " + newProject + " does not exist or the user does not have sufficient rights.");
				}
				if(currentProjectsList.contains(newProject)){
					throw new Exception("Project " + newProject + " is already a member of group " + group);
				}
				String[] projectGroup = getGroupsNames(newProject);
				if(projectGroup.length>0){
					throw new Exception("Project " + newProject + " exists already in another group, a project can only exist in one group atmost.");
				}

				Element newMember = new Element("member");
				newMember.setAttribute("name", newProject);
				config.getChild("members").addContent(newMember);	

			}


			ConfigManager.getInstance().addGroup(loc, config);
			d.setValid(true);
			d.setResult("added projects "+ toAddList.toString() + " to group " + group);

		}
		catch(Exception e){
			log.error(e.getMessage());
			d.setErrorMessage(e.getMessage());
			d.setValid(false);
		}
		return d;
	}

	@SuppressWarnings("unchecked")
	public ResultDescriptor removeProjectsFromGroup(String group,String[] projects){
		ResultDescriptor d = new ResultDescriptor();
		try{
			Locator loc = Locator.parseGroup(group);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));

			OLAPAuthenticator.getInstance().authenticateComponent(loc, header, OLAPAuthenticator.roPermSet4);

			Element c = ConfigManager.getInstance().findGroup(loc);
			if(c==null)
				throw new Exception("Group " + group + " does not exist.");
			
			Element config = (Element) c.clone();

			ArrayList<String> visibleProjectsList = getListFromArray(getNames(null));
			ArrayList<String> toRemoveList = getListFromArray(projects);

			for(String toRemove:toRemoveList){
				if(!visibleProjectsList.contains(toRemove)){
					throw new Exception("Project " + toRemove + " does not exist or the user has no rights to work on it");
				}
			}

			List<Element> memberslist = new ArrayList<Element>();
			memberslist.addAll(config.getChild("members").getChildren());
			for(Element member:memberslist){
				if(toRemoveList.contains(member.getAttributeValue("name"))){
					member.detach();
					config.getChild("members").removeContent(member);
				}
			}

			ConfigManager.getInstance().addGroup(loc, config);
			d.setValid(true);
			d.setResult("Deleted projects " + toRemoveList.toString() +" from group " + group);

		}
		catch(Exception e){
			log.error(e.getMessage());
			d.setErrorMessage(e.getMessage());
			d.setValid(false);
		}
		return d;
	}

	public ResultDescriptor removeGroup(String group){
		ResultDescriptor d = new ResultDescriptor();
		try{
			Locator loc = Locator.parseGroup(group);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			OLAPAuthenticator.getInstance().authenticateComponent(loc, header, OLAPAuthenticator.roPermSet4);
			ConfigManager.getInstance().removeGroup(loc);
			d.setValid(true);
			d.setResult("removed group " + group);

		}catch(Exception e){
			log.error(e.getMessage());
			d.setErrorMessage(e.getMessage());
			d.setValid(false);
		}
		return d;
	}

	public String[] getGroupsNames(String project){
		try {
			ArrayList<String> allowedGroups = OLAPAuthenticator.getInstance().getAuthenticatedComponents(ETL_Element_Types.G, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			if(allowedGroups==null)
				return ConfigManager.getInstance().getGroups(project,false);

			if(project==null)
				return allowedGroups.toArray(new String[0]);
			else
			{
				String[] groups = ConfigManager.getInstance().getGroups(project,false);
				ArrayList<String> filteredList = new ArrayList<String>();

				for(String g:groups){
					if(allowedGroups.contains(g))
						filteredList.add(g);
				}

				return filteredList.toArray(new String[0]);

			}
		} catch (ConfigurationException e) {
			log.error("Error while trying to get the groups info: " + e.getMessage());
		}
		return null;
	}

	public GroupDescriptor getGroupDescriptor(String group){
		Element groupElement;
		GroupDescriptor desc = new GroupDescriptor();
		try {
			Locator loc = Locator.parseGroup(group);
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet1);
			
			groupElement = ConfigManager.getInstance().findGroup(loc);
			if(groupElement==null)
				throw new Exception("Group " + group + " does not exist.");
			
			//filter the groups project
			desc.setName(groupElement.getAttributeValue("name"));
			desc.setDescription(groupElement.getChildText("comment"));
			desc.setChangedUser(groupElement.getAttributeValue("modifiedBy"));
			desc.setChangedDate(Long.parseLong(groupElement.getAttributeValue("modified")));
			ArrayList<String> visibleProjectsList = getListFromArray(getNames(null));

			List<String> memberslist = new ArrayList<String>();
			for(Object memberObj:groupElement.getChild("members").getChildren()){
				String memberName= ((Element) memberObj).getAttributeValue("name");
				if(visibleProjectsList.contains(memberName)){
					memberslist.add(memberName);
				}
			}
			desc.setProjects(memberslist.toArray(new String[0]));
			desc.setValid(true);

		} catch (Exception e) {
			log.error(e.getMessage());
			desc.setErrorMessage(e.getMessage());
			desc.setValid(false);
			return desc;
		}

		return desc;

	}

	/**
	 * gets the configuration of a set of components as XML.
	 * @param locators an array of paths to the components in the format {component.manager.}component: e.g: "myProject.connections.myConnection".
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result contains the XML configuration of the component.
	 */
	public ResultDescriptor[] getComponentConfigs(String[] locators) {
		log.debug("getting component configs for: "+getNamesAsString(locators));
		List<String> allowedProjects = null;
		try {
			allowedProjects = getProjectNames();
		} catch (ConfigurationException e1) {
			ResultDescriptor desc = new ResultDescriptor();
			desc.setValid(false);
			desc.setErrorMessage(e1.getMessage());
			return new ResultDescriptor[]{desc};
		}
		if (locators==null || locators.length==0) {
			log.debug("number of projects: "+allowedProjects.size());
			locators = allowedProjects.toArray(new String[0]);
		}
					
		ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
		for (String locator : locators) {
			ResultDescriptor d = new ResultDescriptor();
			try {
				Locator loc = Locator.parse(locator);
				if(!allowedProjects.contains(loc.getRootName()))
					throw new Exception("The project " + loc.getRootName() + " does not exist or the user has no sufficient rights.");
				d.setResult((ConfigManager.getInstance().getConfigurationString(loc)));
			}
			catch (Exception e) {
				d.setErrorMessage(e.getMessage());
			}
			results.add(d);
			logDescriptor(d,true);
		}
		log.debug("finished");
		return results.toArray(new ResultDescriptor[results.size()]);
	}

	/**
	 * gets the names of all children hosted by the component or manager denoted by the locator
	 * @param locator the path to a component or manager: e.g: "myProject.connections", "myProject.connections.myConnection". If null the root project manager is selected.
	 * @return an array of names
	 * @throws AxisFault
	 */
	public String[] getNames(String locator) throws AxisFault {
		log.debug("getting locateable names for: "+getPrintable(locator));
		try {
			Locator loc = Locator.parse(locator);
			List<String> names=null;
			if(loc.isEmpty()) {
				names = getProjectNames();
			} 
			else {
				OLAPAuthenticator.getInstance().authenticateComponent(loc,getHeaderInfo(),OLAPAuthenticator.roPermSet1);
				names = ConfigManager.getInstance().getNames(loc);
			}
			log.debug("finished");
			return names.toArray(new String[0]);
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getNames for "+getPrintable(locator)+": "+e.getMessage());
		}
	}

	/**
	 * gets the locators of all children hosted by the component or manager denoted by the locator
	 * @param locator the path to a component or manager: e.g: "myProject.connections", "myProject.connections.myConnection". If null the root project manager is selected.
	 * @return an array of locators
	 * @throws AxisFault
	 */
	public String[] getLocators(String locator) throws AxisFault {
		log.debug("getting children component locators for: "+getPrintable(locator));
		try {
			Locator loc = Locator.parse(locator);
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			Locator[] locators = ConfigManager.getInstance().getLocators(loc);
			String[] locStrings = new String[locators.length];
			for (int i=0; i<locators.length; i++) {
				locStrings[i] = locators[i].toString();
			}
			log.debug("finished");
			return locStrings;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getLocators for "+getPrintable(locator)+": "+e.getMessage());
		}
	}

	/**
	 * gets the component scopes (connections,extracts,transforms,loads,jobs,projects)
	 * @return the array of the scopes
	 */
	public String[] getScopes() {
		log.debug("getting scopes");
		ArrayList<String> scopes = new ArrayList<String>();
		for (ITypes.Components c : ITypes.Components.values()) {
			scopes.add(c.toString()+"s");
		}
		log.debug("finished");
		return scopes.toArray(new String[scopes.size()]);
	}

	/**
	 * gets the types registered for the given component scope
	 * @param scope the scope type name as registered in the component.xml (e.g. connections). see {@link #getScopes()}
	 * @param name the name of the component as registered in the component.xml (e.g. File) see {@link #getScopes()}
	 * @return the array of {#link ComponentTypeDescriptor ComponentTypeDescriptors} registered for this scope
	 * @throws AxisFault
	 */
	public ComponentTypeDescriptor[] getComponentTypes(String scope, String name) throws AxisFault {
		log.debug("getting component types descriptors for scope " + scope);
		ArrayList<ComponentTypeDescriptor> result = new ArrayList<ComponentTypeDescriptor>();
		List<ComponentDescriptor> descriptors = new ArrayList<ComponentDescriptor>();
		if (name==null) {
			 descriptors = ComponentFactory.getInstance().getComponentDescriptorsSorted(scope);
		}	 
		else {
			ComponentDescriptor descriptor=ComponentFactory.getInstance().getComponentDescriptor(name, scope);
			if (descriptor!=null)
				descriptors.add(descriptor);
		}
		for (ComponentDescriptor d : descriptors) {
			ComponentTypeDescriptor ce = new ComponentTypeDescriptor();
			ce.setScope(scope);
			ce.setType(d.getName());
			ce.setClassname(d.getClassName());
			ce.setSchema(ConfigValidator.getInstance().getSchema(d.getClassName()));
			ce.setCaption(d.getCaption());
			try {
				ce.setTree(ClassUtil.implementsInterface(Class.forName(d.getClassName()), ITreeSource.class));
			} catch (ClassNotFoundException e) {
				log.error("Class not found: "+e.getMessage());
			}
			ce.setAllowedConnectionTypes(d.getAllowedConnectionTypes());
			int paramSize = d.getParameters().size();
			String[] paramNames = new String[paramSize];
			String[] paramValues = new String[paramSize];
			Iterator<Object> keyIter = d.getParameters().keySet().iterator();
			for(int i=0;i<d.getParameters().size();i++){
				String keyName = keyIter.next().toString();
				paramNames[i]= keyName;
				paramValues[i]=d.getParameters().getProperty(keyName);
			}
			ce.setParametersNames(paramNames);
			ce.setParametersValues(paramValues);
			result.add(ce);
		}
		log.debug("finished");
		return result.toArray(new ComponentTypeDescriptor[result.size()]);
	}

	/**
	 * validates component configs. if a config for a component is given, validation is done with respect to that config. Else it is assumed, that the component already exists and validation is done with respect to its existing config.
	 * @param locators the paths of the components to validate in the format {component.manager.}component: e.g: "myProject.connections.myConnection".
	 * @param configs the configs for some components (optional).
	 * @return an array of {@link ResultDescriptor ResultDescriptors}, one per component. Result is "OK", if validation was successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] validateComponents(String[] locators, String[] configs) throws AxisFault {
		log.debug("validating component configs "+getNamesAsString(locators));
		try {
			ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();

			for (int i=0; i<locators.length; i++) {
				ResultDescriptor d = new ResultDescriptor();
				if (configs == null || i >= configs.length || configs[i] == null ) { //assume that components are present
					try {
						Locator loc = Locator.parse(locators[i]);
						OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
						ConfigManager.getInstance().validate(loc);
						d.setResult("OK");
					}
					catch (Exception e) {
						d.setErrorMessage("Failed: "+e.getMessage());
					}
				}
				else { //add temporary config
					String locatorString = locators[i];
					Locator loc = Locator.parse(locatorString);
					String tempProjectName = NamingUtil.internal(loc.getRootName()+String.valueOf(System.nanoTime()));
					ConfigManager.getInstance().copyProject(loc.getName(), tempProjectName);
					String oldRootName = loc.getRootName();
					loc.setRootName(tempProjectName);
					Element config = XMLUtil.stringTojdom(configs[i]);
					if (loc.isRoot() || loc.isEmpty()) {//if locator denotes a project.  set temp name in config.
						config.setAttribute("name",tempProjectName);
					}
					ConfigManager.getInstance().add(loc, config);
					try {
						ConfigManager.getInstance().validate(loc);
						d.setResult("OK");
					}
					catch (Exception e) {
						String errorMessage = e.getMessage().replaceAll(tempProjectName,oldRootName);
						d.setErrorMessage("Failed: "+ errorMessage);
					}
					finally {
						ConfigManager.getInstance().removeElement(loc.getRootLocator());
					}
				}
				results.add(d);
				logDescriptor(d,true);
			}
			log.debug("finished");
			return results.toArray(new ResultDescriptor[results.size()]);
		}
		catch (Exception e) {
			throw new AxisFault("Failed to validate "+getPrintable(locators)+": "+e.getMessage());
		}
	}

	/**
	 * migrates component configs to actual format
	 * @param configs the configs for some components to migrate
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result is the migrated configuration, if successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] migrateComponents(String[] configs) throws AxisFault {
		log.debug("migrating component configs...");
		try {
			ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
			for (int i=0; i<configs.length; i++) {
				ResultDescriptor d = new ResultDescriptor();
				try { 
					String result = XMLUtil.jdomToString(new ConfigConverter().convert(XMLUtil.stringTojdom(configs[i]),null,null));
					d.setResult(result);
				}
				catch (Exception e) {
					d.setErrorMessage("Failed: "+e.getMessage());
				}
				results.add(d);
				logDescriptor(d,true);
			}
			log.debug("finished");
			return results.toArray(new ResultDescriptor[results.size()]);
		}
		catch (Exception e) {
			throw new AxisFault("Failed to migrate: "+e.getMessage());
		}
	}
	
	public ResultDescriptor[] updateComponents(String[] locators, String[] configs) throws AxisFault {
		log.debug("checking components for update...");
		if (locators == null) locators = new String[configs.length];
		if (locators.length != configs.length) {
			throw new AxisFault("Number of locators has to be identical to number of configs given.");
		}
		ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
		boolean conflict = false;
		for (int i=0; i<locators.length; i++) {
			String locator = locators[i];
			String config = configs[i];
			ResultDescriptor d = new ResultDescriptor();
			results.add(d);
			d.setErrorMessage("Newer component exists on server.");
			d.setResult("not added");
			try {
				Element element = XMLUtil.stringTojdom(config);
				Locator loc = Locator.parse(locator);
				if (loc.isEmpty()) {
					//if locator is empty get correct name from config.
					loc = Locator.parse(element.getAttributeValue("name"));
					locator = loc.toString();
				} 
				else  { //set locator name in config.
					element.setAttribute("name", loc.getName());
				}
				Element existing = ConfigManager.getInstance().get(loc);
				if (existing != null) {
					String existingTimestamp = existing.getAttributeValue("modified","0");
					String updateTimestamp = element.getAttributeValue("modified","0");
					if (Long.parseLong(existingTimestamp) > Long.parseLong(updateTimestamp)) {
						// More recent version of component available 
						conflict = true;
						d.setResult("conflict");
					}
				}
				else { // Component has been deleted, but project still exists					
					conflict=true;
					d.setResult("conflict");
				}
			}
			catch (Exception e) {
				d.setErrorMessage("Failed to add "+locator+": "+e.getMessage());
			}
		}
		if (conflict)
			return results.toArray(new ResultDescriptor[results.size()]); 
		else 			
			return addComponents(locators,configs);
	}
	

	/**
	 * adds components to a given manger
	 * @param locators the paths of the new components in the format {component.manager.}component: e.g: "myProject.connections.myConnection". If null the root project manager is selected.
	 * @param configs the configuration XMLs of the components to add.
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result is "added component [locator]" or "replaced component [locator]", if successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] addComponents(String[] locators, String[] configs) throws AxisFault {
		log.debug("adding components "+getNamesAsString(locators));
		if (locators == null) locators = new String[configs.length];
		if (locators.length != configs.length) {
			throw new AxisFault("Number of locators has to be identical to number of configs given.");
		}
		ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
		for (int i=0; i<locators.length; i++) {
			String locator = locators[i];
			String config = configs[i];
			ResultDescriptor d = new ResultDescriptor();
			try {
				Element element = XMLUtil.stringTojdom(config);
				Locator loc = Locator.parse(locator);
				Properties header = getHeaderInfo();
				loc.setSessioncontext(header.getProperty(NamingUtil.session));

				if (element.getChild("project")!=null )
					// Avoid adding of repository.xml
					throw new AxisFault("File is not a valid ETL-project, it can not be added.");					
				if (loc.isEmpty()) {
					//if locator is empty get correct name from config.
					loc = Locator.parse(element.getAttributeValue("name"));
					locator = loc.toString();
				} 
				else  { //set locator name in config.
					element.setAttribute("name", loc.getName());
				}
				if(ConfigManager.getInstance().get(loc.getRootLocator())!= null){
					OLAPAuthenticator.getInstance().authenticateComponent(loc.getRootLocator(), header,OLAPAuthenticator.roPermSet3);
				}else{
					OLAPAuthenticator.getInstance().checkRightObjects(header, OLAPAuthenticator.roPermSet3);
				}
				Element e = ConfigManager.getInstance().add(loc, element);
				if (e == null)
					d.setResult("added component "+locator);
				else
					d.setResult("replaced component "+locator);
			}
			catch (Exception e) {
				d.setErrorMessage("Failed to add "+locator+": "+e.getMessage());
			}
			results.add(d);
			logDescriptor(d,false);
		}
		log.debug("finished");
		return results.toArray(new ResultDescriptor[results.size()]);
	}
	
	/**
	 * rename components
	 * @param locators the paths of the new components in the format {component.manager.}component: e.g: "myProject.connections.myConnection". If null the root project manager is selected.
	 * @param newNames the new names of the component.
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result is "added component [locator]" or "replaced component [locator]", if successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] renameComponents(String[] locators, String[] newNames, boolean updateReferences) throws AxisFault {
		log.debug("renaming components "+getNamesAsString(locators));
		if (locators.length != newNames.length) {
			throw new AxisFault("Number of locators has to be identical to number of new names given.");
		}
		ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
		for (int i=0; i<locators.length; i++) {
			String locator = locators[i];
			String newName = newNames[i];
			ResultDescriptor d = new ResultDescriptor();
			try {
				if(newName!=null && (newName.isEmpty() || newName.contains(".")))
					throw new RuntimeException("newName \"" + newName + "\" is not a valid value for the new component name.");

				Locator loc = Locator.parse(locator);
				Properties header = getHeaderInfo();
				loc.setSessioncontext(header.getProperty(NamingUtil.session));			
				if (loc.isEmpty()) {
					throw new RuntimeException("Locator can not be empty.");
				}
				Element element = ConfigManager.getInstance().get(loc);
				if (element==null)
					throw new ConfigurationException("Locator " + loc + " does not exist.");			
				if (loc.isRoot()) {
					Locator newloc = Locator.parse(newName);
					if(ConfigManager.getInstance().get(newloc)!=null)
						throw new ConfigurationException("A project with name " + newName + " already exists.");
				} else {
					
					//Locator newloc = Locator.parse(loc.getRootName() + "." + loc.getManager() + "." + newName);
					Locator newloc = loc.clone().reduce().add(newName);
					if(ConfigManager.getInstance().get(newloc)!=null)
						throw new ConfigurationException("A component with name " + newName + " already exists.");
					
					if(loc.getManager().equals(Managers.extracts.toString())){
						newloc = Locator.parse(loc.getRootName() + "." + Managers.transforms + "." + newName);
						if(ConfigManager.getInstance().get(newloc)!=null)
							throw new ConfigurationException("A transform with name " + newName + " already exists, an extract and transform can not have the same name.");
					}
					if(loc.getManager().equals(Managers.transforms.toString())){
						newloc = Locator.parse(loc.getRootName() + "." + Managers.extracts + "." + newName);
						if(ConfigManager.getInstance().get(newloc)!=null)
							throw new ConfigurationException("An extract with name " + newName + " already exists, an extract and transform can not have the same name.");
					}
				}
				if(ConfigManager.getInstance().get(loc.getRootLocator())!= null){
					OLAPAuthenticator.getInstance().authenticateComponent(loc.getRootLocator(), header,OLAPAuthenticator.roPermSet7);
				}
				// Rename Componenent
				int depCount=ConfigManager.getInstance().renameComponent(loc, newName,updateReferences);
				d.setResult("renamed component "+locator+" to "+newName+ (depCount>0?". Updated dependent components: "+depCount:"") );
			}
			catch (Exception e) {
				d.setErrorMessage("Failed to rename "+locator+": "+e.getMessage());
			}
			results.add(d);
			logDescriptor(d,false);
		}
		log.debug("finished");
		return results.toArray(new ResultDescriptor[results.size()]);
	}

	/**
	 * removes components
	 * @param locators the paths to the components in the format {component.manager.}component: e.g: "myProject.connections.myConnection".
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result is "removed component [locator]", if successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] removeComponents(String[] locators) throws AxisFault {
		log.debug("removing components "+getNamesAsString(locators));
		try {
			ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
			Properties header = getHeaderInfo();

			for (String locator: locators) {
				ResultDescriptor d = new ResultDescriptor();
				Locator loc = Locator.parse(locator);
				if(ConfigManager.getInstance().get(loc.getRootLocator())!= null){	
					loc.setSessioncontext(header.getProperty(NamingUtil.session));
					OLAPAuthenticator.getInstance().authenticateComponent(loc, header,OLAPAuthenticator.roPermSet4);
				}

				if (ConfigManager.getInstance().removeElement(loc) != null)
					d.setResult("removed component "+locator);
				else
					d.setErrorMessage(locator +" does not exist.");
				results.add(d);
				logDescriptor(d,false);
			}
			log.debug("finished");
			return results.toArray(new ResultDescriptor[results.size()]);
		}
		catch (Exception e) {
			throw new AxisFault("Failed to remove components for "+getPrintable(locators)+": "+e.getMessage());
		}
	}


	/**
	 * get the metadata for a connection component. This can be applied to relational connections, file connections and connections of type Palo.
	 * In "settings" a selector has to be specified which defines which kind of metadata information should be retrieved in the result (e.g. catalogs of a relational connection).
	 * Beside the selector, filter criteria can be given in the settings array. The possible values of the selector and the filter criteria depend on the connection type. 
	 * For a list of possible values for each connection type see the ETL Server Manual.  
	 * @param locator the path to the connection components in the format {component.manager.}component: e.g: "myProject.connections.myPaloConnection".
	 * @param settings variables including selector and filter criteria. Selector is obligatory, filters are optional. 
	 * @param variables the variables forming the context of the execution in the form: name=value
	 * @return
	 */
	public ExecutionDescriptor getMetadata(String locator, Variable[] settings, Variable[] variables) {
		log.debug("getting metadata for "+getPrintable(locator));			
		Properties properties = getProperties(variables);
		Properties internalSettings = getProperties(settings);
		try {
			Locator loc = Locator.parse(locator);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			OLAPAuthenticator.getInstance().authenticateComponent(loc, header,OLAPAuthenticator.roPermSet2);
			addLoginParameters(properties, header);
			Execution e = Executor.getInstance().createMetadata(loc, properties, internalSettings);
			e.getExecutionState().setDataTarget(DataTargets.CSV_INLINE);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getDescriptor(Executor.getInstance().getExecutionState(e.getKey(), true));
			log.debug("finished");
			return d;
		}
		catch (Exception e) {
			return getErrorDescriptor("Failed to retrieve data for "+getPrintable(locator)+": "+e.getMessage());
		}
	}

	/**
	 * Uploads a file to the internal data repository.
	 * @param name the name of the file to write to
	 * @param data the data to write into the file.
	 * @return a {@link ResultDescriptor}. Result is "added file [name]" or "replaced file [name]" if successful.
	 */
	public ResultDescriptor uploadFile(String name, byte[] data) {
		log.debug("Upload data file " + name);
		ResultDescriptor d = new ResultDescriptor();
		String dir = Settings.getInstance().getDataDir();
		File file = new File(dir+File.separator+name);
		try {
			OLAPAuthenticator.getInstance().checkRightObjects(getHeaderInfo(), OLAPAuthenticator.roPermSet5);
			if (file.exists())
				d.setResult("replaced file "+name);
			else
				d.setResult("added file "+name);
			FileOutputStream stream = new FileOutputStream(file);
			stream.write(data);
			stream.close();
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to upload file "+name+": "+e.getMessage());
		}
		logDescriptor(d,false);
		log.debug("finished");
		return d;
	}


	/**
	 * executes a bulk drillthrough call on a given drillthrough datastore based on a (names,values) filter criterion. The size of values has to a multiple of the size of names, each multiple forming its own condition.
	 * @param datastore the name of the drillthrough datastore to perform the drillthrough for. The name is in the form 'Palo-DB-Name'.'Palo-Cube-Name' in upper case.
	 * @param names for each request in the bulk the names of the columns to filter.
	 * @param values for each request in the bulk the values, the cells of the filtered columns have to have, also in one dimensional array, but not all paths.
	 * @param lines the maximum number of lines to be returned. 0 return all lines.
	 * @return a {@link ResultDescriptor}. Result contains matching data as csv.
	 * @throws AxisFault
	 */
	public ResultDescriptor drillThrough(String datastore, String[] names, String[] values,int[] lengths,  int lines) throws AxisFault {
		log.debug("Starting drillthrough on datastore "+datastore);
		ResultDescriptor d = new ResultDescriptor();
		try {
			StringWriter out = new StringWriter();
			if (lines == 0)
				lines = Integer.MAX_VALUE;

			Properties header = getHeaderInfo();
			OLAPAuthenticator.getInstance().checkRightObjects(header, OLAPAuthenticator.roPermSet6);
			//String database = datastore.substring(0,datastore.indexOf("."));
			//String cube = datastore.substring(datastore.indexOf(".")+1,datastore.length());
			//OLAPAuthenticator.getInstance().drillthoughCheck(database,cube,header);
			Datastore ds = DatastoreManager.getInstance().get(datastore);
			if (ds == null) {
				// Datastore names in upper case since ETL 3.2. old names still found
				ds = DatastoreManager.getInstance().get(datastore.toUpperCase());
			}
			if (ds != null) {
				CSVWriter writer = new CSVWriter(out);
				writer.setAutoClose(false); //do not close on end of write, since we may use multiple writes due bulk mode

				if (lines-writer.getLinesOut() > 0) {
					try {
						IProcessor processor = ds.getFilteredProcessor(names, values,lengths,lines-writer.getLinesOut());
						writer.write(processor);
					}
					catch (RuntimeException ce) {
						d.setErrorMessage("Error in Drillthrough for "+datastore+": " + ce.getMessage());
					}
				}
				writer.close();
				ds.close();
				d.setResult(out.toString());				
			}
			else {
				String message = "No Drillthrough defined for "+datastore;
				log.error(message);
				d.setErrorMessage(message);
			}
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to execute method drillThrough for "+datastore+": "+e.getMessage());
		}
		logDescriptor(d,true);
		log.debug("finished");
		return d;
	}


	/** gets information about the location of the drillthrough datastore 
	 * @param datastore the name of the drillthrough datastore. The name is in the form 'Palo-DB-Name'.'Palo-Cube-Name' in upper casename the name of the executables - null for no filter.
	 * @return the drillthrough info containing Connector type, Connector, Schema name and Table name
	 * @throws AxisFault
	 */
	public DrillthroughInfoDescriptor[] drillThroughInfo(String datastore) throws AxisFault {
		log.debug("Starting drillThroughInfo for datastore "+datastore);
		try {
			OLAPAuthenticator.getInstance().checkRightObjects(getHeaderInfo(), OLAPAuthenticator.roPermSet6);
			ArrayList<DrillthroughInfoDescriptor> result = new ArrayList<DrillthroughInfoDescriptor>();
			for (String d : DatastoreManager.getInstance().getKeys()) {
				Datastore ds = DatastoreManager.getInstance().get(d);
				if (!ds.getConnector().equals("Temporary") && (datastore==null || ds.getLocator().equals(datastore))) { 
					DrillthroughInfoDescriptor dsdescr = new DrillthroughInfoDescriptor();
					dsdescr.setConnector(ds.getConnector());
					dsdescr.setDatastore(ds.getLocator());
					dsdescr.setSchemaname(ds.getSchemaName());
					dsdescr.setTablename(ds.getTableName());
					dsdescr.setConnectorType(ds.getConnectorType().toString());
					result.add(dsdescr);
				}
			}
			return result.toArray(new DrillthroughInfoDescriptor[result.size()]);
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method drillThroughInfo: "+e.getMessage());
		}
	}



	//******************************* R U N T I M E ******************************

	/**
	 * adds an execution. Only allowed for components implementing the IExecutable Interface (e.g. jobs and loads)
	 * @param locator the path to the component in the format {component.manager.}component: e.g: "myProject.jobs.default".
	 * @param variables the variables forming the context of the execution in the form: name=value
	 * @return an {@link ExecutionDescriptor} holding the id of the execution
	 * @throws AxisFault
	 */
	public ExecutionDescriptor addExecution(String locator, Variable[] variables) throws AxisFault {
		log.debug("adding executions for: "+getPrintable(locator));
		try {
			//build context properties
			Properties properties = getProperties(variables);
			ExecutionDescriptor d = null;
			Properties header = getHeaderInfo();
			Locator loc=null;
			try {
				loc = Locator.parse(locator);
				loc.setSessioncontext(header.getProperty(NamingUtil.session));
				addLoginParameters(properties, header);
				OLAPAuthenticator.getInstance().authenticateComponent(loc, header,OLAPAuthenticator.roPermSet2);
				Execution e = Executor.getInstance().createExecution(loc,properties);
				Executor.getInstance().addExecution(e);
				d = getDescriptor(e.getExecutionState());
			}
			catch (Exception e) {
				d = getErrorDescriptor("Failed to execute "+((loc==null)?locator:loc.getDisplayName())+": "+e.getMessage());
			}
			log.debug("finished");
			return d;
		}
		catch (Exception e) {
			throw new AxisFault(e.getMessage());
		}
	}

	/**
	 * removes executions. Only allowed for components implementing the IExecutable Interface (e.g. jobs and loads)
	 * @param ids array of the Executions IDs
	 * @return for each execution an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor[] removeExecutions(Long[] ids) {
		log.debug("removing executions");
		ExecutionDescriptor[] result = new ExecutionDescriptor[ids.length];
		for (int i=0; i<ids.length; i++) {
			Long id = ids[i];
			try {
				ExecutionState state = Executor.getInstance().removeExecution(id,true);
				if (state != null){
					OLAPAuthenticator.getInstance().authenticateComponent(Locator.parse(state.getProject()), getHeaderInfo(), OLAPAuthenticator.roPermSet4);
					result[i] = getDescriptor(state);
				}
				else {
					result[i] = getErrorDescriptor("Failed to remove execution "+id+": Execution not found");
				}
			}
			catch (ExecutionException e) {
				result[i] = getErrorDescriptor("Failed to remove execution "+id+": "+e.getMessage());
			} catch (ConfigurationException e) {
				result[i] = getErrorDescriptor("Failed to remove execution "+id+": "+e.getMessage());
			}
			logDescriptor(result[i],false);
		}
		log.debug("finished");
		return result;
	}

	/**
	 * runs an existing execution indicated by an execution id.
	 * @param id of the execution.
	 * @return an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor runExecution(Long id) {
		log.debug("running execution "+id);
		ExecutionDescriptor descriptor;
		try {
			ExecutionState state = Executor.getInstance().getExecutionState(id, false);
			Locator loc = Locator.parse(state.getProject());
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet2);
			descriptor = getDescriptor(Executor.getInstance().runExecution(id).getExecutionState());
		}
		catch (ExecutionException ee) {
			descriptor = getErrorDescriptor("Failed to run execution "+id+": "+ee.getMessage());
		} catch (ConfigurationException e) {
			descriptor = getErrorDescriptor("Failed to run execution "+id+": no sufficient rights are found for the given user.");
		}
		log.debug("finished");
		return descriptor;
	}

	/** 
	 * adds an execution and runs it immediately
	 * @param locator the path to the component in the format {component.manager.}component: e.g: "myProject.jobs.default".
	 * @param variables the variables forming the context of the execution in the form: name=value
	 * @return an {@link ExecutionDescriptor} holding the id of the execution
	 * @throws AxisFault
	 */
	public ExecutionDescriptor execute(String locator, Variable[] variables) throws AxisFault {
		ExecutionDescriptor descriptor = addExecution(locator,variables);
		if (!descriptor.getStatusCode().equals("0")) {
			log.error(descriptor.getErrorMessage());
		}
		else {	
			log.debug("running directly execution "+descriptor.getId());
			try {
				ExecutionState state = Executor.getInstance().getExecutionState( descriptor.getId(), false);
				Locator loc = Locator.parse(state.getProject());
				OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet2);
				Executor.getInstance().runExecution(descriptor.getId());
			}
			catch (ExecutionException ee) {
				descriptor = getErrorDescriptor("Failed to run execution "+descriptor.getId()+": "+ee.getMessage());
			} catch (ConfigurationException e) {
				descriptor = getErrorDescriptor("Failed to run execution "+descriptor.getId()+": no sufficient rights are found for the given user.");
			}
		}	
		log.debug("finished");
		return descriptor;		
	}
	
	
	/**
	 * stops an existing execution indicated by an execution id.
	 * @param id of the execution.
	 * @return an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor stopExecution(Long id) {
		log.debug("stopping execution: "+id);
		try {
			ExecutionDescriptor ed = new ExecutionDescriptor();
			if (id == -1) { //stop all active executions
				String activeExecutions = "";
				for (ExecutionState s : Executor.getInstance().getUnfinishedExecutions()) {					
					try {
						Locator loc = Locator.parse(s.getProject());
						OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet2);
					} catch (Exception e) {
						continue;
					}
					activeExecutions = activeExecutions.concat(s.getId() + ", ");
					Executor.getInstance().stop(s.getId());
				}
				if(!activeExecutions.equals(""))
					activeExecutions = activeExecutions.substring(0, activeExecutions.length()-2);
				if(activeExecutions.length() != 0){
					ed.setResult("The following executions have been killed: " + activeExecutions  + ".");
				}else{
					ed.setResult("No active executions are on the server.");
				}
			}
			else {
				ExecutionState state = Executor.getInstance().getExecutionState(id, false);
				Locator loc = Locator.parse(state.getProject());
				if(state.getStatus().equals(Codes.statusStopping) || state.getStatus().equals(Codes.statusRunning) || state.getStatus().equals(Codes.statusQueued)){
					OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet2);
					state = Executor.getInstance().stop(id);
					ed=getDescriptor(state);
				}else{
					throw new RuntimeException("Execution has status \"" + new ResultCodes().getString(state.getStatus()) + "\" and therefore can not be stopped.");
				}
				
			}
			logDescriptor(ed,false);
			return ed; //empty descriptor with the list of stopped or aborted executions (if existed)			
		}
		catch (Exception e) {
			return getErrorDescriptor("Failed to stop execution "+id+": "+e.getMessage());
		}
	}

	/**
	 * get the status of an existing execution indicated by an execution id.
	 * @param id of the execution.
	 * @param waitForTermination indicates wither the result should be handed only at the end of the execution.
	 * @return an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor getExecutionStatus(Long id, boolean waitForTermination) throws AxisFault {
		ExecutionDescriptor d = new ExecutionDescriptor();
		try {
			log.debug("getting status for execution: "+id);
			try {
				ExecutionState state = Executor.getInstance().getExecutionState(id, waitForTermination);
				d = getDescriptor(state);
				Locator loc = Locator.parse(state.getProject());
				OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet1);
				log.debug("finished");
				return d;
			}
			catch (ExecutionException e) {
				return getErrorDescriptor("Failed to get status for execution "+id+": "+e.getMessage());
			}
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getExecutionStatus for execution "+id+": "+e.getMessage());
		}
	}

	/**
	 * gets the (filtered) histories of executions, this includes all possible executions
	 * @param project the name of the project as filter criterion - null for no filter
	 * @param type the type of executables (jobs, loads, transforms, extracts) - null for no filter
	 * @param name the name of the executables - null for no filter.
	 * @param after a start date timestamp as filter criterion - 0 for no filter
	 * @param before a end date timestamp as filter criterion - 0 for no filter
	 * @param status the status of the execution as filter criterion - null for no filter
	 * @return an array of {@link ExecutionDescriptor ExecutionDescriptors} matching the query
	 */
	@Deprecated
	public ExecutionDescriptor[] getExecutionHistory(String project, String type, String name, long after, long before, String status) throws AxisFault {
		String[] types = null;
		String[] statuses = null;
		if(type!=null) {
			types = new String[1];
			types[0]=type;
		}	
		if(status!=null) {
			statuses = new String[1];
			statuses[0]=status;
		}	
		return getExecutionList(project, types, name, after, before, statuses, null);
	}
	
	private Date getDate(long timestamp) {
		return timestamp == 0 ? null : new Date(timestamp);
	}
	
	/**
	 * gets the (filtered) histories of executions, this includes all possible executions
	 * @param project the name of the project as filter criterion - null for no filter
	 * @param types array of type of executables (jobs, loads, transforms, extracts) - null for no filter
	 * @param name the name of the executables - null for no filter.
	 * @param after a start date timestamp as filter criterion - 0 for no filter
	 * @param before a end date timestamp as filter criterion - 0 for no filter
	 * @param statuses array of statuses of the execution as filter criterion - null for no filter
	 * @return an array of {@link ExecutionDescriptor ExecutionDescriptors} matching the query
	 */
	public ExecutionDescriptor[] getExecutionListPaged(String project, String[] types, String name, long after, long before, String[] statuses, String[] executiontypes, int start, int pagesize) throws AxisFault {
		log.debug("Getting execution history.");
		try {
			log.debug("Project:"+project+" Name:"+name+" After:"+after+" Before:"+before+" Types: "+getPrintable(types," ")+" Stati: "+getPrintable(statuses," ")+" Executiontypes: "+getPrintable(executiontypes," ")+".");
			if (start!=0 || pagesize!=0)
				log.debug("Start: "+start+" Pagesize: "+pagesize);
			
			List<ExecutionState> states = new ArrayList<ExecutionState>();
			if(project == null || project.isEmpty()){
				ArrayList<String> projects = OLAPAuthenticator.getInstance().getAuthenticatedComponents(ETL_Element_Types.P,getHeaderInfo(),OLAPAuthenticator.roPermSet1);
				if(projects!=null){
					for(String proj:projects)
						states.addAll(StateManager.getInstance().getResults(proj, types, name, getDate(after), getDate(before), statuses, executiontypes, start, pagesize, SortDirection.desc));
				}else{
					states = StateManager.getInstance().getResults(project, types, name, getDate(after), getDate(before), statuses, executiontypes, start, pagesize, SortDirection.desc);
				}
			}else{
				OLAPAuthenticator.getInstance().authenticateComponent(Locator.parse(project),getHeaderInfo(),OLAPAuthenticator.roPermSet1);
				states = StateManager.getInstance().getResults(project, types, name, getDate(after), getDate(before), statuses, executiontypes, start, pagesize, SortDirection.desc);
			}
			ExecutionDescriptor[] result = new ExecutionDescriptor[states.size()];
			for (int i=0; i<states.size(); i++) {
				result[i] = getDescriptor(states.get(i));
			}
			log.debug("Retrieved exections: "+states.size());
			log.debug("finished");
			return result;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getExecutionListPaged for project "+project+": "+e.getMessage());
		}
	}
	
	private long getExecutionListPagedCount(String project, String[] types, String name, long after, long before, String[] statuses, String[] executiontypes, int start, int pagesize) throws AxisFault {
		log.debug("Getting execution list count");
		try {
			log.debug("Project:"+project+" Name:"+name+" After:"+after+" Before:"+before+" Types: "+getPrintable(types," ")+" Stati: "+getPrintable(statuses," ")+" Executiontypes: "+getPrintable(executiontypes," ")+".");
			if (start!=0 || pagesize!=0)
				log.debug("Start: "+start+" Pagesize: "+pagesize);
			
			Long statesCount = Long.valueOf("0");
			if(project == null || project.isEmpty()){
				ArrayList<String> projects = OLAPAuthenticator.getInstance().getAuthenticatedComponents(ETL_Element_Types.P,getHeaderInfo(),OLAPAuthenticator.roPermSet1);
				if(projects!=null){
					for(String proj:projects)
						statesCount +=(StateManager.getInstance().getResultsCount(proj, types, name, getDate(after), getDate(before), statuses, executiontypes, start, pagesize));
				}else{
					statesCount += StateManager.getInstance().getResultsCount(project, types, name, getDate(after), getDate(before), statuses, executiontypes, start, pagesize);
				}
			}else{
				OLAPAuthenticator.getInstance().authenticateComponent(Locator.parse(project),getHeaderInfo(),OLAPAuthenticator.roPermSet1);
				statesCount += StateManager.getInstance().getResultsCount(project, types, name, getDate(after), getDate(before), statuses, executiontypes, start, pagesize);
			}
			
			log.debug("finished");
			return statesCount;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getExecutionListPagedCount for project "+project+": "+e.getMessage());
		}
	}
	
	public Long getExecutionListCount(String project, String[] types, String name, long after, long before, String[] statuses, String[] executiontypes) throws AxisFault {
		return getExecutionListPagedCount(project,types, name, after, before, statuses, executiontypes, 0,0);
	}
	
	/**
	 * gets the (filtered) histories of executions, this includes all possible executions
	 * @param project the name of the project as filter criterion - null for no filter
	 * @param types array of type of executables (jobs, loads, transforms, extracts) - null for no filter
	 * @param name the name of the executables - null for no filter.
	 * @param after a start date timestamp as filter criterion - 0 for no filter
	 * @param before a end date timestamp as filter criterion - 0 for no filter
	 * @param statuses array of statuses of the execution as filter criterion - null for no filter
	 * @return an array of {@link ExecutionDescriptor ExecutionDescriptors} matching the query
	 */
	public ExecutionDescriptor[] getExecutionList(String project, String[] types, String name, long after, long before, String[] statuses, String[] executiontypes) throws AxisFault {
		return getExecutionListPaged(project,types, name, after, before, statuses, executiontypes, 0,0);
	}
	
	/**
	 * gets the number of messages in an execution state with the given filter
	 * @param id the id of the execution.
	 * @param timestamp the end time point where the log after this time stamp will be ignored in the result
	 * @return a {@link ResultDescriptor}. Result contains the log.
	 */
	public ResultDescriptor getExecutionLogCount(Long id, String type, Long timestamp) throws AxisFault {
		log.debug("Getting execution log count for execution id: " + id);
		ResultDescriptor d = new ResultDescriptor();
		try {
			ExecutionState state  = Executor.getInstance().getExecutionState(id,false);
			Locator loc = Locator.parse(state.getProject());
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet1);
			d.setResult(String.valueOf(StateManager.getInstance().getMessagesCount(state, type, timestamp)));
			logDescriptor(d,true);
			log.debug("finished");
			return d;
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to get log count: "+e.getMessage());
			throw new AxisFault("Failed to execute method getExecutionLogCount for execution "+id+": "+e.getMessage());
		}
	}
	
	/**
	 * gets the log of an execution
	 * @param id the id of the execution.
	 * @param timestamp the end time point where the log after this time stamp will be ignored in the result
	 * @return a {@link ResultDescriptor}. Result contains the log.
	 */
	public ResultDescriptor getExecutionLog(Long id, String type, Long timestamp) throws AxisFault {
		return getExecutionLogPaged(id,type,timestamp,0,0);
	}

	/**
	 * gets the log of an execution
	 * @param id the id of the execution.
	 * @param timestamp the end time point where the log after this time stamp will be ignored in the result
	 * @return a {@link ResultDescriptor}. Result contains the log.
	 */
	public ResultDescriptor getExecutionLogPaged(Long id, String type, Long timestamp, int start, int pagesize) throws AxisFault {
		log.debug("Getting execution log for execution id: " + id);
		ResultDescriptor d = new ResultDescriptor();
		try {
			ExecutionState state  = Executor.getInstance().getExecutionState(id,false);
			Locator loc = Locator.parse(state.getProject());
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(), OLAPAuthenticator.roPermSet1);
			d.setResult(StateManager.getInstance().getMessagesText(state, type, timestamp,start,pagesize));
			logDescriptor(d,true);
			log.debug("finished");
			return d;
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to get log: "+e.getMessage());
			throw new AxisFault("Failed to execute method getExecutionLog for execution "+id+": "+e.getMessage());
		}
	}
	
	/**
	 * gets the execution details of an execution
	 * @param id the id of the execution.
	 * @return a {@link ExecutionDetailDescriptor}. Result contains the performance details per component and a svg representation of this.
	 */
	public ExecutionDetailDescriptor getExecutionDetails(Long id, Variable[] settings) throws AxisFault {
		log.debug("Getting execution performance monitor details for execution id: " + id);
		try {
			List<ExecutionDetail> details = StateManager.getInstance().getExecutionDetails(id);
			ExecutionDetailEntry[] results = new ExecutionDetailEntry[details.size()];
			for (int i=0; i<details.size(); i++) {
				results[i] = getDetailDescriptor(details.get(i));
			}
			ExecutionDetailDescriptor desc = new ExecutionDetailDescriptor();
			desc.setExecutionDetailentries(results);
			
			//calculate the graph
			ExecutionState state  = Executor.getInstance().getExecutionState(id,true);
			Locator loc = Locator.parse(state.getProject());			
			desc.setSvgGraph("");
			// Check if the project exists
			if(ConfigManager.getInstance().findElement(loc)!=null) {
				IProject project = (IProject) ConfigManager.getInstance().getProject(loc.getRootLocator().getName());
				List<Locator> invalidComponents = ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true);
				if(!state.getType().equals("sources")) {
					String locator = state.getProject()+"."+state.getType()+"."+ state.getName();
					if(ConfigManager.getInstance().findElement(Locator.parse(locator))!=null){
						desc.setSvgGraph(GraphManager.getInstance().getSVG(project, locator, getProperties(settings), invalidComponents,id));
					}	
				} else {
					// Data previews are stored with "sources" as locator. Find corresponding extract or transform 					
					String locator = state.getProject()+".extracts."+ state.getName();
					if(ConfigManager.getInstance().findElement(Locator.parse(locator))==null){
						locator = state.getProject()+".transforms."+ state.getName();
					}
					if(ConfigManager.getInstance().findElement(Locator.parse(locator))!=null){
						desc.setSvgGraph(GraphManager.getInstance().getSVG(project, locator, getProperties(settings), invalidComponents,id));
					}
				}
			}
			log.debug("finished");
			return desc;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getExecutionDetails for execution "+id+": "+e.getMessage());
		}
	}

	/**
	 * prepares, waits for and fetches data of the component.
	 * @param locator the path to the component in the format component{.manager.component}: e.g: "myProject.sources.mySource". Must not be null.
	 * @param variables the variables used to to define a runtime context.
	 * @param view the view of the data. relevant only for components providing a tree data representation
	 * @param lines number of line of the data to be delivered
	 * @return an {@link ExecutionDescriptor} holding the data in its result property.
	 * @throws AxisFault
	 */
	public ExecutionDescriptor getData(String locator, Variable[] variables, String view, int lines, int start, Boolean waitForTermination, String outputFormat) throws AxisFault {
		log.debug("retrieving data for "+getPrintable(locator));	
		if (waitForTermination == null)
			waitForTermination = Boolean.TRUE;
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			OLAPAuthenticator.getInstance().authenticateComponent(loc, header,OLAPAuthenticator.roPermSet2);
			addLoginParameters(properties, header);
			Execution e;
			try {
				e = Executor.getInstance().createData(loc, properties,view);
			}
			catch (Exception ex) {
				// Validation error during initialization phase
				return getErrorDescriptor(ex.getMessage());
			}
			e.getExecutable().getParameter().setProperty("sample", String.valueOf(lines));
			start = handleOffset(start);
			
			e.getExecutable().getParameter().setProperty("offset", String.valueOf(start));
			if(outputFormat==null || outputFormat.equalsIgnoreCase("csv"))
				e.getExecutionState().setDataTarget(DataTargets.CSV_INLINE);
			else if(outputFormat.equalsIgnoreCase("xml"))
				e.getExecutionState().setDataTarget(DataTargets.XML_INLINE);
			else
				throw new Exception("OutputFormat can only be csv or xml");
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getDescriptor(Executor.getInstance().getExecutionState(e.getKey(), waitForTermination));
			log.debug("finished");
			return d;
		}
		catch (Exception ex) {
			return getErrorDescriptor("Failed to retrieve data for "+getPrintable(locator)+": "+ex.getMessage());
		}
	}

	/**
	 * *Experimental* Prepares data from a single component in the internal persistence. Only allowed for components implementing the ISource Interface. (sources and serializers for now)
	 * @param locator the path to the component in the format component{.manager.component}: e.g: "myProject.sources.mySource". Must not be null.
	 * @param variables variables if existed
	 * @param view the view of the data
	 * @param lines number of lines of the data to be delivered
	 * @return an {@link ExecutionDescriptor} holding the datastore id in its result property.
	 * @throws AxisFault
	 */
	@SuppressWarnings("unused")
	private ExecutionDescriptor prepareData(String locator, Variable[] variables, String view, int lines) {
		log.debug("preparing data for: "+getPrintable(locator));
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
			Properties header = getHeaderInfo();
			//OLAPAuthenticator.getInstance().validate(loc, header,ElementPermission.R,OLAPAuthenticator.roPermSet2);
			addLoginParameters(properties, header);
			Execution e;
			try {
				e = Executor.getInstance().createData(loc, properties,view);
			}
			catch (Exception ex) {
				// Validation error during initialization phase
				return getErrorDescriptor(ex.getMessage());
			}
			e.getExecutable().getParameter().setProperty("sample", String.valueOf(lines));
			e.getExecutionState().setDataTarget(DataTargets.CSV_PERSISTENCE);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			String target = Executor.getInstance().getExecutionState(e.getKey(), false).getData();
			log.debug("Data is streamed internally to: "+target);
			return getDescriptor(e.getExecutionState());
		}
		catch (Exception ex) {
			return getErrorDescriptor("Failed to prepare data for "+getPrintable(locator)+": "+ex.getMessage());
		}
	}

	/**
	 * *Experimental* fetches data from a datastore from the internal persistence. The datastore needs to be created via {@link #prepareData(String, Variable[], String, int)} first.
	 * @param datastore the name of the datastore th fetch from
	 * @param offset offset
	 * @param lines number of lines of the data to be delivered
	 * @return a {@link ResultDescriptor}. Result contains the data
	 * @throws AxisFault
	 */
	@SuppressWarnings("unused")
	private ResultDescriptor fetchData(String datastore, int start, int lines) throws AxisFault {
		log.debug("Getting data from datastore "+datastore);
		//change this to pagination without filtering!!
		ResultDescriptor d = new ResultDescriptor();
		String errorPrefix = "Error in getting Data from Datastore "+datastore+" ";
		try {
			StringWriter out = new StringWriter();
			CSVWriter writer = new CSVWriter(out);
			Datastore ds = DatastoreManager.getInstance().get(datastore);
			if (ds != null) {
				start = handleOffset(start);
				IProcessor processor = ds.getProcessor("select * from "+NamingUtil.internalDatastoreName()+" where "+ds.escapeName(NamingUtil.internalKeyName())+">="+start,lines);
				//processor.setLastRow(lines);
				writer.write(processor);
			}
			else {
				String message = "Datastore "+datastore+" not found.";
				log.error(message);
				d.setErrorMessage(errorPrefix + message);
			}
			d.setResult(out.toString());
		}
		catch (Exception e) {
			d.setErrorMessage(errorPrefix+e.getMessage());
		}
		logDescriptor(d,true);
		log.debug("finished");		
		return d;
	}


	/**
	 * Gets the output description of a component.
	 * @param locator locator the path to the component in the format component{.manager.component}: e.g: "myProject.sources.mySource". Must not be null.
	 * @param variables the variables used to to define a runtime context.
	 * @param view the view of the data. relevant only for components providing a tree data representation
	 * @return an {@link ExecutionDescriptor} holding the ComponentOutputDescription in its metadata property.
	 * @throws AxisFault
	 */
	public ExecutionDescriptor getComponentOutputs(String locator, Variable[] variables, String view, Boolean waitForTermination) throws AxisFault {
		log.debug("getting component output for "+getPrintable(locator));		
		if (waitForTermination == null)
			waitForTermination = Boolean.TRUE;
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			OLAPAuthenticator.getInstance().authenticateComponent(loc, header,OLAPAuthenticator.roPermSet2);
			Execution e = Executor.getInstance().createOutputDescription(loc, properties,view);
			e.getExecutionState().setDataTarget(DataTargets.CSV_INLINE);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getExecutionStatus(e.getKey(),waitForTermination);
			log.debug("finished");			
			return d;
		}
		catch (Exception e) {
			return getErrorDescriptor("Failed to retrieve component outputs for "+getPrintable(locator)+": "+e.getMessage());
		}
	}


	/**
	 * Tests the components in runtime
	 * @param locator the paths to the components in the format {component.manager.}component: e.g: "myProject.jobs.default".
	 * @param variables the variables used to to define a runtime context.
	 * @return an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor testComponent(String locator, Variable[] variables, Boolean waitForTermination) {
		log.debug("testing component "+getPrintable(locator));					
		if (waitForTermination == null)
			waitForTermination = Boolean.TRUE;
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
			Properties header = getHeaderInfo();
			loc.setSessioncontext(header.getProperty(NamingUtil.session));
			OLAPAuthenticator.getInstance().authenticateComponent(loc, header,OLAPAuthenticator.roPermSet2);
			addLoginParameters(properties, header);
			Execution e;
			try {
				e = Executor.getInstance().createTest(loc, properties);
			}
			catch (Exception ex) {
				// Validation error during initialization phase
				return getErrorDescriptor(ex.getMessage());
			}
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getExecutionStatus(e.getKey(),waitForTermination);
			log.debug("finished");
			return d;
		}
		catch (Exception ex) {
			return getErrorDescriptor("Failed to test "+getPrintable(locator)+": "+ex.getMessage());
		}
	}

	/**
	 * Gets the recursive list of all ingoing dependent components of a component e.g. all loads of a job
	 * @param locator the path to the component in the format component.manager.component: e.g: "myProject.jobs.default".
	 * @param includeVariables if set dependencies to project variables are also returned
	 * @return the list of all components from which the input-component is dependent
	 * @throws AxisFault
	 */
	public ComponentDependencyDescriptor[] getComponentDependencies(String locator, boolean includeVariables) throws AxisFault {
		log.debug("getting component dependencies for "+getPrintable(locator));		
		try {			
			Locator loc = Locator.parse(locator);
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			// if it is a variable then it is empty
			if(loc.getManager().equals(ITypes.Managers.variables.toString())){
				ComponentDependencyDescriptor empty= new ComponentDependencyDescriptor();
				empty.setName(loc.toString());
				empty.setComponents(new String[0]);
				return new ComponentDependencyDescriptor[]{empty};
			}
			IProject project = (IProject) ConfigManager.getInstance().getProject(loc.getRootName());
			ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true); 
			Map<String,List<String>> deps = project.getAllDependencies(locator,includeVariables);
			ComponentDependencyDescriptor[] result = new ComponentDependencyDescriptor[deps.keySet().size()];
			Iterator<String> iterator = deps.keySet().iterator();
			int i = 0;
			while (iterator.hasNext()) {
				String key = iterator.next();
				List<String> line = deps.get(key);
				result[i] = new ComponentDependencyDescriptor();
				result[i].setName(key);
				result[i].setComponents(line.toArray(new String[line.size()]));
				i++;
			}
			log.debug("finished");
			return result;
		}
		catch (Exception e) {
			ComponentDependencyDescriptor notValidCD=getErrorDependencyDescriptor(locator,"Failed to get dependencies for "+locator+": "+e.getMessage());
			return new ComponentDependencyDescriptor[]{notValidCD};
		}
	}

	/**
	 * Gets the list of all directly ingoing dependent components of a component e.g. all loads of a job
	 * @param locator the path to the component in the format component.manager.component: e.g: "myProject.jobs.default".
	 * @param includeVariables if set dependencies to project variables are also returned
	 * @return the list of all components from which the input-component is dependent
	 * @throws AxisFault
	 */
	public ComponentDependencyDescriptor getComponentDirectDependencies(String locator, boolean includeVariables) throws AxisFault {
		log.debug("getting direct component dependencies for "+getPrintable(locator));						
		try {
			Locator loc = Locator.parse(locator);
			ComponentDependencyDescriptor result = new ComponentDependencyDescriptor();
			result.setName(locator);
			result.setComponents(new String[0]);			
			if(loc.getManager().equals(ITypes.Managers.variables.toString()))
				return result;
			
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			IProject project = (IProject) ConfigManager.getInstance().getProject(loc.getRootName());
			ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true); 
			List<String> line = project.getDirectDependencies(locator,includeVariables);	
			result.setComponents(line.toArray(new String[line.size()]));
			log.debug("finished");
			return result;
		}
		catch (Exception e) {
			return getErrorDependencyDescriptor(locator,"Failed to get direct dependencies for "+locator+": "+e.getMessage());
		}
	}
	
	
	/**
	 * Gets the recursive list of all outgoing components of a component e.g. all jobs which include a load  
	 * @param locator the path to the component in the format component.manager.component: e.g: "myProject.jobs.default".
	 * @return the list of all components which depend on the input-component
	 * @throws AxisFault
	 */
	public ComponentDependencyDescriptor[] getComponentDependents(String locator) throws AxisFault {
		log.debug("getting component dependents for "+getPrintable(locator));				
		try {
			Locator loc = Locator.parse(locator);
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			IProject project = (IProject) ConfigManager.getInstance().getProject(loc.getRootName());
			ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true); 
			Map<String,List<String>> deps = null;
			if(loc.getManager().equals(ITypes.Managers.variables.toString()))
				deps = project.getAllDependents(locator,true);
			else
				deps = project.getAllDependents(locator,false);
			ComponentDependencyDescriptor[] result = new ComponentDependencyDescriptor[deps.keySet().size()];
			Iterator<String> iterator = deps.keySet().iterator();
			int i = 0;
			while (iterator.hasNext()) {
				String key = iterator.next();
				List<String> line = deps.get(key);
				result[i] = new ComponentDependencyDescriptor();
				result[i].setName(key);
				result[i].setComponents(line.toArray(new String[line.size()]));
				i++;
			}
			log.debug("finished");
			return result;
		}
		catch (Exception e) {
			ComponentDependencyDescriptor notValidCD=getErrorDependencyDescriptor(locator,"Failed to get dependents for "+locator+": "+e.getMessage());
			return new ComponentDependencyDescriptor[]{notValidCD};
		}
	}

	/**
	 * Gets the list of all directly outgoing components of a component e.g. all jobs which include a load 
	 * @param locator the path to the component in the format component.manager.component: e.g: "myProject.jobs.default".
	 * @return the list of all components which depend on the input-component
	 * @throws AxisFault
	 */
	public ComponentDependencyDescriptor getComponentDirectDependents(String locator) throws AxisFault {
		log.debug("getting direct component dependents for "+getPrintable(locator));								
		try {
			Locator loc = Locator.parse(locator);
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			IProject project = (IProject) ConfigManager.getInstance().getProject(loc.getRootName());
			ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true); 
			List<String> line = null;
			if(loc.getManager().equals(ITypes.Managers.variables.toString()))
				line = project.getDirectDependents(locator,true);
			else
				line = project.getDirectDependents(locator,false);
			ComponentDependencyDescriptor result = new ComponentDependencyDescriptor();
			result.setName(locator);
			result.setComponents(line.toArray(new String[line.size()]));
			log.debug("finished");			
			return result;
		}
		catch (Exception e) {
			return getErrorDependencyDescriptor(locator,"Failed to get direct dependents for "+locator+": "+e.getMessage());
		}
	}
	
	
	public ResultDescriptor getProjectDocumentation(String locator, String[] graphLocators) throws AxisFault {
		log.debug("getting project documentation for "+getPrintable(locator));				
		try {
			ResultDescriptor d = new ResultDescriptor();
			String projectName = null;
			try {
				Locator loc = Locator.parse(locator);
				projectName = loc.getRootName();
				OLAPAuthenticator.getInstance().authenticateComponent(loc.getRootLocator(), getHeaderInfo(),OLAPAuthenticator.roPermSet1);
				IProject project = ConfigManager.getInstance().getProject(loc.getRootLocator().getName());					
				DocuUtil util = new DocuUtil(project, loc, graphLocators, null);
				d.setResult(util.getDocu());			
			}
			catch (ConfigurationException e) {
				String message="Could not generate documentation for project "+projectName+": "+e.getMessage();
				log.error(message);
				d.setErrorMessage(message);
				return d;
			}
			log.debug("finished");
			return d; 
		}	
		catch (Exception e) {
			e.printStackTrace();
			throw new AxisFault("Failed to get project documentation for "+locator+": "+e.getMessage());
		}
	}

	public ResultDescriptor calculateComponentGraph(String locator, Variable[] settings) {
		log.debug("calculating flow graph for "+getPrintable(locator));
		ResultDescriptor d = new ResultDescriptor();
		try {
			Locator loc = Locator.parse(locator);
			OLAPAuthenticator.getInstance().authenticateComponent(loc, getHeaderInfo(),OLAPAuthenticator.roPermSet1);
			IProject project;
			List<Locator> invalidComponents = new ArrayList<Locator>(); 
			// get project config 
			project = (IProject) ConfigManager.getInstance().getProject(loc.getRootLocator().getName());
			invalidComponents = ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true);
			// Check if component exists
			if(ConfigManager.getInstance().findElement(loc)==null)
				throw new Exception("Component " + loc.toString() + " does not exist.");

			String svg = GraphManager.getInstance().getSVG(project, locator, getProperties(settings), invalidComponents,Long.MIN_VALUE);
			d.setResult(svg);
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to calculate graph for "+locator+": "+e.getMessage());
			d.setValid(false);
			log.error(d.getErrorMessage());
		}
		log.debug("finished");
		return d;
	}

	/**
	 * gets the server current status, info like free memory,used memory,...
	 * @return the {@link ServerStatus}
	 * @throws AxisFault
	 */
	public ServerStatus getServerStatus() throws AxisFault {
		log.debug("getting server status");
		try{
			ServerStatus result = new ServerStatus();
			result.setVersion(Settings.getInstance().getVersion());
			result.setAutosaving(Settings.getInstance().getAutoSave());
			result.setValidating(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("validate","true").equalsIgnoreCase("true"));
			result.setLogLevel(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("loglevel","true"));
			result.setFreeMemory(Runtime.getRuntime().freeMemory());
			result.setMaxMemory(Runtime.getRuntime().maxMemory());
			result.setTotalMemory(Runtime.getRuntime().totalMemory());
			result.setProcessorsAvailable(Runtime.getRuntime().availableProcessors());
			result.setRunningExecutions(Executor.getInstance().getRunningExecutions().size());
			result.setDatastores(DatastoreManager.getInstance().getKeys());
			log.debug("finished");
			return result;
		}
		catch (Exception e) {
			throw new AxisFault(e.getMessage());
		}
	}


	/**
	 * gets the list of all formats available to be rendered by a tree (e.g. PCW for Parent-Child-Weight)
	 * @return array with the tree formats
	 */
	public String[] getTreeFormats() {
		log.debug("getting tree formats");
		ArrayList<String> treeFormats = new ArrayList<String>();
		for (IView.Views view : IView.Views.values()) {
			if (!view.equals(Views.NONE) && !view.equals(Views.FHWA))
				treeFormats.add(view.toString());
		}
		log.debug("finished");
		return treeFormats.toArray(new String[treeFormats.size()]);
	}	
	
	/**
	 * gets the list of all codes describing the status of an execution (e.g. numericCode 10, Description: Completed successfully)
	 * @return array with the execution status codes
	 */	
	public ExecutionStatusCode[] getExecutionStatusCodes() {
		log.debug("getting execution status codes");		
		List<ExecutionStatusCode> statusCodes = new ArrayList<ExecutionStatusCode>();
		ResultCodes resultCodes = new ResultCodes();
		for (ResultCodes.Codes resultCode : ResultCodes.Codes.values()) {			
			ExecutionStatusCode statusCode = new ExecutionStatusCode();
			statusCode.setNumericCode(Integer.parseInt(resultCodes.getNumeric(resultCode)));
			statusCode.setDescription(resultCodes.getString(resultCode));
			statusCodes.add(statusCode);
		}
		Collections.sort(statusCodes);
		log.debug("finished");						
		return statusCodes.toArray(new ExecutionStatusCode[statusCodes.size()]);
	}	
	
	
	@SuppressWarnings("unchecked")
	private Properties getHeaderInfo() throws ConfigurationException{

		Properties props = new Properties();
		if (MessageContext.getCurrentMessageContext() != null) {
			SOAPHeader header = MessageContext.getCurrentMessageContext().getEnvelope().getHeader();
			if(header!=null) {
				java.util.Iterator<Object> it = header.getChildrenWithLocalName(NamingUtil.etlsession); 
				if (it!=null && it.hasNext()) {
					OMElement etlsessionElement = (OMElement) it.next();
					if (etlsessionElement!=null) {
						String etlsession = etlsessionElement.getText();
						if (etlsession.trim().isEmpty())
							throw new ConfigurationException("Empty etlsession");
						String[] results =  decrypt(etlsession);
						props.put(NamingUtil.username, results[0]);
						props.put(NamingUtil.password, results[1]);
						props.put(NamingUtil.session, results[2]);
					}
				}
			}
		}
		return props;
	}
	
	private String[] decrypt(String etlsession){
		BasicTextEncryptor crypt = new BasicTextEncryptor();
		crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));
		String result =crypt.decrypt(etlsession);
		return result.split("\n");
	}
	
	/**
     * Generates an ETL project based on given parameters which can be used as a simple starting point for ETL modeling.
     * @param prototype Name of the prototype scenario. Currently available CSVToOlap and ExcelToOlap
     * @param projectname Name of the generated ETL project
     * @param parameters Name of the scenario specific parameter-value pairs.
     * @return Result contains the XML configuration of the generated ETL project. 
     * 
     * 	 */
	public ResultDescriptor generatePrototypeProject(String prototype, String projectname, Variable[] parameters) {
		log.debug("Generating prototype "+prototype+" for project "+projectname);
		ResultDescriptor d = new ResultDescriptor();
		try {
			String config = PrototypeGenerator.getInstance().generate(prototype, projectname, getProperties(parameters)); 
			d.setResult(config);
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to generate ETL project: "+e.getMessage());
			d.setValid(false);
			log.error(d.getErrorMessage());
		}
		log.debug("finished");
		return d;	
	}

	/**
     * Generates and adds an ETL project based on given parameters which can be used as a simple starting point for ETL modeling. The project is directly executed on the Server
     * @param prototype Name of the prototype scenario. Currently available CSVToOlap and ExcelToOlap
     * @param locator the path of the component to be executed in the generated ETL project {component.manager.}component: e.g: "myProject.jobs.default". If only project name is given job named default is used
     * @param parameters Name of the scenario specific parameter-value pairs.
     * @return a {@link ResultDescriptor ResultDescriptors}. Result is "added component [locator]" or "replaced component [locator]", if successful. 
     * 
     **/	
	public ExecutionDescriptor executePrototypeProject(String prototype, String locator, Variable[] parameters, boolean overwrite) throws AxisFault {
		try {
			Locator loc = Locator.parse(locator);
			if (loc.isRoot()) {
				loc.add("jobs").add("default");
			}	
			ResultDescriptor d = generatePrototypeProject(prototype, loc.getRootName(), parameters);
			if (!d.getValid()) {
				return getErrorDescriptor(d.getErrorMessage());	
			}
			String config = d.getResult();
			if (!overwrite && ConfigManager.getInstance().get(loc.getRootLocator()) != null) {
				return getErrorDescriptor("Project "+loc.getRootName()+" is already existing.");
			}
			d = addComponents(new String[]{loc.getRootName()}, new String[]{config})[0];		
			if (!d.getValid()) {
				return getErrorDescriptor(d.getErrorMessage());
			}	
			return execute(loc.toString(), null);
		}	
		catch (Exception e) {
			return getErrorDescriptor("Invalid locator "+locator);			
		}	
		
	}
	
}
