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
*   @author Andreas Fršhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.function;

import org.jdom.Element;

import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;
import com.jedox.etl.core.component.RuntimeException;

/**
 * Manager Class for {@link IFunction Functions}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class FunctionManager extends Manager {
//	private Hashtable<String, ITransformation> lookup = new Hashtable<String, ITransformation>();
	
	/**
	 * adds a {@link IFunction Function}
	 * @param function the function
	 * @return the function replaced by this function if any.
	 */
	public IFunction add(IComponent function) throws RuntimeException {
		if (function instanceof IFunction) {
			return (IFunction)super.add(function);
		}
		throw new RuntimeException("Failed to add non-Function object");
	}
	
	/**
	 * removes a function from this manager
	 */
	public IFunction remove(String name) {
		return (IFunction) super.remove(name);
	}
	
	/**
	 * removes all functions from this manager
	 */
	public void removeAll() {
		clear();
	}
	
	/**
	 * gets a function by name
	 * @param name the name of the function as registered 
	 * @return the function
	 */
	public IFunction get(String name) {
		return (IFunction) super.get(name);
	}
	
	/**
	 * gets all functions registered in this manager.
	 */
	public IFunction[] getAll() {
		IComponent[] components = super.getAll();
		IFunction[] transformations = new IFunction[components.length];
		for (int i=0; i<components.length; i++) {
			transformations[i] = (IFunction) components[i];
		}
		return transformations;
	}
	
	/**
	 * Adds a Function by receiving its configuration as xml, delegating the creating process to {@link ComponentFactory#createFunction(String, com.jedox.etl.core.component.ILocatable, com.jedox.etl.core.context.IContext, Element)}
	 */
	public IFunction add(Element config) throws CreationException, RuntimeException {
		IFunction transformationObject = ComponentFactory.getInstance().createFunction(config.getAttributeValue("type"), this, getContext(), config);
		add(transformationObject);
		return transformationObject;
	}
	
	public String getName() {
		return ITypes.Functions;
	}
	
	public FunctionManager cloneManager() throws RuntimeException {
		FunctionManager manager = new FunctionManager();
		manager.setLocator(this.getLocator(), this.getContext());
		manager.setLookupMode(this.getLookupMode());
		try {
			for (IFunction f : this.getAll()) {
				manager.add(f.getConfigurator().getXML());
			}
		}
		catch (CreationException e) {
			throw new RuntimeException("Error cloning functions of manager "+this.getLocator().toString()+": "+e);
		}
		return manager;
	}
}
