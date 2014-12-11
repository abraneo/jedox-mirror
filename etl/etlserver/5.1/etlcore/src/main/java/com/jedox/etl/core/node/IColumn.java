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
package com.jedox.etl.core.node;

import com.jedox.etl.core.aliases.IAliasElement;


/**
 * Interface for the basic functionality of a single data column.
 * In ETL-Server all data is stored internally in Columns organized as {@link Row Rows}.
 * Generally null values and empty Strings should be treated equivalently.
 * This data usually is processed with the help of {@link com.jedox.etl.core.source.processor.IProcessor Processors}.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IColumn extends INamedValue<Object> {

	/**
	 * Gets the java data type, the value is internally represented in. The default type is java.lang.String.
	 * @return the java data type.
	 */
	public Class<?> getValueType();
	/**
	 * Sets the java data type the internal value should be represented in.
	 * @param type the name of the java data type, e.g. java.lang.Double. Default type is java.lang.String.
	 */
	public void setValueType(Class<?> type);
	public void setAliasElement(IAliasElement aliasElement);
	public IAliasElement getAliasElement();
	public boolean isEmpty();
	/**
	 * Configures this column to mimic a other column in all its internal settings.
	 * @param source the column to mimic.
	 */
	public void mimic(IColumn source);
	
}
