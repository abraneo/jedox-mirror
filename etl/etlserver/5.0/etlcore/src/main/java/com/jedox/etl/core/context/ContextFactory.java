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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.context;

import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.util.NamingUtil;

/**
 * Factory class for the creation of {@link IContext Contexts}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

public class ContextFactory {

		
	private static final ContextFactory instance = new ContextFactory();
	private static final Log log = LogFactory.getLog(ContextFactory.class);
	
	public ContextFactory() {
	}
	
	/**
	 * gets the static instance
	 * @return the ContextFactory instance
	 */
	public static final ContextFactory getInstance() {
		return instance;
	}
	
	private void addExecutionParameters(IConfigurator c) {
		Properties executionSettings = Settings.getInstance().getContext(Settings.ExecutionsCtx);
		for (Object key : executionSettings.keySet()) {
			if (key.toString().startsWith(NamingUtil.internalPrefix())) {
				c.getParameter().setProperty(key.toString(), executionSettings.getProperty(key.toString()));
			}
		}
	}
	
	/**
	 * creates a new context from its xml-definition
	 * @param className the class name of the context to create
	 * @param parent the parent manager to hold the context
	 * @param config the xml config
	 * @return the newly created context
	 * @throws CreationException
	 */
	public IContext newContext(String className, ILocatable parent, Element config) throws CreationException {
		try {
			Class<?> contextClass = Class.forName(className);
			IContext c = (IContext) contextClass.newInstance();
			addExecutionParameters(c.getConfigurator());
			c.getConfigurator().addParameter(parent.getParameter());
			c.getConfigurator().setLocator(parent.getLocator(),null);
			c.getConfigurator().setXML(config);
			c.init();
			if (config == null)
				c.setName(IContext.defaultName);
			log.debug("Created Context "+c.getName());
			return c;
		}
		catch (Exception e) {
			throw new CreationException(e);
		}
	}
	
	/**
	 * creates a context as a child context of an existing context inheriting the parent's variables
	 * @param className the class name of the context to create
	 * @param parent the parent context to inherit from
	 * @param name the name of the new context
	 * @return the newly created context.
	 * @throws CreationException
	 */
	public IContext newContext(String className, ILocatable parent, String name) throws CreationException {
		try {
			Class<?> contextClass = Class.forName(className);
			IContext c = (IContext) contextClass.newInstance();
			//add runtime settable execution parameters.
			addExecutionParameters(c.getConfigurator());
			c.getConfigurator().addParameter(parent.getParameter());
			c.getConfigurator().setLocator(parent.getLocator().clone().reduce(),null);
			c.init();
			c.setName(name);
			log.debug("Created Context "+c.getName());
			return c;
		}
		catch (Exception e) {
			throw new CreationException(e);
		}
	}



}
