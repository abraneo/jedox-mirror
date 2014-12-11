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
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.io.FileOutputStream;
import java.io.File;

import org.jdom.Element;

import org.apache.axis2.AxisFault;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.ConfigConverter;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.ConfigValidator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.ExecutionException;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.execution.ResultCodes;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.StateManager;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ResultCodes.DataTargets;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.DatastoreManager;
import com.jedox.etl.core.persistence.hibernate.HibernateUtil;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.CSVWriter;
import com.jedox.etl.core.util.ClassUtil;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.docu.DocuUtil;
import com.jedox.etl.core.util.svg.GraphUtilFactory;
import com.jedox.etl.core.util.svg.IGraphUtil;

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
	private MetadataUtil meta = new MetadataUtil();

	public ETLService() {
		try {
			ConfigManager.getInstance();
			Settings.getInstance().getContext(Settings.PersistenceCtx).setProperty("mode", "server");
			new HibernateUtil();
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

	private String getPrintable(String[] locators) {
		StringBuffer result = new StringBuffer();
		if (locators == null) return "";
		for (String locator : locators) {
			if (locator == null)
				result.append("projects ");
			else
				result.append(locator+" ");
		}
		return result.toString();
	}

	private void logDescriptor(ResultDescriptor d, boolean errorsOnly) {
		if (d.getValid()) {
			if (!errorsOnly)
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
				d.setDefaultValue(column.getDefaultValue());
				d.setPosition(row.indexOf(column)+1);
				d.setRole(column.getColumnType().toString());
				d.setType(column.getValueType());
				d.setOriginalName(row.getOriginalName(row.indexOf(column)));
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
		d.setErrors(state.getErrors());
		d.setWarnings(state.getWarnings());
		d.setStartDate((state.getStartDate() == null) ? 0 : state.getStartDate().getTime());
		d.setStopDate((state.getStopDate() == null) ? 0 : state.getStopDate().getTime());
		d.setStatus(state.getString(state.getStatus()));
		d.setStatusCode(state.getNumeric(state.getStatus()));
		d.setErrorMessage(state.getFirstErrorMessage());
		d.setMetadata(getOutputDescription(state.getMetadata()));
		switch (state.getDataTarget()) {
		case INLINE:  d.setResult(state.getData()); break;
		case PERSISTENCE: d.setResult(state.getData()); break;
		default: d.setResult(null);
		}
		if (!Codes.statusInvalid.equals(state.getStatus()))
			d.setValid(true);
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

	// *********************** G E T  M E T H O D E S ********************

	/**
	 * gets the configuration of a set of components as XML.
	 * @param locators an array of paths to the components in the format {component.manager.}component: e.g: "myProject.connections.myConnection".
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result contains the XML configuration of the component.
	 */
	public ResultDescriptor[] getComponentConfigs(String[] locators) {
		log.info("getting component configs for: "+getNamesAsString(locators));
		ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
		for (String locator : locators) {
			ResultDescriptor d = new ResultDescriptor();
			try {
				d.setResult((ConfigManager.getInstance().getConfigurationString(Locator.parse(locator))));
			}
			catch (Exception e) {
				d.setErrorMessage(e.getMessage());
			}
			results.add(d);
			logDescriptor(d,true);
		}
		log.info("finished");
		return results.toArray(new ResultDescriptor[results.size()]);
	}

	/**
	 * gets the names of all children hosted by the component or manager denoted by the locator
	 * @param locator the path to a component or manager: e.g: "myProject.connections", "myProject.connections.myConnection". If null the root project manager is selected.
	 * @return an array of names
	 * @throws AxisFault
	 */
	public String[] getNames(String locator) throws AxisFault {
		try {
			log.info("getting locateable names for: "+getPrintable(locator));
			Locator loc = Locator.parse(locator);
			String[] names=null;
			try {
				if (!loc.isEmpty())
					ConfigManager.getInstance().validate(loc);
				names = ConfigManager.getInstance().getNames(loc);
			} catch (Exception e) {
				log.info("Validation error: "+e.getMessage());
			}
			log.info("finished");
			return names;
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
		try {
			log.info("getting children component locators for: "+getPrintable(locator));
			Locator loc = Locator.parse(locator);
			if (!loc.isEmpty())
				ConfigManager.getInstance().validate(loc);
			Locator[] locators = ConfigManager.getInstance().getLocators(loc);
			String[] locStrings = new String[locators.length];
			for (int i=0; i<locators.length; i++) {
				locStrings[i] = locators[i].toString();
			}
			log.info("finished");
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
		log.info("getting scopes");
		ArrayList<String> scopes = new ArrayList<String>();
		for (ITypes.Components c : ITypes.Components.values()) {
			scopes.add(c.toString()+"s");
		}
		log.info("finished");
		return scopes.toArray(new String[scopes.size()]);
	}

	/**
	 * gets the types registered for the given component scope
	 * @param scope the scope type name as registered in the component.xml. see {@link #getScopes()}
	 * @return the array of {#link ComponentTypeDescriptor ComponentTypeDescriptors} registered for this scope
	 * @throws AxisFault
	 */
	public ComponentTypeDescriptor[] getComponentTypes(String scope) throws AxisFault {
		log.info("getting component types descriptors for scope " + scope);
		ArrayList<ComponentTypeDescriptor> result = new ArrayList<ComponentTypeDescriptor>();
		List<ComponentDescriptor> descriptors = ComponentFactory.getInstance().getComponentDescriptors(scope);
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
			result.add(ce);
		}
		log.info("finished");
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
		try {
			log.info("validating component configs...");
			ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
			ConfigConverter converter = new ConfigConverter();
			
			for (int i=0; i<locators.length; i++) {
				ResultDescriptor d = new ResultDescriptor();
				if (configs == null || i >= configs.length || configs[i] == null ) { //assume that components are present
					try {
						ConfigManager.getInstance().validate(Locator.parse(locators[i]));
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
					ConfigManager.getInstance().add(converter,loc, config);
					try {
						ConfigManager.getInstance().validate(loc);
						d.setResult("OK");
					}
					catch (Exception e) {
						String errorMessage = e.getMessage().replaceAll(tempProjectName,oldRootName);
						d.setErrorMessage("Failed: "+ errorMessage);
					}
					finally {
						ConfigManager.getInstance().remove(loc.getRootLocator());
					}
				}
				results.add(d);
				logDescriptor(d,true);
			}
			log.info("finished");
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
		try {
			log.info("migrating component configs...");
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
			log.info("finished");
			return results.toArray(new ResultDescriptor[results.size()]);
		}
		catch (Exception e) {
			throw new AxisFault("Failed to migrate: "+e.getMessage());
		}
	}

	/**
	 * adds components to a given manger
	 * @param locators the paths of the new components in the format {component.manager.}component: e.g: "myProject.connections.myConnection". If null the root project manager is selected.
	 * @param configs the configuration XMLs of the components to add.
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result is "added component [locator]" or "replaced component [locator]", if successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] addComponents(String[] locators, String[] configs) throws AxisFault {
		if (locators == null) locators = new String[configs.length];
		log.info("adding components ...");
		if (locators.length != configs.length) {
			throw new AxisFault("Number of locators has to be identical to number of configs given.");
		}
		ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
		ConfigConverter converter = new ConfigConverter();
		for (int i=0; i<locators.length; i++) {
			String locator = locators[i];
			String config = configs[i];
			ResultDescriptor d = new ResultDescriptor();
			try {
				Element element = XMLUtil.stringTojdom(config);
				Locator loc = Locator.parse(locator);
				if (element.getChild("project")!=null )
					// Avoid adding of repository.xml
					throw new AxisFault("File is not a valid ETL-project, it can not be added.");					
				if (loc.isEmpty()) {
					//if locator is empty get correct name from config.
					loc = Locator.parse(element.getAttributeValue("name"));
					locator = loc.toString();
				} 
				else if (loc.isRoot()) {//if locator denotes a project set this name in config.
					element.setAttribute("name", loc.getName());
				}				
				Element e = ConfigManager.getInstance().add(converter,loc, element);
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
		log.info("finished");
		return results.toArray(new ResultDescriptor[results.size()]);
	}

	/**
	 * removes components
	 * @param locators the paths to the components in the format {component.manager.}component: e.g: "myProject.connections.myConnection".
	 * @return an array of {@link ResultDescriptor ResultDescriptors}. Result is "removed component [locator]", if successful.
	 * @throws AxisFault
	 */
	public ResultDescriptor[] removeComponents(String[] locators) throws AxisFault {
		try {
			log.info("removing components ...");
			ArrayList<ResultDescriptor> results = new ArrayList<ResultDescriptor>();
			for (String locator: locators) {
				ResultDescriptor d = new ResultDescriptor();
				if (ConfigManager.getInstance().remove(Locator.parse(locator)) != null)
					d.setResult("removed component "+locator);
				else
					d.setErrorMessage(locator +" does not exist.");
				results.add(d);
				logDescriptor(d,false);
			}
			log.info("finished");
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
		Properties properties = getProperties(variables);
		Properties internalSettings = getProperties(settings);
		try {
			Locator loc = Locator.parse(locator);
			Execution e = Executor.getInstance().createMetadata(loc, properties, internalSettings);
			e.getExecutionState().setDataTarget(DataTargets.INLINE);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getDescriptor(Executor.getInstance().getExecutionState(e.getKey(), true));
			return d;
		}
		catch (Exception e) {
			return getErrorDescriptor("Failed to retrieve data for "+getPrintable(locator)+": "+e.getMessage());
		}
		/*
		ResultDescriptor d = new ResultDescriptor();
		try {
			IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(locator),null);
			if (component instanceof IConnection) {
				d.setResult(((IConnection)component).getMetadata(properties));
			}
			else
				d.setErrorMessage(locator +" does not address a Connection.");
		}
		catch (Exception e) {
			if (e.getMessage().startsWith("Failed"))
				d.setErrorMessage(e.getMessage());
			else
				d.setErrorMessage("Failed to get Metadata for "+Locator.parse(locator).getName()+": "+e.getMessage());
		}
		logDescriptor(d,true);
		return d;
		*/
	}

	/**
	 * gets the database MataData from an existing (olap) connection.
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param mask a binary 1-bit filter criterion on the types of databases. "0" the exclude system database, "1" to include system databases
	 * @return a {@link ResultDescriptor}. Result contains database information from an OLAP-System as csv. ("Id"; "Name"; "isSystemDatabase")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapDatabases(String locator, String mask) {
		ResultDescriptor d = meta.getOlapDatabases(locator, mask);
		logDescriptor(d,true);
		return d;
	}

	/**
	 * get the cube MetaData from an existing (olap) connection
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param mask a binary 4-bit filter criterion on types of cubes. null for no filter. 1st bit = AttributeCubes, 2nd bit = SubSetCubes, 3rd bit = SystemCubes, 4th bit = ViewCube. Turn the bits on ("1") to include and off ("0") to exclude.
	 * @return a {@link ResultDescriptor}. Result contains cube information from an OLAP-System as csv. ("Id"; "Name"; "isAttributeCube"; "isSubSetCube"; "isSystemCube"; "isViewCube")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapCubes(String locator, String mask) {
		ResultDescriptor d = meta.getOlapCubes(locator, mask);
		logDescriptor(d,true);
		return d;
	}

	/**
	 * get the dimension MetaData from an existing (olap) connection
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param mask abinary 3-bit filter criterion on types of dimensions. null for no filter. 1st = AttributeDimensions, 2nd bit = SubSetDimensions, 3rd bit = isSystemDimension. Turn the bits on ("1") to include and off ("0") to exclude.
	 * @return a {@link ResultDescriptor}. Result contains dimension information from an OLAP-System as csv. ("Id"; "Name";"maxDepth"; "maxLevel"; "isAttributeDimension"; "isSubSetDimensio"; "isSystemDimension")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapDimensions(String locator, String mask) {
		ResultDescriptor d = meta.getOlapDimensions(locator, mask);
		logDescriptor(d,true);
		return d;
	}

	/**
	 * get the dimension MetaData of a given cube from an existing (olap) connection
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param cube the name of the cube within the database
	 * @param mask abinary 3-bit filter criterion on types of dimensions. null for no filter. 1st = AttributeDimensions, 2nd bit = SubSetDimensions, 3rd bit = isSystemDimension. Turn the bits on ("1") to include and off ("0") to exclude.
	 * @return a {@link ResultDescriptor}. Result contains dimension information from an OLAP-System as csv. ("Id"; "Name";"maxDepth"; "maxLevel"; "isAttributeDimension"; "isSubSetDimensio"; "isSystemDimension")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapCubeDimensions(String locator, String cube, String mask) {
		ResultDescriptor d = meta.getOlapCubeDimensions(locator, cube, mask);
		logDescriptor(d,true);
		return d;
	}

	/**
	 * Uploads a file to the internal data repository.
	 * @param name the name of the file to write to
	 * @param data the data to write into the file.
	 * @return a {@link ResultDescriptor}. Result is "added file [name]" or "replaced file [name]" if successful.
	*/
	public ResultDescriptor uploadFile(String name, byte[] data) {
		log.info("Upload data file " + name);
		ResultDescriptor d = new ResultDescriptor();
		String dir = Settings.getInstance().getDataDir();
		File file = new File(dir+File.separator+name);
		try {
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
		log.info("finished");
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

		ResultDescriptor d = new ResultDescriptor();
		try {
			log.info("Starting drillthrough on datastore "+datastore);
			StringWriter out = new StringWriter();
			if (lines == 0)
				lines = Integer.MAX_VALUE;
			
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
			log.info("finished");
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to execute method drillThrough for datastore "+datastore+": "+e.getMessage());
		}
		logDescriptor(d,true);
		return d;
	}

	
	/** gets information about the location of the drillthrough datastore 
	 * @param datastore the name of the drillthrough datastore. The name is in the form 'Palo-DB-Name'.'Palo-Cube-Name' in upper casename the name of the executables - null for no filter.
	 * @return the drillthrough info containing Connector type, Connector, Schema name and Table name
	 * @throws AxisFault
	 */
	public DrillthroughInfoDescriptor[] drillThroughInfo(String datastore) throws AxisFault {
		try {
			log.info("Starting drillThroughInfo for datastore "+datastore);
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
		try {
			log.info("adding executions for: "+getPrintable(locator));
			//build context properties
			Properties properties = getProperties(variables);
			ExecutionDescriptor d = null;
			try {
				Execution e = Executor.getInstance().createExecution(Locator.parse(locator),properties);
				Executor.getInstance().addExecution(e);
				d = getDescriptor(e.getExecutionState());
			}
			catch (Exception e) {
				d = getErrorDescriptor("Failed to execute "+locator+": "+e.getMessage());
			}
			log.info("finished");
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
		log.info("removing executions");
		ExecutionDescriptor[] result = new ExecutionDescriptor[ids.length];
		for (int i=0; i<ids.length; i++) {
			Long id = ids[i];
			try {
				ExecutionState state = Executor.getInstance().removeExecution(id,true);
				if (state != null)
					result[i] = getDescriptor(state);
				else {
					result[i] = getErrorDescriptor("Failed to remove execution "+id+": Execution not found");
				}
			}
			catch (ExecutionException e) {
				result[i] = getErrorDescriptor("Failed to remove execution "+id+": "+e.getMessage());
			}
		}
		log.info("finished");
		return result;
	}

	/**
	 * runs an existing execution indicated by an execution id.
	 * @param id of the execution.
	 * @return an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor runExecution(Long id) {
		log.info("running execution "+id);
		ExecutionDescriptor descriptor;
		try {
			descriptor = getDescriptor(Executor.getInstance().runExecution(id).getExecutionState());
		}
		catch (ExecutionException ee) {
			descriptor = getErrorDescriptor("Failed to run execution "+id+": "+ee.getMessage());
		}
		log.info("finished");
		return descriptor;
	}

	/**
	 * stops an existing execution indicated by an execution id.
	 * @param id of the execution.
	 * @return an {@link ExecutionDescriptor}
	 */
	public ExecutionDescriptor stopExecution(Long id) {
		try {
			log.info("stopping execution: "+id);
			if (id == -1) { //stop all active executions

				String activeExecutions = "";
				for (ExecutionState s : Executor.getInstance().getUnfinishedExecutions()) {
					activeExecutions = activeExecutions.concat(s.getId() + ", ");
					Executor.getInstance().stop(s.getId());
				}
				if(!activeExecutions.equals(""))
					activeExecutions = activeExecutions.substring(0, activeExecutions.length()-2);
				ExecutionDescriptor ed = new ExecutionDescriptor();
				if(activeExecutions.length() != 0){
					ed.setResult("The following executions have been killed: " + activeExecutions  + ".");
				}else{
					ed.setResult("No active executions are on the server.");
				}
				return ed; //empty descriptor with the list of stopped or aborted executions (if existed)
			}
			else {
				ExecutionState c = Executor.getInstance().stop(id);
				return getDescriptor(c);
			}
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
				d = getDescriptor(Executor.getInstance().getExecutionState(id, waitForTermination));
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
	public ExecutionDescriptor[] getExecutionHistory(String project, String type, String name, long after, long before, String status) throws AxisFault {
		try {
			log.debug("Getting execution history.");
			log.debug("Project:"+project+" Type:"+type+" Name:"+name+" After:"+after+" Before:"+before+" Status:"+status+".");
			List<ExecutionState> states = StateManager.getInstance().getResults(project, type, name, after == 0 ? null : new Date(after), before == 0 ? null : new Date(before), status);
			ExecutionDescriptor[] result = new ExecutionDescriptor[states.size()];
			for (int i=0; i<states.size(); i++) {
				result[i] = getDescriptor(states.get(i));
			}
			log.debug("Retrieved exections: "+states.size());
			log.debug("finished");
			return result;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getExecutionHistory for project "+project+": "+e.getMessage());
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
		ResultDescriptor d = new ResultDescriptor();
		try {
			log.debug("Getting execution log details.");
			//first look in executor, since executor also has non-persistent executions.
			try {
				Execution execution = Executor.getInstance().getExecution(id);
				d.setResult(execution.getExecutionState().getMessagesText(type, timestamp,start,pagesize));
			}
			catch (ExecutionException e) {
				if (id == null) { //we got an invalid null id
					d.setErrorMessage("Failed to get log: "+e.getMessage());
				}
				else { //execution is not present in memory. try lookup in persistence
					ExecutionState result = StateManager.getInstance().getResult(id);
					if (result != null)
						d.setResult(result.getMessagesText(type, timestamp,start,pagesize));
					else
						d.setErrorMessage("Failed to get log for execution "+id+": "+e.getMessage());
				}
			}
			logDescriptor(d,true);
			log.debug("finished");
			return d;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to execute method getExecutionLog for execution "+id+": "+e.getMessage());
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
	public ExecutionDescriptor getData(String locator, Variable[] variables, String view, int lines, Boolean waitForTermination) throws AxisFault {
		if (waitForTermination == null)
			waitForTermination = Boolean.TRUE;
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
			Execution e;
			try {
				e = Executor.getInstance().createData(loc, properties,view);
			}
			catch (Exception ex) {
				// Validation error during initialization phase
				return getErrorDescriptor(ex.getMessage());
			}
			e.getExecutable().getParameter().setProperty("sample", String.valueOf(lines));
			e.getExecutionState().setDataTarget(DataTargets.INLINE);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getDescriptor(Executor.getInstance().getExecutionState(e.getKey(), waitForTermination));
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
	public ExecutionDescriptor prepareData(String locator, Variable[] variables, String view, int lines) {
		Properties properties = getProperties(variables);
		try {
			log.info("preparing data for: "+getPrintable(locator));
			Locator loc = Locator.parse(locator);
			Execution e;
			try {
				e = Executor.getInstance().createData(loc, properties,view);
			}
			catch (Exception ex) {
				// Validation error during initialization phase
				return getErrorDescriptor(ex.getMessage());
			}
			e.getExecutable().getParameter().setProperty("sample", String.valueOf(lines));
			e.getExecutionState().setDataTarget(DataTargets.PERSISTENCE);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			String target = Executor.getInstance().getExecutionState(e.getKey(), false).getData();
			log.info("Data is streamed internally to: "+target);
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
	public ResultDescriptor fetchData(String datastore, int offset, int lines) throws AxisFault {
		//change this to pagination without filtering!!
		ResultDescriptor d = new ResultDescriptor();
		String errorPrefix = "Error in getting Data from Datastore "+datastore+" ";
		try {
			log.info("Getting data from datastore "+datastore);
			StringWriter out = new StringWriter();
			CSVWriter writer = new CSVWriter(out);
			Datastore ds = DatastoreManager.getInstance().get(datastore);
			if (ds != null) {
				IProcessor processor = ds.getProcessor("select * from "+NamingUtil.internalDatastoreName()+" where "+ds.escapeName(NamingUtil.internalKeyName())+">="+offset,lines);
				//processor.setLastRow(lines);
				writer.write(processor);
			}
			else {
				String message = "Datastore "+datastore+" not found.";
				log.error(message);
				d.setErrorMessage(errorPrefix + message);
			}
			log.info("finished");
			d.setResult(out.toString());
		}
		catch (Exception e) {
			d.setErrorMessage(errorPrefix+e.getMessage());
		}
		logDescriptor(d,true);
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
		if (waitForTermination == null)
			waitForTermination = Boolean.TRUE;
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
			Execution e = Executor.getInstance().createOutputDescription(loc, properties,view);
			Executor.getInstance().addExecution(e);
			Executor.getInstance().runExecution(e.getKey());
			ExecutionDescriptor d = getExecutionStatus(e.getKey(),waitForTermination);
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
		if (waitForTermination == null)
			waitForTermination = Boolean.TRUE;
		Properties properties = getProperties(variables);
		try {
			Locator loc = Locator.parse(locator);
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
			return d;
		}
		catch (Exception ex) {
			return getErrorDescriptor("Failed to test "+getPrintable(locator)+": "+ex.getMessage());
		}
	}

	/**
	 * Gets the list of all ingoing dependent components of a component e.g. all loads of a job
	 * @param locator the path to the component in the format component.manager.component: e.g: "myProject.jobs.default".
	 * @param mode 0: only the direct dependencies are calculated, 1: dependencies calculated recursively, 2: dependencies calculated recursively with distinction on referenced component type for connections 
	 * @return the list of all components from which the input-component is dependent
	 * @throws AxisFault
	 */
	public ComponentDependencyDescriptor[] getComponentDependencies(String locator) throws AxisFault {
		try {
			Locator loc = Locator.parse(locator);
			IProject project = (IProject) ConfigManager.getInstance().getComponent(loc.getRootLocator(), IContext.defaultName);
			Map<String,List<String>> deps = project.getAllDependencies(locator,false);
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
			return result;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to get dependencies for "+locator+": "+e.getMessage());
		}
	}

	/**
	 * Gets the list of all outgoing components of a component e.g. all jobs which include a load 
	 * @param locator the path to the component in the format component.manager.component: e.g: "myProject.jobs.default".
	 * @param mode 0: only the direct dependencies are calculated, 1: dependencies calculated recursively, 2: dependencies calculated recursively with distinction on referenced component type for connections 
	 * @return the list of all components which depend on the input-component
	 * @throws AxisFault
	 */
	public ComponentDependencyDescriptor[] getComponentDependents(String locator) throws AxisFault {
		try {
			Locator loc = Locator.parse(locator);
			IProject project = (IProject) ConfigManager.getInstance().getComponent(loc.getRootLocator(), IContext.defaultName);
			Map<String,List<String>> deps = project.getAllDependents(locator,false);
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
			return result;
		}
		catch (Exception e) {
			throw new AxisFault("Failed to get dependants for "+locator+": "+e.getMessage());
		}
	}
	
	public ResultDescriptor getProjectDocumentation(String locator, String[] graphLocators) throws AxisFault {
		try {
			ResultDescriptor d = new ResultDescriptor();
			String projectName = null;
			try {
				Locator loc = Locator.parse(locator);
				projectName = loc.getRootName();
				IProject project = ConfigManager.getInstance().getProject(loc.getRootLocator().getName());					
				DocuUtil util = new DocuUtil(project, loc, graphLocators, null);
				d.setResult(util.getDocu());			
			}
			catch (ConfigurationException e) {
				d.setErrorMessage("Could not generate documentation for project "+projectName+": "+e.getMessage());
				return d;
			}
			return d; 
		}	
		catch (Exception e) {
			e.printStackTrace();
			throw new AxisFault("Failed to get project documentation for "+locator+": "+e.getMessage());
		}
	}

	
	public ResultDescriptor calculateComponentGraph(String locator, Variable[] settings) {
		ResultDescriptor d = new ResultDescriptor();
		try {
			Locator loc = Locator.parse(locator);
			IProject project;
			List<Locator> invalidComponents = new ArrayList<Locator>(); 
			try {
				project = (IProject) ConfigManager.getInstance().getProject(loc.getRootLocator().getName());
				invalidComponents = ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true);
				// Check if component exists
				ConfigManager.getInstance().getComponent(loc, IContext.defaultName);
			}
			catch (Exception ex) {
				// Validation error during initialization phase
				d.setErrorMessage("Graph could not be generated: "+ex.getMessage());
				return d;
			}
			IGraphUtil util = GraphUtilFactory.getGraphUtil(project, locator, getProperties(settings), invalidComponents);
			d.setResult(util.getSVG());
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to calculate graph for "+locator+": "+e.getMessage());
		}
		return d;
	}
		
	/**
	 * saves a project to disk
	 * @param locator the locator of the project (= project name) or the locator of a component from a project
	 * @return a {@link ResultDescriptor}
	 */
	public ResultDescriptor save(String locator) {
		ResultDescriptor d = new ResultDescriptor();
		try {
			Locator loc = Locator.parse(locator);
			ConfigManager.getInstance().save(loc);
			d.setResult("Saved Configuration.");
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to save "+getPrintable(locator)+": "+e.getMessage());
		}
		logDescriptor(d,false);
		return d;
	}

	/**
	 * gets the server current status, info like free memory,used memory,...
	 * @return the {@link ServerStatus}
	 * @throws AxisFault
	 */
	public ServerStatus getServerStatus() throws AxisFault {
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
		return result;
	}
	catch (Exception e) {
		throw new AxisFault(e.getMessage());
	}
	}

}
