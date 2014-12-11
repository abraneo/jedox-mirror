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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.sql.Types;
import java.text.NumberFormat;
import java.text.ParsePosition;
import com.jedox.etl.core.component.RuntimeException;

import com.jedox.etl.core.node.IColumn;

public class TypeConversionUtil {

	private Double getDouble (Object value, Double maxvalue) throws RuntimeException {
		Double d = Double.valueOf(convertToNumeric(value.toString()));
		if (Math.abs(d) > maxvalue)
			throw new RuntimeException("Number exceeds maximal value");
		return d;
	}
	
	/**
	 * converts the value of the input column to an object of the type defined by {@link IColumn#getValueType()}
	 * @param column the input column
	 * @return an object of the type defined by {@link IColumn#getValueType()}
	 * @throws FunctionException
	 */
	public Object convert(IColumn column) throws RuntimeException {
		Object value = column.getValue();
		try {
			if (value == null) return null;
			String classname = column.getValueType();
			Class<?> clazz = Class.forName(classname);
			if (clazz.isInstance(value)) //value is of type class or can be cast directly. Everything is fine.
				return clazz.cast(value);
			if (classname.equals(String.class.getCanonicalName()))
				return value.toString();
			if (classname.equals(Double.class.getCanonicalName()))
				return Double.valueOf(convertToNumeric(value.toString())); // recognize numbers like 55,3 as a double
			if (classname.equals(Integer.class.getCanonicalName()))
				return Integer.valueOf(getDouble(value,(double)Integer.MAX_VALUE).intValue());
			if (classname.equals(Long.class.getCanonicalName()))
				return Long.valueOf(getDouble(value,(double)Long.MAX_VALUE).longValue());
			if (classname.equals(Byte.class.getCanonicalName()))
				return Byte.valueOf(getDouble(value,(double)Byte.MAX_VALUE).byteValue());
			if (classname.equals(Float.class.getCanonicalName()))
				return Float.valueOf(convertToNumeric(value.toString()));
			if (classname.equals(Short.class.getCanonicalName()))
				return Short.valueOf(getDouble(value,(double)Short.MAX_VALUE).shortValue());
			if (classname.equals(BigInteger.class.getCanonicalName()))
				return new BigInteger(value.toString());
			if (classname.equals(BigDecimal.class.getCanonicalName()))
				return new BigDecimal(value.toString());
			return value;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to convert value "+value+" in column "+column.getName()+" to type "+column.getValueType());
		}
	}

	/**
	 * fast test if a given input data is numeric or not
	 * @param inputData the data to be tested for being numeric
	 * @return true, if numeric, false otherwise.
	 */
	public boolean isNumeric(String inputData) {
		  NumberFormat formatter = NumberFormat.getInstance();
		  ParsePosition pos = new ParsePosition(0);
		  formatter.parse(inputData, pos);
		  return inputData.length() == pos.getIndex();
	}
	
	/**
	 * converts input data to a numerical format with "." as decimal separator and without grouping separator e.g. 1.234,56 to 1234.56
	 * Default value is "0" for initial inputs 
	 * @param inputData the data to be converted
	 * @return the converted string  
	 */
	public String convertToNumeric(String inputData) {
		if (inputData==null || inputData.isEmpty())
			  return "0";
		if (inputData.equalsIgnoreCase(Boolean.TRUE.toString()))
			return "1";
		if (inputData.equalsIgnoreCase(Boolean.FALSE.toString()))
			return "0";
		if(inputData.contains(",") && inputData.indexOf(',') != inputData.length()-1
				&& inputData.indexOf(',')== inputData.lastIndexOf(',') ) {
			// replace the group separator
			// replace the comma with dot so that java will recognize the string as double
			return inputData.replaceAll("\\.", "").replace(",", ".");
		}
		else
			return inputData;			
	}
	
	/**
	 * map the sqlType to Java Type
	 * based on 
	 * http://docs.oracle.com/javase/6/docs/technotes/guides/jdbc/getstart/mapping.html#table1
	 * @param type sqltype as in java.sqlTypes
	 * @return
	 */
	 public static String convertSqlTypetoJavaClassName( int type ) {
		    String result = "java.lang.Object";

		    switch( type ) {
		      case Types.CHAR:
		      case Types.VARCHAR:
		      case Types.LONGVARCHAR:
		        result = "java.lang.String";
		        break;

		      case Types.NUMERIC:
		      case Types.DECIMAL:
		        result = "java.math.BigDecimal";
		        break;

		      case Types.BIT:
		        result = "java.lang.Boolean";
		        break;

		      case Types.TINYINT:
		      case Types.SMALLINT:
		      case Types.INTEGER:
		      //to avoid out of range exception, happened in sqlite
		      /*  result = "java.lang.Integer";
		        break;*/

		      case Types.BIGINT:
		        result = "java.lang.Long";
		        break;

		      case Types.REAL:
		        result = "java.lang.Float";
		        break;

		      case Types.FLOAT:
		      case Types.DOUBLE:
		        result = "java.lang.Double";
		        break;

		      case Types.BINARY:
		      case Types.VARBINARY:
		      case Types.LONGVARBINARY:
		        result = "java.lang.Byte[]";
		        break;

		      case Types.DATE:
		        result = "java.sql.Date";
		        break;

		      case Types.TIME:
		        result = "java.sql.Time";
		        break;

		      case Types.TIMESTAMP:
		        result = "java.sql.Timestamp";
		        break;
		    }

		    return result;
		  }
}
