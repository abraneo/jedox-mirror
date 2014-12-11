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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Properties;

import com.jedox.etl.core.util.NamingUtil;
/**
 * Internal class for component registration, which holds the description of a single component defined in a component.xml 
 * @author gerhard
 *
 */
public class ComponentDescriptor implements Comparable<ComponentDescriptor> 
{
	//private static final Log log = LogFactory.getLog(ComponentDescriptor.class);	
	
	private String name;
	private String qname;
	private String className;
	private boolean def;
	private String[] drivers;
	private String type;
	private String status;
	private String caption;
	private String[] jdbcs;
	private ArrayList<String> modes = new ArrayList<String>();
	private String[] connections;//added by kais (connections that are allowed to be used in this component)
	private Properties parameter = new Properties();
	
	/**
	 * Constructor
	 * @param name the name of the component 
	 * @param className the canonical name of the implementing class
	 * @param scope the {@link ITypes.Components scope} 
	 * @param def true if this component is the default component for this scope. 
	 * @param status the status (production, experimental, deprecated)
	 * @param caption the display name for this component
	 * @param connectionList list of allowed connection (needed only in loads)
	 * @param modes list of modes e.g. server,embedded (needed only in connections)
	 */
	public ComponentDescriptor(String name, String className, String scope, boolean def, String caption, String connectionList,String modes, String status) {
		super();
		this.type = scope;
		this.setName(name);
		this.className = className;
		this.def = def;
		this.status = status;
		this.caption = caption;
		if(modes!=null){
			this.modes.addAll(Arrays.asList(modes.split(","))); 
			drivers = new String[this.modes.size()];
			jdbcs = new String[this.modes.size()];
		}else{
			drivers = new String[1];
			jdbcs = new String[1];
		}
			
		if(connectionList!=null)
			this.connections = connectionList.split(",");
	}
	
	/**
	 * gets the qualified name for this component (scope.name)
	 * @return the qualified name
	 */
	public String getQName() {
		return qname;
	}

	/**
	 * gets the name of this component
	 * @return the name
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * gets the name of jdbc driver for this component (added by Kais)
	 * @return the jdbc driver name
	 * @throws CreationException 
	 */
	public String getJDBC(String mode) throws CreationException {
		if(modes.size()==0)
			return jdbcs[0];
		else{
			if(mode==null)
				throw new CreationException("Connection of type " + name + " should include a mode.");
			int index = modes.indexOf(mode);
			if(index==-1)
				throw new CreationException("Connection of type " + name + " can not have mode " + mode);
			return jdbcs[index];
		}
	}
	
	/**
	 * determines if this component is intended for productional use
	 * @return true if so, false otherwise.
	 */
	public boolean isProductional() {
		return !(isExperimental() || isDeprecated());
	}
	
	/**
	 * determines if this component is still experimental
	 * @return true if so, false otherwise
	 */
	public boolean isExperimental() {
		return "experimental".equalsIgnoreCase(status);
	}
	
	/**
	 * determines if this component is deprecated and will be removed soon.
	 * @return true if so, false otherwise
	 */
	public boolean isDeprecated() {
		return "deprecated".equalsIgnoreCase(status);
	}

	/**
	 * sets the name of the component
	 * @param name the name
	 */
	private void setName(String name) {
		this.name = name;
		this.qname = type + '.' + name;
	}

	/**
	 * gets the class name of the component in canonical form
	 * @return the class name
	 */
	public String getClassName() {
		return className;
	}
	
	/**
	 * adds a parameter for this component serving as a default parameter
	 * @param driver
	 */
	public void addParameter(String name, String value) {
		if(modes.size() == 0)
			if(name.equals(NamingUtil.internalPrefix() + "jdbc"))
				jdbcs[0] = (jdbcs[0]==null?value:jdbcs[0]+value);
			else if(name.equals(NamingUtil.internalPrefix() + "driver"))
				drivers[0] = (drivers[0]==null?value:drivers[0]+value);
			else
				parameter.put(name, (parameter.get(name)!=null?parameter.get(name):"") +value);
		else{	
			for(int i=0;i<modes.size();i++){
				if(name.equals(NamingUtil.internalPrefix() + modes.get(i) + "_jdbc")){
					jdbcs[i] = (jdbcs[i]==null?value:jdbcs[i]+value);
					return;
				}
				else if(name.equals(NamingUtil.internalPrefix() + modes.get(i) + "_driver")){
					drivers[i] = (drivers[i]==null?value:drivers[i]+value);
					return;
				}
			}
			parameter.put(name, (parameter.get(name)==null?"":parameter.get(name)) + value);
		}
	}
	
	/**
	 * gets the parameters for this component serving as default parameters
	 * @return
	 */
	public Properties getParameters() {
		return parameter;
	}
	
	/**
	 * gets the driver of this component
	 * @return the driver
	 * @throws CreationException 
	 */
	public String getDriver(String mode) throws CreationException {
		if(modes.size()==0)
			return drivers[0];
		else{
			if(mode==null)
				throw new CreationException("Connection of type " + name + " should include a mode.");
			int index = modes.indexOf(mode);
			if(index==-1)
				throw new CreationException("Connection of type " + name + " can not have mode " + mode);
			return drivers[index];
		}
	}
	
	/**
	 * gets All possible drivers of this component
	 * @return the list of all possible drivers
	 * @throws CreationException 
	 */
	public String[] getAllDrivers() throws CreationException {
		return drivers;
	}
	
	/**
	 * gets the scope of this component
	 * @return the scope 
	 */
	public String getScope() {
		return type;
	}
	
	/**
	 * gets the connections types that can be used in this component.
	 * special cases:
	 * null means no connections
	 * one entry with value "*" means all connection types are accepted
 	 * @return the scope 
	 */
	public String[] getAllowedConnectionTypes() {
		return connections;
	}
	
	/**
	 * gets the display name of this component
	 * @return the display name
	 */
	public String getCaption() {
		return caption;
	}

	public String toString() {
		StringBuffer sb = new StringBuffer();
		sb.append(name).append(',').append(className).append('[');
		sb.append(type);
		sb.append(']');
		return sb.toString();
	}

	/**
	 * determines if this component is the default one for this scope. when a requested component is not found in a scope the default component is returned instead.
	 * @return true if so, false otherwise
	 */
	public boolean isDefault() {
		return def;
	}

	/**
	 * sets this component as the default component for this scope. when a requested component is not found in a scope the default component is returned instead.
	 * @param def true, if default
	 */
	public void setDefault(boolean def) {
		this.def = def;
	}

	@Override
	public int compareTo(ComponentDescriptor c) {
		return getName().compareTo(c.getName());
	}
	
	

}
