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

import java.util.Vector;

import com.jedox.etl.core.util.NamingUtil;

/**
 * Class, which holds all information necessary for addressing {@link ILocatable Locatables}.
 * Setting properties of the locator does not change anything within the addressed Locatable. It does change the address, which may then point to a different Locatable.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Locator implements Cloneable {
	
	public final static String deleted = "deleted";

	private Vector<String> path = new Vector<String>();
	private String context = "";
	private String sessioncontext = "";
	private String schema;
	private String table;
	private String quote = "\"";
	private Locator origin;

	/**
	 * creates a Locator from a string representation
	 * @param locator
	 * @return the new Locator
	 */
	public static Locator parse(String locator) {
		Locator loc = new Locator();
		if (locator != null) {
			String[] hops = locator.split("\\.");
			for (int i=0; i<hops.length; i++) {
				if (hops[i].length() > 0)
					loc.path.add(hops[i].trim());
			}
		}
		return loc;
	}
	
	/**
	 * creates a Locator from a string representation
	 * @param locator
	 * @return the new Locator
	 */
	public static Locator parseGroup(String locator) {
		Locator loc = new Locator();
		if (locator != null) {
			loc.path.add(locator);
			loc.path.add(NamingUtil.group_manager);
		}
		return loc;
	}

	/**
	 * determines, if this locator addresses a {@link IManager manager}
	 * @return true if so, false otherwise
	 */
	public boolean isManager() {
		return path.size() % 2 == 0;
	}

	/**
	 * determines, if this locator addresses a {@link IComponent component}
	 * @return true if so, false otherwise
	 */
	public boolean isComponent() {
		return path.size() % 2 != 0;
	}

	/**
	 * determines, if this locator is a basic {@link IComponent component}. A component is considered as basic, when it is managed by a root level {@link IManager manager} of a {@link com.jedox.etl.core.project.Project project}.
	 * @return true, if so, false otherwise
	 */
	public boolean isBasicComponent() {
		return path.size() == 3;
	}

	/**
	 * determines if this locator is empty. By convention an empty locator references to the global {@link com.jedox.etl.core.project.ProjectManager}
	 * @return true if so, false otherwise
	 */
	public boolean isEmpty() {
		return path.isEmpty();
	}

	/**
	 * determines if this locator is on root level. By convention root level locators address {@link com.jedox.etl.core.project.IProject projects}.
	 * @return true if so, false otherwise
	 */
	public boolean isRoot() {
		return path.size() == 1;
	}

	/**
	 * gets the root name of the locator. By convention the root name of the locator is the name of the {@link com.jedox.etl.core.project.IProject project}.
	 * @return the root name, which is the name of the project
	 */
	public String getRootName() {
		if (path.size() > 0) return path.get(0);
		return null;
	}

	public void setRootName(String rootName) {
		if (path.size() > 0) {
			path.remove(0);
		}
		path.add(0, rootName);
	}

	/**
	 * gets the name of the locator, which is the last part of the address. This corresponds to {@link ILocatable#getName()} of the addressed locatable.
	 * @return the name of the locatable
	 */
	public String getName() {
		/*	cschw: changed this on 29.01.2014. Use getPersistentTable to get the tableName with fallback on name....
		if(this.table!=null)
			return table;
		*/
		if (path.size() > 0) return path.get(path.size()-1);
		return null;
	}

	/**
	 * gets the root locator. By convention this is the locator addressing the {@link com.jedox.etl.core.project.IProject project} of the locatable addressed by this locator.
	 * @return the project locator
	 */
	public Locator getRootLocator() {
		Locator loc = clone();
		loc.path.clear();
		if (!path.isEmpty()) {
			loc.add(path.firstElement());
		}
		return loc;
	}

	/**
	 * gets the parent locator of this locator. If the locatable addressed by this locator is a component the result addresses its manager. If the locator addresses a manager, the result addresses its enclosing component.
	 * @return the parent locator
	 */
	public Locator getParentLocator() {
		Locator loc = clone();
		loc.reduce();
		return loc;
	}

	/**
	 * Adds a name of a locatable to this locator. If the original locator addresses a manager then the resulting locator addresses the component with the given name managed by this manager. If the original locator addresses a component the resulting manager addresses a manager of this component with the given name.
	 * @param locatable the name of the locatable to add to the locator.
	 * @return the modified locator
	 */
	public Locator add(String locatable) {
		if (locatable != null)
			path.add(locatable.trim());
		return this;
	}

	/**
	 * Reduces the locator. If the original locator addresses a component the resulting manager addresses its manager. If the original locator addresses a manager the resulting locator addresses its enclosing component.
	 * @return the modified locator
	 */
	public Locator reduce() {
		if (!path.isEmpty())
			path.remove(path.size()-1);
		return this;
	}

	/**
	 * Clones the locator creating an exact copy.
	 */
	public Locator clone() {
		Locator locator = new Locator();
		locator.path.addAll(path);
		locator.context = getContext();
		locator.sessioncontext = getSessioncontext();
		locator.schema = schema;
		locator.table = table;
		locator.quote = quote;
		return locator;
	}

	/**
	 * gets the string representation of the locator. Each name in the path is separated by a "."
	 */
	public String toString() {
		StringBuffer buffer = new StringBuffer();
		for (String s : path) {
			buffer.append(".");
			buffer.append(s);
		}
		if (buffer.length() > 0) buffer.deleteCharAt(0);
		return buffer.toString();
	}

	/**
	 * gets the path of the locator. Each element of the vector represents a name in the manager-component hierarchy
	 * @return the vector containing the path.
	 */
	public Vector<String> getPath() {
		return path;
	}

	/**
	 * gets the type of the locator.
	 * @return "manager" if the locator addresses a manager. The component {@link ITypes.Components type} if the locator addresses a component.
	 */
	public String getType() {
		if (isEmpty())
			return "manager";
		if (isRoot())
			return ITypes.Components.project.toString();
		if (isComponent()) {
			String manager = path.get(path.size()-2);
			return manager.substring(0, manager.length()-1);
		}
		return "manager";
	}

	/**
	 * gets the name of the manager of this locator. If the locator addresses a manager the result is identical to {@link #getName()}. If the locator addresses a component the result is the name of the manager managing this component.
	 * @return the manager name
	 */
	public String getManager() {
		if (isEmpty() || isRoot())
			return ITypes.Projects;
		if (isManager())
			return getName();
		if (isComponent())
			return path.get(path.size()-2);
		return "unknown";
	}

	/**
	 * sets the name of the manager. If the locator addresses a component this changes the name of the manager managing this component. If the locator addresses a manager this changes the name of the manager.
	 * @param manager the new name of the manager to set
	 * @return
	 */
	public Locator setManager(String manager) {
		if (!isRoot() && !isEmpty() && isComponent()) {
			path.set(path.size()-2, manager);
		}
		if (!isEmpty() && isManager())
			reduce().add(manager);
		return this;
	}

	/**
	 * sets the name of the {@link com.jedox.etl.core.context.IContext context} of the locatable. The context is relevant for calculating the {@link #getPersistentName() persistent} name of the locatable.
	 * @param context the name of the context.
	 */
	public void setContext(String context) {
		this.context = context;
	}

	/**
	 * gets the name of the {@link com.jedox.etl.core.context.IContext context} this locatable lives in
	 * @return the name of the context
	 */
	public String getContext() {
		return context;
	}

	/**
	 * gets the persistent name of the locatable under which its data is saved in the internal {@link com.jedox.etl.core.persistence.IPersistence persistence}.
	 * the persistent name consists of the {@link #getPersistentSchema() schema} name and the name of the locatable separated by a "."
	 * @return the persistent name as used by the persistence layer.
	 */
	public String getPersistentName() {
		return getPersistentName(quote);
	}

	/**
	 * gets the persistent name of the locatable under which its data is saved in the internal {@link com.jedox.etl.core.persistence.IPersistence persistence}.
	 * the persistent name consists of the {@link #getPersistentSchema() schema} name and the name of the locatable separated by a "."
	 * @return the persistent name as used by the persistence layer.
	 */
	public String getPersistentName(String quote) {
		return ("".equals(getPersistentSchema())) ? NamingUtil.escape(getPersistentTable() ,quote) : NamingUtil.escape(getPersistentSchema(),quote)+"."+NamingUtil.escape(getPersistentTable(),quote);
	}

	/**
	 * sets the persistent schema name. This can be used to override the default schema name calculation, which is calculated from the {@link #getRootName() root} name and the {@link #getContext() context} name separated by a "_". Only use this, if you really know what you are doing, since the non-careful may mess up the persistence layer here.
	 * @param schema the persistent schema name
	 */
	public void setPersistentSchema(String schema) {
		this.schema = schema;
	}

	/**
	 * gets the persistent schema name. If not {{@link #setPersistentSchema(String) overridden} the schema name is calculated from the {@link #getRootName() root} name and the {@link #getContext() context} name separated by a "_".
	 * @return the persistent schema name.
	 */
	public String getPersistentSchema() {
		return (schema == null) ? getRootName()+"_"+getContext() : schema;
	}
	
	/**
	 * sets the persistent table name. This can be used to override the default table name calculation, which is calculated from the {@link #getName() root} name and the {@link #getContext() context} name. Only use this, if you really know what you are doing, since the non-careful may mess up the persistence layer here.
	 * @param table the persistent table name
	 */
	public void setPersistentTable(String table) {
		this.table = table;
	}

	/**
	 * gets the persistent table name. Table can be null, only needed for drillthrough locators.
	 * @return the persistent table name.
	 */
	public String getPersistentTable() {
		return table != null ? table : getName();
	}

	public void setQuote(String quote) {
		this.quote = quote;
	}


	/**
	 * @param sessioncontext the sessioncontext to set
	 */
	public void setSessioncontext(String sessioncontext) {
		this.sessioncontext = sessioncontext;
	}

	/**
	 * @return the sessioncontext
	 */
	public String getSessioncontext() {
		return sessioncontext;	
	}
	
	public String getDisplayName() {
		return getType()+" "+getName();		
	}

	/**
	 * @param oldLoc the oldLoc to set
	 */
	public void setOrigin(Locator oldLoc) {
		origin = oldLoc;
	}

	/**
	 * @return the oldLoc
	 */
	public Locator getOrigin() {
		return origin;
	}
	
}
