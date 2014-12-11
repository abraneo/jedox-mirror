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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;

/**
 * Manager class for {@link IContext Contexts}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ContextManager extends Manager {

	private static final Log log = LogFactory.getLog(ContextManager.class);
	private static int contextNumber = 0;
	private static final ContextManager instance = new ContextManager();
	
	public static final ContextManager getInstance() {
		return instance;
	}
	
	/**
	 * provides a default context within this manager.
	 * @throws CreationException
	 */
	public void provideDefault() throws CreationException {
		try {
			Element projectConfig = ConfigManager.getInstance().get(getLocator().getRootLocator());
			IContext context = ContextFactory.getInstance().newContext(Context.class.getCanonicalName(), this, projectConfig);
			context.setName(Context.defaultName);
			add(context);
		}
		catch (Exception e) {
			throw new CreationException(e.getMessage());
		}
	}
	
	/**
	 * provides a newly created context, which inherits all variables from its parent
	 * @param parentName the name of the parent context
	 * @param childName the name of the child context
	 * @return the newly created context
	 * @throws CreationException
	 */
	public synchronized IContext provide(IContext parentContext) throws CreationException, RuntimeException {
		contextNumber++;
		if (parentContext != null) {
			IContext childContext = ContextFactory.getInstance().newContext(parentContext.getClass().getCanonicalName(), parentContext, String.valueOf(contextNumber));
			parentContext.addChildContext(childContext);
			childContext.setBaseContextName(parentContext.getBaseContextName());
			childContext.getVariables().putAll(parentContext.getVariables());
			add(childContext);
			return childContext;
		} 
		else
			throw new CreationException("Context does not exist.");
	}
	
	/**
	 * gets a context by name
	 */
	public IContext get(String name) {
		if (name == null) {
			name = IContext.defaultName;
		}
		return (IContext) super.get(name);
	}
	
	/**
	 * removes a context
	 */
	public IContext remove(String name) {
		IContext result = (IContext) super.remove(name);
		if (result != null) { //cschw: also remove child contexts. 
			ContextManager ccm = (ContextManager) result.getManager(ITypes.Contexts);
			for (IContext c : ccm.getAll()) {
				remove(c.getName());
			}
			ccm.clear();
		}
		return result;
	}
	
	/**
	 * gets all registered contexts
	 */
	public IContext[] getAll() {
		IComponent[] components = super.getAll();
		IContext[] contexts = new IContext[components.length];
		for (int i=0; i<components.length; i++) {
			contexts[i] = (IContext) components[i];
		}
		return contexts;
	}

	/**
	 * creates a context by its variable definition from xml
	 */
	public IContext add(Element config) throws CreationException, RuntimeException {
		IContext contextObject = ContextFactory.getInstance().newContext(Context.class.getCanonicalName(), this, config); 
		add(contextObject);
		return contextObject;
	}

	/**
	 * adds a context to this manager
	 */
	public IContext add(IComponent context) throws RuntimeException {
		if (context instanceof IContext) {
			return (IContext) super.add(context);
		}
		else log.error("Failes to add non Context Object. Is ignored");
		return null;
	}
	
	public String getName() {
		return ITypes.Contexts;
	}
	
}
