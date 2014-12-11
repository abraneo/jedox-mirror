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
package com.jedox.etl.core.aliases;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.util.NamingUtil;

/**
 * Data class used in {@link IAliasMap} to map column numbers to name and provide extra information such as default values.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class AliasMapElement implements Cloneable, Comparable<AliasMapElement>, IAliasElement {
	private static final Log log = LogFactory.getLog(AliasMapElement.class);
	
	private String name;
	private int column = 0;
	private String defaultValue = null;
	private String origin;
	
	public AliasMapElement() {
	}
	
	public AliasMapElement(String name, int column) {
		this.name = name;
		this.column = column;
	}
	
	/**
	 * gets the column number of this mapping
	 * @return the column number
	 */
	public int getColumn() {
		return column;
	}
	
	/**
	 * sets the column number of this mapping
	 * @param column the column number
	 */
	public void setColumn(int column) {
		this.column = column;
	}
	
	/**
	 * sets the column number of this mapping from a string. convenience function.
	 * @param column the column number as string.
	 */
	public void setColumn(String column) {
		try {
			int i = Integer.parseInt(column);
			setColumn(i);
		}
		catch (Exception e) {
			log.error("Failed to map alias "+getName()+" to column "+column+": "+e.getMessage());
			log.debug(e);
		}
	}
	
	/**
	 * gets the default value for this mapping which is used when no value is defined in the mapped column.
	 * @return the default value
	 */
	public String getDefaultValue() {
		return defaultValue;
	}
	
	/** 
	 * gets the default value for this mapping which is used when no value is defined in the mapped column.
	 * @param defaultValue the default value
	 */
	public void setDefaultValue(String defaultValue) {
		if (defaultValue != null)
			this.defaultValue = defaultValue;
	}
	
	private String getInternalDefaultName() {
		return NamingUtil.internal(String.valueOf(getColumn()));
	}
	
	/**
	 * gets the name set for this mapping. This name serves as alias for the column number.
	 * @return the name
	 */
	public String getName() {
		if (name == null || name.isEmpty()) {
			return getInternalDefaultName();
		}
		return name;
	}
	
	/**
	 * Sets the name set for this mapping. This name serves as alias for the column number.
	 * @param name the name
	 */
	public void setName(String name) {
		this.name = name;
	}
	
	/**
	 * delivers an exact clone of this mapping.
	 */
	public AliasMapElement clone() {
		AliasMapElement element = new AliasMapElement();
		//Todo: Check the name logic as getName() may return value if name is null 
		if (hasName())
			element.setName(getName());
		element.setColumn(getColumn());
		element.setDefaultValue(getDefaultValue());
		element.setOrigin(getOrigin());
		return element;
	}
	
	/**
	 * compares this mapping to with the mapping provided using the column number for comparison.
	 */
	public int compareTo(AliasMapElement element) {
		return getColumn() - element.getColumn();
	}

	public void setOrigin(String origin) {
		this.origin = origin;
	}

	public String getOrigin() {
		return origin;
	}
	
	public boolean hasName() {
		return !(name == null || name.isEmpty()); 
	}

}
