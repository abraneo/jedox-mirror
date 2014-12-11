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

import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.util.Hashtable;
import java.io.StringReader;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
//import org.w3c.dom.NodeList;
//import org.w3c.dom.Node;
import org.xml.sax.SAXException;

import org.jdom.Element;
import org.jdom.input.DOMBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.util.ClassUtil;
import com.jedox.etl.core.util.XMLUtil;

/**
 * Class for the validation of configurations according to their supplied XML-Schema Documents (xsds) 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ConfigValidator {
	
	private static ConfigValidator instance;
	private static final Log log = LogFactory.getLog(ConfigValidator.class);
	private Hashtable<String, Validator> validators = new Hashtable<String, Validator>();
	private Hashtable<String, Document> documents = new Hashtable<String, Document>();
	private DocumentBuilder parser;
	//private Document corelib;
	
	ConfigValidator() {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		factory.setNamespaceAware(true);
		factory.setValidating(false);
		try {
			parser = factory.newDocumentBuilder();
			//corelib = parser.parse(this.getClass().getResourceAsStream("corelib.xsd"));
		} catch (Exception e) {
			log.error("Failed to create ConfigValidator: "+e.getMessage());
		}
	}
	
	/**
	 * gets the instance of the ConfigValidator
	 * @return the COnfigValidator
	 */
	public final synchronized static ConfigValidator getInstance() {
		if (instance == null) {
			instance = new ConfigValidator();
		}
		return instance;
	}
	
	/**
	 * gets the XML-Schema Document (xsd) in a human readable string representation.
	 * @param className the name of the java class to get the Schema for. 
	 * @return the Schema Document in a String representation.
	 */
	public synchronized String getSchema(String className) {
		String ret = null;
		try {
			Class<?> cc = Class.forName(className);
			DOMBuilder b = new DOMBuilder();
			StringWriter writer = new StringWriter();
			XMLOutputter outputter = new XMLOutputter(Format.getPrettyFormat());
			outputter.output(b.build(getSchemaDocument(cc)), writer);
			ret = writer.toString();
		} catch (Exception e) {
			log.warn("No schema for " + className +": "+e.getMessage());
		}
		return ret;
	}
	
	/**
	 * gets the default schema document of a class with respect to its interfaces implemented. The default schema is a fallback mechanism for components not having their own schema document. The default schema document has to be stored in a file called "default.xsd" in a resource directory, which maps to the implemented interface of a class. 
	 * @param implementingClass the java class to get the default schema for. 
	 * @return the Schema Document
	 * @throws SAXException
	 * @throws IOException
	 */
	private Document getDefaultSchemaDocument(Class<?> implementingClass) throws SAXException, IOException {
		InputStream is = implementingClass.getResourceAsStream("default.xsd");
		if (is == null) {
			for (Class<?> c : ClassUtil.getAllInterfaces(implementingClass)) {
				is = c.getResourceAsStream("default.xsd");
				if (is != null)
					break;
			}
		}
		if (is == null)
			throw new IOException("Default schema document for "+implementingClass+" does not exist either. Validation failed.");
		return parser.parse(is);
	}
	
	private synchronized Document getSchemaDocument(Class<?> implementingClass) throws SAXException, IOException {
		String className = implementingClass.getCanonicalName();
		Document schemaDoc = documents.get(className);
		if (schemaDoc == null) {
			//get ressource as stream
			InputStream is = implementingClass.getResourceAsStream(implementingClass.getSimpleName()+".xsd");
			if (is == null) //fall back to default
				schemaDoc = getDefaultSchemaDocument(implementingClass);
			else //parse document from stream
				schemaDoc = parser.parse(is);
			/*
			//add lib types to schema document
			NodeList l = corelib.getDocumentElement().getChildNodes();
			for (int i=0; i<l.getLength(); i++) {
				Node n = schemaDoc.importNode(l.item(i), true);
				schemaDoc.getDocumentElement().appendChild(n);
			}
			*/
			documents.put(className, schemaDoc);
		}
		return schemaDoc;
	}

	private Validator getDefaultValidator(Class<?> implementingClass) throws SAXException, IOException {
		Document schemaDoc = getDefaultSchemaDocument(implementingClass);
		SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
		Schema schema = sf.newSchema(new DOMSource(schemaDoc));
		return schema.newValidator();
	}
	
	private Validator getValidator(Class<?> implementingClass) throws SAXException, IOException {
		String className = implementingClass.getCanonicalName();
		Validator validator = validators.get(className);
		if (validator == null) {
			Document schemaDoc = null;
			schemaDoc = getSchemaDocument(implementingClass);
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new DOMSource(schemaDoc));
			validator = schema.newValidator();
			validators.put(className, validator);
		}	
		return validator;
	}

	private String getSchemaURI(Class<?>implementingClass) {
		Document d;
		String uri = null;
		try {
			d = getSchemaDocument(implementingClass);
			uri = d.lookupNamespaceURI("c");
		} 
		catch (SAXException e) {} 
		catch (IOException e) {}
		if (uri == null)
			uri = "http://schemas.proclos.com/etlcore";
		return uri;
	}
	
	private String jdomToString(Element element, Class<?> implementingClass) {
		String configString = "";
		try {
			configString = XMLUtil.jdomToString(element).trim();
		} catch (IOException e) {
			log.error("Failed to convert Element to String: "+e.getMessage());
		}
		return configString.replaceFirst(element.getName(), element.getName()+" xmlns=\""+getSchemaURI(implementingClass)+"\" ");
	}
	
	/**
	 * validates a configuration according to the schema document specified for its implementing class. If no such schema document is present, a default schema is tried to apply in a fallback mechanism. The default schema document has to be stored in a file called "default.xsd" in a resource directory, which maps to the implemented interface of a class.    
	 * @param implementingClass the java class, which implements the configuration 
	 * @param config the configuration
	 * @throws IOException, if no valid schema documents exists
	 * @throws SAXException, if the configuration is not valid with respect to its schema document. 
	 */
	private void doValidate(Class<?> implementingClass, Element config) throws IOException, SAXException {
		
		Validator validator=null;
		try {
			validator = getValidator(implementingClass);
		}
		catch (IOException e) {
			if (ClassUtil.implementsInterface(implementingClass,IFunction.class)) {
				log.info("Schema for class " + implementingClass + ", was not found. Using default schema instead.");
				validator = getDefaultValidator(implementingClass);
			}else{
				throw e;
			}
		}
			
			String configString = jdomToString(config, implementingClass);
			validator.validate(new StreamSource(new StringReader(configString)));
		
	}
	
	private String getComponentName(Element config) {
		return config.getAttributeValue("name","default");
	}
	
	private String getTypeName(Class<?> implementingClass) {
		String packageName = implementingClass.getPackage().getName();
		return packageName.substring(packageName.lastIndexOf('.')+1);
	}
	
	private String rewriteValidationMessage(String message, Class<?> implementingClass) {
		message = message.substring(message.indexOf(':')+1);
		message = message.replaceAll("\""+getSchemaURI(implementingClass)+"\":", "");
		return message;
	}
	
	/**
	 * tries to validate a configuration according to the schema document specified for its implementing class. If no such schema document is present, a default schema is tried to apply in a fallback mechanism. The default schema document has to be stored in a file called "default.xsd" in a resource directory, which maps to the implemented interface of a class. If validation is turned off in the "config.xml" nothing is done here.
	 * @param implementingClass the java class implementing the configuration
	 * @param config the configuration
	 * @throws ConfigurationException, if validation fails.
	 */
	public synchronized void validate(Class<?> implementingClass, Element config) throws ConfigurationException {
		if ((config != null) && Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("validate", "false").equalsIgnoreCase("true")) {
			try {
				doValidate(implementingClass,config);
			} catch (IOException e) {
				throw new ConfigurationException("Failed to find schema for class "+implementingClass.getCanonicalName()+": "+e.getMessage());
			} catch (SAXException e) {
				throw new ConfigurationException("Failed to validate "+getTypeName(implementingClass)+" "+getComponentName(config)+":"+rewriteValidationMessage(e.getMessage(),implementingClass));
				//System.out.println(jdomToString(config));
			}
		}
	}

}
