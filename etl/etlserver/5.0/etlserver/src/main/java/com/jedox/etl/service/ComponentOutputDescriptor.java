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
package com.jedox.etl.service;

/**
 * Data class for the description of the output of a table based {@link com.jedox.etl.core.source.ISource source}. Each column of a table based source is represented by a single ComponentOutputDescriptor. 
 * Setting properties of a descriptor does not change the properties of the described column. The application is responsible for initializing the descriptor properties accordingly. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ComponentOutputDescriptor implements Comparable<ComponentOutputDescriptor> {
	
	private String name;
	private String originalName;
	private int position;
	private String type;
	private String role = "data";
	private String defaultValue;
	
	/**
	 * gets the name of the output column
	 * @return the name
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * sets the descriptor name of the output column
	 * @param name the name to set
	 */
	public void setName(String name) {
		this.name = name;
	}
	
	/**
	 * gets the position (column index) of the output column 
	 * @return the position
	 */
	public int getPosition() {
		return position;
	}
	
	/**
	 * sets the descriptor position (column index)
	 * @param position the position
	 */
	public void setPosition(int position) {
		this.position = position;
	}
	
	/**
	 * gets the data type of the output column as canonical name of a java class (e.g. java.lang.String)
	 * @return the data type
	 */
	public String getType() {
		return type;
	}
	
	/**
	 * sets the descriptor data type as canonical name of a java class (e.g. java.lang.String)
	 * @param type the data type
	 */
	public void setType(String type) {
		this.type = type;
	}
	
	/**
	 * gets the default value, which is returned by this column, when no value is set. 
	 * @return the default value
	 */
	public String getDefaultValue() {
		return defaultValue;
	}
	
	/**
	 * sets the descriptor default value 
	 * @param defaultValue
	 */
	public void setDefaultValue(String defaultValue) {
		this.defaultValue = defaultValue;
	}
	
	/**
	 * gets the {@link com.jedox.etl.core.node.IColumn.ColumnTypes role} of the output column
	 * @return the role as string
	 */
	public String getRole() {
		return role;
	}
	
	/**
	 * sets the descriptor {@link com.jedox.etl.core.node.IColumn.ColumnTypes role} as string
	 * @param role the role
	 */
	public void setRole(String role) {
		this.role = role;
	}
	
	public int compareTo(ComponentOutputDescriptor c) {
		return getPosition() - c.getPosition();
	}

	public void setOriginalName(String originalName) {
		this.originalName = originalName;
	}

	public String getOriginalName() {
		return originalName;
	}

}
