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

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;
import java.util.List;


import com.jedox.etl.core.context.IContext;
/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Manager extends Locatable implements IManager {
	
	private Properties parameter = new Properties();
	private Map<String, IComponent> lookup = Collections.synchronizedMap(new LinkedHashMap<String, IComponent>());
	private LookupModes mode = LookupModes.Name;
		
	public void setParameter(Properties properties) {
		//parameter = new Properties();
		if (properties != null) parameter.putAll(properties);
	}
	
	public Properties getParameter() {
		return parameter;
	}
	
	public void setLocator(Locator locator, IContext locatable) {
		super.setLocator(locator, locatable);
		if (locatable != null)
			setParameter(locatable.getParameter());
	}
	
	public void setLookupMode(LookupModes mode) {
		this.mode = mode;
	}
	
	/**
	 * gets the LookupMode of this Manager.
	 * @return the LookupMode
	 */
	protected LookupModes getLookupMode() {
		return mode;
	}
	
	public IComponent add(IComponent component) throws RuntimeException {
		synchronized(lookup) {
			if (component != null) {
				String name = null;
				switch (mode) { 
					case Name: name = component.getName(); break;
					case Locator: name = component.getLocator().toString(); break;
					default: name = component.getName();
				}
				IComponent old = lookup.get(name);
				//only set component dirty when in locator mode (which is true for the central repository)
				if ((old != null) && mode.equals(LookupModes.Locator)) {
					old.setDirty();
					fireConfigChanged(old.getLocator(),old.getConfigurator().getXML(),component.getConfigurator().getXML());
				}
				lookup.put(name, component);
				return old;
			}
		}
		return null;
	}
	
	/**
	 * Bulk-adds a list of components to this manager. 
	 * @param components the list of components to add.
	 */
	public void addAll(List<IComponent> components) throws RuntimeException {
		for (IComponent component : components) {
			add(component);
		}
	}
	
	public IComponent get(String name) {
		IComponent component = null;
		synchronized(lookup) {
			if (name != null) {
				component = lookup.get(name);
			}
		}
		return component;
	}
	
	public IComponent[] getAll() {
		synchronized(lookup) {
			return lookup.values().toArray(new IComponent[lookup.values().size()]);
		}
	}
	
	public IComponent getFirst() {
		try {
			synchronized(lookup) {
				return lookup.values().iterator().next();
			}
		}
		catch (Exception e) {
			return null;
		}
	}
	
	public void clear() {
		synchronized(lookup) {
			lookup.clear();
		}
	}
	
	
	public IComponent remove(String name) {
		synchronized(lookup) {
			IComponent component = lookup.remove(name);
			if (component != null) {
				component.setDirty();
				fireConfigChanged(component.getLocator(),component.getConfigurator().getXML(),null);
			}
			return component;
		}
	}
	
	public boolean isEmpty() {
		synchronized(lookup) {
			return lookup.keySet().isEmpty();
		}
	}
}
