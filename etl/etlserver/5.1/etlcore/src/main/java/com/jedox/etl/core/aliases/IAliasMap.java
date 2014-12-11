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
import java.util.Set;

import com.jedox.etl.core.component.IComponent;

/**
 * Interface for Alias-Maps. 
 * An Alias-Map object is responsible for mapping column numbers for data stored in a table based source to names used for further accessing this data in a ETL-Server project.
 * Example:
 * <pre>
 * {@code
   <alias_map>
        <alias name="CustomerID" default="Customer n.a.">1</alias>
        <alias name="CustomerName">2</alias>
        <alias name="City">3</alias>
        <alias name="PLZ">4</alias>
        <alias name="CountryID" default="Country n.a.">5</alias>
        <alias name="CountryName">6</alias>
   </alias_map>
   }
   </pre>
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IAliasMap extends IComponent, Cloneable {
	
	/**
	 * maps a single AliasMapElement
	 * @param element the element to be mapped.
	 */
	public void map(AliasMapElement element);
	/**
	 * gets an AliasMapElement by its name.
	 * @param name the name of the Element
	 * @return the Element
	 */
	public AliasMapElement getElement(String name);
	/**
	 * determines if this alias map contains a particular alias in a mapping.
	 * @param alias the name of the alias
	 * @return true if this alias is contained, false otherwise
	 */
	public boolean hasAlias(String alias);	
	/**
	 * determines if this alias map contains for a particular column number a mapping.
	 * @param alias the name of the alias
	 * @return true if this alias is contained, false otherwise
	 */
	public boolean hasAlias(int column);	
	/**
	 * gets the column number mapped to an alias name
	 * @param alias the alias name
	 * @return the mapped column number 
	 */
	public int getCol(String alias);
	/**
	 * gets the set of all mapped aliases
	 * @return the aliases
	 */
	public Set<String> getAliases();
	/**
	 * gets the number of aliases mapped.
	 * @return the number of aliases mapped.
	 */
	public int getAliasCount();
	/**
	 * clones this Alias-Map by performing a deep copy.
	 * @return a new identical Alias-Map.
	 */
	public IAliasMap clone();
	/**
	 * gets the name of an alias for a particular mapped column
	 * @param column the column number to get the mapped alias name for.
	 * @param defaultAlias the default name to return if no mapping was made for this column.
	 * @return the mapped alias
	 */
	public String getAlias(int column, String defaultAlias);
	/**
	 * Determines if this Alias-Map is contains mappings.
	 * @return true if empty, false otherwise.
	 */
	public boolean isEmpty();
	/**
	 * shifts all mappings by a given offset. this is useful, when columns are added or removed when the mapping was already made and should keep the mapping consistent in such a case.
	 * @param offset
	 */
	public void shift(int offset);
	
	public boolean hasDefaultValues();
	public Set<String> getAliasesOrigin();
}
