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

import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Properties;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locatable;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;
/**
 * Abstract base class for configurators, which provides the framework functionality common to all configurators.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class BasicConfigurator extends Locatable implements IConfigurator {

	//private static final Log log = LogFactory.getLog(BasicConfigurator.class);

	private Properties parameters = new Properties();
	private Properties variables = new Properties();
	private List<String> resolvedVariables;
	private Element root;

	public BasicConfigurator() {

	}

	public void setLocator(Locator locator, IContext context) {
		super.setLocator(locator,context);
		if (context != null) {
			addParameter(context.getParameter());
			variables.putAll(context.getVariables());
		}
	}

	public void configure() throws ConfigurationException {
		try {
			setName();
			//process all parameters
			parameters.putAll(getOwnParameters());	
			variables.putAll(getOwnVariables());
		}
		catch (Exception e) {
			throw new ConfigurationException(e);
		}
	}

	public void addParameter(Properties properties) {
		parameters.putAll(properties);
	}

	public void setXML(Element element, List<String> resolvedVariables) {
		root = element;
		this.resolvedVariables = resolvedVariables;
	}
	
	public List<String> getResolvedVariables() {
		return resolvedVariables;
	}

	public Element getXML() {
		return root;
	}

	public Properties getParameter() {
		return parameters;
	}

	public String getParameter(String name, String defaultValue) throws ConfigurationException {
		//use this method for backwards compatibility reason!!
		//try to get from a tag with an according name
		String result = getXML().getChildTextTrim(name);

		//in case of the functions
		if(result == null && getXML().getChild("parameters") != null){
			result = getXML().getChild("parameters").getChildTextTrim(name);
		}
		//try to get from a parameter field
		if (result == null)
			result = getParameter().getProperty(name);
		//finally take default value.
		if (result == null)
			result = defaultValue;
		return result;
	}

	/**
	 * gets an attribute value from the root XML-tag.
	 * @param name the name of the attribute
	 * @param defaultValue the default value to used, if the attribute is not set.
	 * @return the value of the attribute.
	 * @throws ConfigurationException
	 */
	protected String getAttribute(String name, String defaultValue) throws ConfigurationException {
		return getXML().getAttributeValue(name,defaultValue);
	}
	
	/**
	 * gets all parameters set within this configuration. Inherited parameters (e.g. from the parent or the context) are ignored.
	 * @return the parameters of this configuration.
	 * @throws ConfigurationException
	 */
	public Properties getOwnParameters() throws ConfigurationException {
		Properties parameters = new Properties();
		if (getXML() != null) {
			// process static parameters
			parameters.putAll(XMLUtil.parseProperties(getChildren(getXML(),"parameter")));
		}
		return parameters;
	}
	
	public Properties getVariables() {
		return variables;
	}
	
	public Properties getOwnVariables() throws ConfigurationException {
		Properties variables = new Properties();
		if (getXML() != null) {
			Properties prop = XMLUtil.parseProperties(getChildren(getXML(),"variable"));
			if (!prop.isEmpty()) {
				// Check name for variables
				Iterator<Object> i = prop.keySet().iterator();
				while (i.hasNext()) {
					String key = i.next().toString();
					if (key.startsWith(NamingUtil.internalPrefix())) {
						//allow for execution parameters to be set as variables; 
						if (!Settings.getInstance().getContext(Settings.ExecutionsCtx).containsKey(key))
							throw new ConfigurationException("Variable name "+key+" is not correct. Variables must not start with "+NamingUtil.internalPrefix());
					}
					if (key.startsWith(NamingUtil.hiddenInternalPrefix())) {
						throw new ConfigurationException("Variable name "+key+" is not correct. Variables must not start with "+NamingUtil.hiddenInternalPrefix());
					}
				}
				variables.putAll(prop);			
			}	
		}
		return variables;
	}
		
	protected void setName() throws ConfigurationException {
		if (getXML() != null) {
			String name = getXML().getAttributeValue("name");
			if (name != null) {
				// Check for not allowed charachters
				if (name.matches(".*\\..*")) {
					throw new ConfigurationException("Invalid name "+name+": The character . is not allowed");
				}
				setName(name.trim());
			}	
			else
				setName("");
		}
	}

	protected String escapeName(String name) {
		return NamingUtil.escape(name);
	}

	protected List<Element> getChildren(Element element, String name) {
		return getChildren(element,name,name+"s");
	}

	protected List<Element> getChildren(Element element, String name, String manager) {
		ArrayList<Element> result = new ArrayList<Element>();
		if (element != null) {
			Element m = element.getChild(manager);
			if (m != null) {
				List<?> l = m.getChildren(name);
				for (int i=0; i<l.size(); i++) {
					Element e = (Element) l.get(i);
					result.add(e);
				}
			} else {
				//fallback to parent element, since in 1.0 for some elements no managers where specified
				List<?> l = element.getChildren(name);
				for (int i=0; i<l.size(); i++) {
					Element e = (Element) l.get(i);
					result.add(e);
				}
			}
		}
		return result;
	}

	@Override
	public <T> T getRootClass(Class<T> clazz) throws ConfigurationException {
		throw new ConfigurationException("Not supported by "+this.getClass().getCanonicalName());
	}

}
