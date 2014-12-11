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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source;

import java.util.Properties;
import java.util.HashMap;
import java.util.ArrayList;

import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locatable;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;

/**
 * A Manager Proxy for other source managers, which allows transparent access to sources independent of which particaular type they are (extracts, transforms)
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CompositeSourceManager extends Locatable implements IManager {

	private Properties parameter = new Properties();
	private HashMap<String,SourceManager> managers = new HashMap<String, SourceManager>(); 
	private LookupModes mode = LookupModes.Name;
	
	/**
	 * Adds a dependent source manager to this proxy
	 * @param manager the manager
	 */
	public void addManager(SourceManager manager) {
		managers.put(manager.getName(), manager);
	}
	
	private SourceManager getManager(String name) {
		SourceManager manager = managers.get(name);
		if (manager == null) 
			manager = managers.get(getName());
		return manager;
	}
	
	/**
	 * creates a new Source via this proxy by delegation to a registered manager of the appropriate type.
	 */
	public ISource add(Element config) throws CreationException, RuntimeException {
		SourceManager manager = getManager(config.getName()+"s");
		if (manager != null) {
			return manager.add(config);
		} else throw new CreationException("Manager "+config.getName()+"s not found."); 
	}

	public ISource add(IComponent component) throws RuntimeException {
		SourceManager manager = getManager(component.getLocator().getManager());
		if (manager != null)
			return manager.add(component);
		throw new RuntimeException("Manager not found: "+component.getLocator().getManager());
	}

	public void clear() {
		for (SourceManager manager : managers.values()) {
			manager.clear();
		}
	}

	public ISource get(String name) {
		if (mode.equals(LookupModes.Locator)) {
			Locator loc = Locator.parse(name);
			for (SourceManager manager : managers.values()) {
				loc.setManager(manager.getName());
				ISource source = manager.get(loc.toString());
				if (source != null)
					return source;
			}
		} 
		else { 
			for (SourceManager manager : managers.values()) {
				ISource source = manager.get(name);
				if (source != null)
					return source;
			}
		}
		return null;
	}

	public ISource[] getAll() {
		ArrayList<ISource> result = new ArrayList<ISource>();
		for (SourceManager manager : managers.values()) {
			result.addAll(manager.getSources());
		}
		return result.toArray(new ISource[result.size()]);
	}

	public IComponent getFirst() {
		ISource[] all = getAll();
		if (all.length > 0)
			return all[0];
		return null;
	}

	public boolean isEmpty() {
		ISource[] all = getAll();
		return (all.length == 0);
	}

	public IComponent remove(String name) {
		for (SourceManager manager : managers.values()) {
			ISource source = manager.remove(name);
			if (source != null)
				return source;
		}
		return null;
	}

	public void setLookupMode(LookupModes mode) {
		this.mode = mode;
		for (SourceManager manager : managers.values()) {
			manager.setLookupMode(mode);
		} 
	}

	public void setParameter(Properties properties) {
		parameter = properties;
	}

	public Properties getParameter() {
		return parameter;
	}
	
	public String getName() {
		return ITypes.Sources;
	}

}
