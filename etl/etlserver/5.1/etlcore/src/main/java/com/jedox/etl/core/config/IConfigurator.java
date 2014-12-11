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
import org.jdom.Element;

import java.util.List;
import java.util.Properties;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.context.IContext;

/**
 * Interface for the common functionality of all Configurators. 
 * A configurator is responsible to provide the translation from a XML-configuration of a {@link com.jedox.etl.core.component.IComponent Component} to data-structures understood by the component in its initialization phase.
 * In this phase a Component is intended to configure itself by its {@link com.jedox.etl.core.component.IComponent#init()} method with the help of its configurator. The component is not intended to directly access the XML structures.
 * Thus the configurator should encapsulate all XML-processing and expose suitable data-structures to the component. Whenever changes in the XML syntax are made, the configurator should cope with them making them transparent for the component.
 * <p>
 * Another key task of the configurator is the resolving of variables from the XML-configuration to context sensitive values. The default value of a variable is declared in the <variables> section of a project like this:
 * <br>
 * <pre>
 * {@code
 * <variables>
 * 	<variable name="myVariable">myValue</variable>
 * </variables>
 * }
 * </pre> 
 * <br>
 * A variable is referenced from XML by ${myVariable}. Each variable can be set in a specific context either by redeclaring it in a {@link com.jedox.etl.core.job.IJob job} or externally via the client interface on {@link com.jedox.etl.core.execution.IExecutor execution} runtime.
 * A variable can be both used in an attribute value or in the context of elements. Variables are especially useful if you want to be able to change the behavior of your project in a single point, instead of doing a copy, search, replace approach.   
 * </p>
 * <p>
 * The basic data-structure provided by all configurators is a Properties object, which gives access to key-value pairs.
 * Any configurator may implement additional methods to expose more complex data-structures to a component. 
 * </p>   
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IConfigurator {
	
	/**
	 * Sets the locator and the context of this configurator. 
	 * The context is the context the component should be configured for.
	 * The locator does not include the name of the component itself, which is determined by the configurator from the XML and is available via {@link #getName()}. Instead the locator given here corresponds to the locator of the manager requesting the component to be configured.  
	 * @param locator the full address path to the component to be configured. Component name must not be included itself.
	 * @param locatable the context holding the variables to be used for configuration. 
	 */
	public void setLocator(Locator locator, IContext locatable);
	/**
	 * Gets the locator of the configurator. The locator does not include the name of the component itself, which is determined by the configurator from the XML and is available via {@link #getName()}.
	 * @return the locator as full address path to the component
	 */
	public Locator getLocator();
	/**
	 * Sets the XML from which this configurator should build the required data-structures for the component.
	 * @param element the root element of the component configuration XML
	 */
	public void setXML(Element element, List<String> variables);
	/**
	 * Gets the raw XML for this configurator. Normally you should not work an this raw XML outside of the configurator, since you will break encapsulation of processing and variable resolving. So use this only if you really know what you are doing.
	 * @return the root element of the component configuration XML
	 */
	public Element getXML();
	/**
	 * Gets the context holding the variables for this configurator.
	 * @return the context
	 */
	public IContext getContext();
	/**
	 * Gets the parameters of this configurator. The parameters returned contain all externally inherited parameters (set via {@link #addParameter(Properties)}) and all own parameters set in the XML via the <parameter name="myParameter">myValue</parameter> tag.
	 * <p>
	 * This method is intended for internal use only to handle inheritance of parameters in the component / manager hierarchy.
	 * To get the value of a specific parameter use {@link #getParameter(String, String)}.
	 * @return a Properties object holding the parameters
	 */
	public Properties getParameter();
	/**
	 * Adds external parameters to this configurator. When used before {@link #configure()} is called, they serve as default values, in case the XML does not specify them.  
	 * @param properties the external parameters
	 */
	public void addParameter(Properties properties);
	/**
	 * Gets a specific parameter value from this configurator. Additionally to the parameters available via the {@link #getParameter()} method, all simple XML tags of the form <someTag>value</someTag> are also accessible for convenience and backwards compatibility.
	 * @param name the name of the parameter
	 * @param defaultValue the value to return if the parameter is not set.
	 * @return the parameter value
	 * @throws ConfigurationException if the value is a variable, which is not defined.
	 */
	public String getParameter(String name, String defaultValue) throws ConfigurationException;
	/**
	 * Gets the name of this configurator, which normally is also the name of the component to be configured.
	 * @return the name.
	 */
	public String getName();
	/**
	 * Configures the configurator. Builds up internal data-structures from the XML. Usually {@link #setXML(Element)} and {@link #setLocator(Locator, ILocatable)} should have been called before.  
	 * @throws ConfigurationException if anything goes wrong in this process.
	 */
	public void configure() throws ConfigurationException;
	/**
	 * gets all variables defined by this configurator or inherited. In contrast to parameters variables are dynamically substituted by at context creation 
	 * @return the variables
	 */
	public Properties getVariables();
	/**
	 * gets only the variables defined by this configurator
	 * @return
	 * @throws ConfigurationException
	 */
	public Properties getOwnVariables() throws ConfigurationException;
	
	public Properties getOwnParameters() throws ConfigurationException;
	
	public List<String> getResolvedVariables(); 
	
	public <T> T getRootClass(Class<T> clazz) throws ConfigurationException;
}
