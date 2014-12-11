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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.node;

import com.jedox.palojlib.interfaces.IElement.ElementType;

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
	 * Enumerates the available types / roles of columns. The type of column specifies the semantic (origin or intended use) the data stored in this column has.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum ColumnTypes {
		/**
		 * general data with no further semantic. the default role
		 */
		data,
		/**
		 * data serves as attribute in an OLAP Dimension or related usage
		 */
		attribute,
		/**
		 * data serves as alias in an OLAP Dimension or related usage. Similar to attribute, but with technical name as default value, if value is not set
		 */
		alias,
		/**
		 * data serves as rule for an OLAP Cube or related usage.
		 */
		rule,
		/**
		 * data serves as additional internal information available via drillthrough
		 */
		annex,
		/**
		 * data serves as coordinate in a multi-dimensional data vector
		 */
		coordinate,
		/**
		 * data serves as level in tree data structure
		 */
		level,
		/**
		 * data serves as value in a multi-dimensional data vector
		 */
		value,
		/**
		 * data calculated by a function
		 */
		function,
		/**
		 * data serving as key in an relational update
		 */
		key,
		/**
		 * data to be kept / ignored in an relational update
		 */
		first,
		/**
		 * data to be put / replaced in an relational update
		 */
		last,
		/**
		 * data to be added / summed in an relational update
		 */
		sum,
		/**
		 * data to be counted in an relational update
		 */
		count,
		/**
		 * data, where the minimum shall be calculated from in an relational update
		 */
		min,
		/**
		 * data where the maximum shall be calculated from in an relational update
		 */
		max,
		/**
		 * data o be averaged in an relational update
		 */
		avg,
		/**
		 * data serving as primary key
		 */
		primary


	}

	/**
	 * Gets the java data type, the value is internally represented in. The default type is java.lang.String.
	 * @return the java data type.
	 */
	public String getValueType();
	/**
	 * Sets the java data type the internal value should be represented in.
	 * @param type the name of the java data type, e.g. java.lang.Double. Default type is java.lang.String.
	 */
	public void setValueType(String type);
	/**
	 * Sets a default value for this column. Whenever the value is null or the empty string "", the default value used instead. Default is null.
	 * @param value the value to use a default.
	 */
	public void setDefaultValue(String value);

	/**
	 * Gets the default value set for this column. Whenever the value is null or the empty string "", the default value used instead. Default is null.
	 * @return the default value
	 */
	public String getDefaultValue();

	/**
	 * Determines if this column is empty meaning the the value is null or an empty String.
	 * @return true if empty, false otherwise.
	 */

	public boolean isEmpty();
	/**
	 * Gets the ColumnType of this column.
	 * @return the column type specifying the intended semantic.
	 */
	public ColumnTypes getColumnType();
	/**
	 * Gets the ElementType of this column giving the target data structure a hint, on how the received data should be processed.
	 * @return the ElementType of this column.
	 */
	public ElementType getElementType();
	/**
	 * Sets the ElmentType of this column giving the target data structure a hint, on how the received data should be processed.
	 * @param type the ElementType of this column.
	 */
	public void setElementType(String type);

	/**
	 * Configures this column to mimic a other column in all its internal settings.
	 * @param source the column to mimic.
	 */
	public void mimic(IColumn source);

	/**
	 * Gets the ElementType of this column giving the target data structure a hint, on how the received data should be processed.
	 * @return the ElementType of this column.
	 */
	public ElementType getElementType(ElementType defaulttype);
	
	/**
	 * Gets the scale parameter which represents the number of digits to the right of the decimal point (especially for Big Decimal)
	 * @return
	 */
	public int getScale();
	
	/**
	 * Sets he scale parameter which represents the number of digits to the right of the decimal point (especially for Big Decimal)
	 * @param scale
	 */
	public void setScale(int scale);

	
}
