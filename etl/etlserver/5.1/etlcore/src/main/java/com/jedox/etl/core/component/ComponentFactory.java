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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;

import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;

import javax.xml.XMLConstants;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;
import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.helpers.DefaultHandler;

import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.ConnectionFactory;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.extract.ExtractFactory;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.function.FunctionFactory;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.job.JobFactory;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.LoadFactory;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.project.ProjectFactory;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformFactory;

/**
 * Factory class for all components defined in the component registry built from one or more component.xml files. 
 * @author gerhard
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

public class ComponentFactory
{
	/**
	 * Custom handler for parsing component.xml files and constructing ComponentDescriptors
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	private class ComponentParser extends DefaultHandler {

		private ComponentDescriptor currentComp;
		private String scope;
		private String paramName;

		public void startElement(String nsUri, String name, String qName,
				Attributes attributes) throws SAXException {
			super.startElement(nsUri, name, qName, attributes);
			//TODO: check nameSpace here too
			if (name.equals("component")) {
				String compName = attributes.getValue("name");
				String className = attributes.getValue("class");
				Boolean def = Boolean.valueOf(attributes.getValue("default"));
				String modes = attributes.getValue("modes");
				String status = attributes.getValue("status");
				String caption = attributes.getValue("caption",compName);
				//String jdbc = attributes.getValue("jdbc"); // added by Kais
				String connectionList = attributes.getValue("connectionList"); // added by Kais
				currentComp = new ComponentDescriptor(compName, className, scope, def, caption, connectionList,modes, status);
			} else if (name.equals("parameter")) {
				paramName = attributes.getValue("name");
			} else {
				scope = name;
			}
		}

		public void characters(char[] ch, int start, int length) throws SAXException {
			super.characters(ch, start, length);
			if (currentComp != null && paramName != null) {
				currentComp.addParameter(paramName, String.valueOf(ch).substring(start, start+length).trim());
			}
		}

		public void endElement(String uri, String localName, String name)
		throws SAXException {
			super.endElement(uri, localName, name);
			//TODO: check nameSpace here too
			if (name.equalsIgnoreCase("component"))
			{
				// TODO: check if currentComp is not Empty
				if (currentComp != null)
				{
					componentMap.put(currentComp.getQName().toLowerCase(), currentComp);
					currentComp = null;
				}
			}
			else if (name.equals("parameter")) {
				paramName = null;
			} else {
				scope = null;
			}
		}

		public void startDocument() throws SAXException {
			// TODO Auto-generated method stub
			super.startDocument();
			currentComp = null;
			scope = null;
		}

		public void error(SAXParseException e) throws SAXException {
			// TODO Auto-generated method stub
			super.error(e);
			log.error(e.getMessage());
			throw e;
		}

		public void fatalError(SAXParseException e) throws SAXException {
			// TODO Auto-generated method stub
			super.fatalError(e);
			log.error("Fatal :" + e.getMessage());
			throw e;
		}

		public void warning(SAXParseException e) throws SAXException {
			// TODO Auto-generated method stub
			super.warning(e);
			log.warn(e.getMessage());
			// throw e;
		}
	}

	private static Log log = LogFactory.getLog(ComponentFactory.class);

	private Map<String, ComponentDescriptor> componentMap = new HashMap<String, ComponentDescriptor>();

	private static ComponentFactory instance;

	private String filename;

	private ComponentParser parser = new ComponentParser();


	private ComponentFactory() {
		super();
	}
/*	
	public static List<ComponentDescriptor> getAllComponentDescriptors(String fileName) throws IOException, ParserConfigurationException, SAXException {
		ComponentFactory f = new ComponentFactory();
		f.readComponentConfig(fileName);
		List<ComponentDescriptor> result = new ArrayList<ComponentDescriptor>();
		for (ITypes.Components ct : ITypes.Components.values()) {
			for (ComponentDescriptor d : f.componentMap.values()) {
				if (d.getScope().startsWith(ct.name())) result.add(d);
			}
		}
		return result;
	}
*/
	/**
	 * gets the factory instance.
	 * Parses all component.xml files found in the directories below the config directory. 
	 * @return the factory instance
	 */
	public static synchronized ComponentFactory getInstance() 
	{
		if (instance == null)
		{
			instance = new ComponentFactory();
			try {
				File dir = new File(Settings.getConfigDir());
				if (!dir.exists()) {
					log.error("Configuration directory " + Settings.getConfigDir() + " does not exists.");	
					return instance;
				}	
				String standard = Settings.getConfigDir() + File.separator + "standard" + File.separator + "component.xml";
				File f = new File(standard);
				if (f.exists()) {
					instance.readComponentConfig(standard);	
				} else {
					log.warn("standard component.xml does not exists.");
				}
				File[] plugins = dir.listFiles();
				for (File p : plugins) {
					//test for directory and ignore core directory
					if (p.isDirectory() && !p.getName().equalsIgnoreCase("standard")) {
						f = new File(Settings.getConfigDir() + File.separator + p.getName() + File.separator + "component.xml");
						if (f.exists()) {
							instance.readComponentConfig(f.getCanonicalPath());
						} else {
							log.debug("No component.xml available in "+p.getName());
						}
					}
				}
				
			} catch (SAXException e) {
				log.error("Error in component.xml: "+e.getMessage());
				log.debug(e);
			} catch (IOException e) {
				log.error("Error in component.xml: "+e.getMessage());
				log.debug(e);
			} catch (ParserConfigurationException e) {
				log.error("Error in component.xml: "+e.getMessage());
				log.debug(e);
			}
		}
		return instance;
	}

	/**
	 * reads component configs from a file
	 * @param fname the filename
	 * @throws IOException
	 * @throws ParserConfigurationException
	 * @throws SAXException
	 */
	private void readComponentConfig(String fname) throws IOException, ParserConfigurationException, SAXException
	{
		if (filename == null || filename.compareTo(fname) != 0)
		{
			filename = fname;
			Reader r = new BufferedReader(new FileReader(filename));
			readComponentConfig(r);
		}
	}

	/**
	 * reads component configs from a reader 
	 * @param componentxml the reader
	 * @throws ParserConfigurationException
	 * @throws IOException
	 * @throws SAXException
	 */
	private void readComponentConfig(Reader componentxml) throws ParserConfigurationException, IOException, SAXException {
		loadConfiguration(new InputSource(componentxml));
	}

	/**
	 * creates an IConnection component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param context the context this component should be created in. 
	 * @param config the xml config defining the component.
	 * @return the object implementing the IConnection interface
	 * @throws CreationException
	 */
	public IConnection createConnection(String name, ILocatable manager, IContext context, Element config) throws CreationException{
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Connections);
		if (cd != null) {
			return ConnectionFactory.getInstance().newConnection(cd, manager, context, config);
		}
		throw new CreationException("Connection type " + name + " not found.");
	}

	/**
	 * creates an ILoad component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param context the context this component should be created in. 
	 * @param config the xml config defining the component.
	 * @return the  object implementing the ILoad interface
	 * @throws CreationException
	 */
	public ILoad createLoad(String name, ILocatable manager, IContext context, Element config) throws CreationException {
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Loads);
		if (cd != null) {
			return LoadFactory.getInstance().newLoad(cd,manager, context, config);
		}
		throw new CreationException("Load type " + name + " not found.");
	}

	/**
	 * creates an IExtract component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param context the context this component should be created in. 
	 * @param config the xml config defining the component.
	 * @return the  object implementing the IExtract interface
	 * @throws CreationException
	 */
	public IExtract createExtract(String name, ILocatable manager, IContext context, Element config) throws CreationException {
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Extracts);
		if (cd != null) {
			return ExtractFactory.getInstance().newExtract(cd, manager, context, config);
		}
		throw new CreationException("Extract type " + name + " not found.");
	}

	/**
	 * creates an ITransformer component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param context the context this component should be created in. 
	 * @param config the xml config defining the component.
	 * @return the  object implementing the ITransformer interface
	 * @throws CreationException
	 */
	public IFunction createFunction(String name, ILocatable manager, IContext context, Element config) throws CreationException {
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Functions);
		if (cd != null) {
			return FunctionFactory.getInstance().newFunction(cd, manager, context, config);
		}
		throw new CreationException("Function type " + name + " not found.");
	}

	/**
	 * creates an ITransform component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param context the context this component should be created in. 
	 * @param config the xml config defining the component.
	 * @return the  object implementing the ITransform interface
	 * @throws CreationException
	 */
	public ITransform createTransform(String name, ILocatable manager, IContext context, Element config) throws CreationException {
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Transforms);
		if (cd != null) {
			return TransformFactory.getInstance().newTransform(cd, manager, context, config);
		}
		throw new CreationException("Transform type " + name + " not found.");
	}

	/**
	 * creates an IProject component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param config the xml config defining the component.
	 * @return the  object implementing the IProject interface
	 * @throws CreationException
	 */
	public IProject createProject(String name, ILocatable manager, Element config) throws CreationException {
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Projects);
		if (cd != null) {
			return ProjectFactory.getInstance().newProject(cd, manager, config);
		}
		throw new CreationException("Project type " + name + " not found. Verify the ETL Server configuration");
	}

	/**
	 * creates an IJob component with the settings specified.
	 * @param name the name of the component as registered
	 * @param manager the parent holding this component
	 * @param context the context this component should be created in. 
	 * @param config the xml config defining the component.
	 * @return the  object implementing the IJob interface
	 * @throws CreationException
	 */
	public IJob createJob(String name, ILocatable manager, IContext context, Element config) throws CreationException {
		ComponentDescriptor cd = getComponentDescriptor(name, ITypes.Jobs);
		if (cd != null) {
			return JobFactory.getInstance().newJob(cd, manager, context, config);
		}
		throw new CreationException("Job type " + name + " not found.");
	}

	/**
	 * gets the ComponentDescriptors for all production components. experimental are deprecated components are not listed in the result.
	 * @param scope the scope of the components as in the component.xml e.g. extracts
	 * @return a list of ComponentDescriptors
	 */
	public List<ComponentDescriptor> getComponentDescriptors(String scope) {
		ArrayList<ComponentDescriptor> result = new ArrayList<ComponentDescriptor>();
		for (ComponentDescriptor i: componentMap.values()) {
			if (i.getScope().equalsIgnoreCase(scope) && i.isProductional()) {
				result.add(i);
			}
		}
		return result;
	}

	/** gets the ComponentDescriptors for all production components in sorted order on component name
	 * @param scope the scope of the components as in the component.xml e.g. extracts
	 * @return a list of ComponentDescriptors
	 */
	public List<ComponentDescriptor> getComponentDescriptorsSorted (String scope) {
		List<ComponentDescriptor> result = getComponentDescriptors(scope);
		Collections.sort(result);
		return result;
	}	
	
	/**
	 * gets a ComponentDescriptor by name
	 * @param name the name of the registered component
	 * @param type the type of the registered component as in the component.xml
	 * @return the ComponentDescriptor  
	 */
	public ComponentDescriptor getComponentDescriptor(String name,
			String type) {
		
		if (name == null) {
			// Determine default component if no component is specified
			for (ComponentDescriptor i: componentMap.values()) {
				if (i.getScope().equalsIgnoreCase(type) && i.isDefault()) {
					log.debug("Component "+name+" not found. Falling back to default.");
					return i;
				}
			}
			return null;
		} 
		else {
			return componentMap.get((type + '.' + name).toLowerCase());			
		}
	}

	/**
	 * parses and validates a component registry from a sax input source
	 * @param s the input source
	 * @throws SAXException
	 * @throws IOException
	 * @throws ParserConfigurationException
	 */
	private void loadConfiguration(InputSource s) throws SAXException, IOException, ParserConfigurationException
	{
		SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
		InputStream is = this.getClass().getResourceAsStream("component.xsd");
		Schema schema = sf.newSchema(new StreamSource(is));

		//File schemaFile = new File("component.xsd");
		//Schema schema = sf.newSchema(schemaFile);


		SAXParserFactory spf = SAXParserFactory.newInstance();
		//spf.setValidating(true);
		spf.setNamespaceAware(true);
		spf.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
		spf.setFeature("http://xml.org/sax/features/namespaces", true);
		//spf.setFeature("http://xml.org/sax/features/xmlns-uris", true);
		//spf.setFeature("http://xml.org/sax/features/validation", true);

		spf.setSchema(schema);

		SAXParser sp = spf.newSAXParser();
		sp.parse(s, parser);		

	}


}
