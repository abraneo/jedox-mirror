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

import java.util.Properties;

import org.jdom.Element;

/**
 * Interface describing the common functionality of all managers.
 * A manager is an object for the management of {@link IComponent components} of a certain {@link ITypes.Components type}.
 * Each component may have multiple managers for its sub-components, but only only one manager for a given type. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IManager extends ILocatable {
	
	/**
	 * Defines the types of LookupModes a manager can handle.
	 * <p>
	 * Name: Components are resolved by their name, which is the last part of their {@link Locator}.
	 * <p>
	 * Locator: Components are resolved by their full address provided by their {@link Locator}.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum LookupModes {
		Name, Locator
	}
	
	/**
	 * Sets the lookup mode for this manager. Legal modes are Name and Locator (full address).
	 * Locator mode enables manager to host components with the same name from different name spaces.
	 * Default is Name.
	 * 
	 * @param mode the lookup mode.
	 */
	public void setLookupMode(LookupModes mode);
	/**
	 * Sets parameter values for this manager. Parameters of a manager are used as default values for the parameters of the hosted components. Components may overwrite these defaults based on their own configuration.
	 * @param properties the parameters 
	 */
	public void setParameter(Properties properties);
	/**
	 * Adds a component to this manager by creating it from its XML configuration. The creation process is delegated to the {@link ComponentFactory} for component type resolution. Then the ComponentFactory delegates the creation process to the factory of the given component type.
	 * @param config the XML configuration of the component.
	 * @return the Component as created by the Component creation process.
	 * @throws CreationException if anything goes wrong in the creation process.
	 */
	public IComponent add(Element config) throws CreationException, RuntimeException;
	/**
	 * Adds an existing component to this manager
	 * @param component the component to add
	 * @return the component, that was replaced by the new component or NULL if no component was replaced.
	 */
	public IComponent add(IComponent component) throws RuntimeException;
	/**
	 * Removes a component from this manager.
	 * @param name the name of the component to be removed. If manager is in lookup mode Locator, the full address of the component as provided by {@link Locator#toString()}. 
	 * @return the removed component or NULL if no component with this name was found.
	 */
	public IComponent remove(String name);
	/**
	 * Gets a component from this manager by its name.
	 * @param name the name of the component. If manager is in lookup mode Locator, the full address of the component as provided by {@link Locator#toString()}. 
	 * @return the component of NULL if no such component is managed by this manager.
	 */
	public IComponent get(String name);
	/**
	 * Gets all components managed by this manager.
	 * @return an array of components.
	 */
	public IComponent[] getAll();
	/**
	 * Gets the first component of this manager.
	 * @return the component, which was the first component to be added to this manager or NULL if the manager is empty.
	 */
	public IComponent getFirst();
	/**
	 * Determines if there any components in this manager.
	 * @return true if manager is empty, false if there are components.
	 */
	public boolean isEmpty();
	
	/**
	 * Clears this manager
	 */
	public void clear();
}
