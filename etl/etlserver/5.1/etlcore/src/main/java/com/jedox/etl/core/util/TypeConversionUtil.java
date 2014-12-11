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
package com.jedox.etl.core.util;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.sql.Types;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TypeConversionUtil {
	
	
	private TypeConversionUtil() {}

	private static Double getDouble (Object value, Double maxvalue) throws RuntimeException {
		Double d = convertToNumeric(value.toString());
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
	public static Object convert(IColumn column) throws RuntimeException {
		Class<?> clazz = column.getValueType();
		if(column.getValue()==null)
			return column.getValue();
		Object value;
		if (clazz.equals(String.class))
			value = column.getValueAsString();
		else
			value = column.getValue();
		try {
			if (value == null) 
				return null;
			if (clazz.isInstance(value)) //value is of type class or can be cast directly. Everything is fine.
				return clazz.cast(value);
			if (clazz.equals(String.class))
				return value.toString();
			if (clazz.equals(Double.class))
				return convertToNumeric(value.toString()); // recognize numbers like 55,3 as a double
			if (clazz.equals(Integer.class))
				return Integer.valueOf(getDouble(value,(double)Integer.MAX_VALUE).intValue());
			if (clazz.equals(Long.class))
				return Long.valueOf(getDouble(value,(double)Long.MAX_VALUE).longValue());
			if (clazz.equals(Byte.class))
				return Byte.valueOf(getDouble(value,(double)Byte.MAX_VALUE).byteValue());
			if (clazz.equals(Float.class))
				return Float.valueOf(getDouble(value,(double)Float.MAX_VALUE).floatValue());
			if (clazz.equals(Short.class))
				return Short.valueOf(getDouble(value,(double)Short.MAX_VALUE).shortValue());
			if (clazz.equals(BigInteger.class))
				return new BigInteger(value.toString());
			if (clazz.equals(BigDecimal.class))
				return new BigDecimal(value.toString());
			if (clazz.equals(Boolean.class))
				return new Boolean(value.toString().equalsIgnoreCase(Boolean.TRUE.toString()));
			// add primitive types
			if (clazz.equals(double.class))
				return (double)convertToNumeric(value.toString()); // recognize numbers like 55,3 as a double
			if (clazz.equals(int.class))
				return (int)(getDouble(value,(double)Integer.MAX_VALUE).intValue());
			if (clazz.equals(long.class))
				return (long)(getDouble(value,(double)Long.MAX_VALUE).longValue());
			if (clazz.equals(short.class))
				return (short)(getDouble(value,(double)Short.MAX_VALUE).longValue());
			if (clazz.equals(byte.class))
				return (byte)(getDouble(value,(double)Byte.MAX_VALUE).byteValue());
			if (clazz.equals(boolean.class))
				return value.toString().equalsIgnoreCase(Boolean.TRUE.toString());
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
	public static boolean isNumeric(String inputData) {
		 /*NumberFormat formatter = NumberFormat.getInstance();
		  ParsePosition pos = new ParsePosition(0);
		  formatter.parse(inputData, pos);
		  return inputData.length() == pos.getIndex();*/
		  return org.apache.commons.lang.math.NumberUtils.isNumber(inputData);
	}
	
	public static Double convertToNumeric(String inputData) throws NumberFormatException {
		return Double.valueOf(convertToNumericString(inputData));	
	}
	
	public static Double convertToNumeric(String inputData, Double defaultValue) {
		String numericString = convertToNumericString(inputData);
		try {
			return Double.valueOf(numericString);
		} catch (NumberFormatException e) {
			return defaultValue;
		}
	}
	
	/**
	 * converts input data to a numerical format with "." as decimal separator and without grouping separator e.g. 1.234,56 to 1234.56
	 * Default value is "0" for initial inputs 
	 * @param inputData the data to be converted
	 * @return the converted string  
	 */
	public static String convertToNumericString(String inputData) {
		String numericValue = inputData;
		if (inputData==null || inputData.isEmpty())
			  numericValue = "0";
		else if (inputData.equalsIgnoreCase(Boolean.TRUE.toString()))
			numericValue = "1";
		else if (inputData.equalsIgnoreCase(Boolean.FALSE.toString()))
			numericValue = "0";
		else if(inputData.contains(",") && inputData.indexOf(',') != inputData.length()-1
				&& inputData.indexOf(',')== inputData.lastIndexOf(',') ) {
			// replace the group separator
			// replace the comma with dot so that java will recognize the string as double
			numericValue = inputData.replaceAll("\\.", "").replace(",", ".");
		}
		return numericValue;
	}
	
	public static Object convertAttribute(IAttribute attribute, IElement e) throws NumberFormatException {
		Object value = e.getAttributeValue(attribute.getName());
		//if (value == null && attribute.getMode().equals(AttributeModes.ALIAS)) value = e.getName();
		if (attribute.getType().equals(ElementType.ELEMENT_NUMERIC)) {
			return convertToNumeric(value!=null?value.toString():null);
		} else {
			return value;
		}
	}
	
	/**
	 * map the sqlType to Java Type
	 * based on 
	 * http://docs.oracle.com/javase/6/docs/technotes/guides/jdbc/getstart/mapping.html#table1
	 * @param type sqltype as in java.sqlTypes
	 * @return
	 */
	 public static Class<?> convertSqlTypetoJavaClass( int type ) {
		    Class<?> result = Object.class;

		    switch( type ) {
		      case Types.CHAR:
		      case Types.VARCHAR:
		      case Types.LONGVARCHAR:
		        result = String.class;
		        break;

		      case Types.NUMERIC:
		      case Types.DECIMAL:
		        result = BigDecimal.class;
		        break;

		      case Types.BIT:
		        result = Boolean.class;
		        break;

		      case Types.TINYINT:
		      case Types.SMALLINT:
		      case Types.INTEGER:
		      //to avoid out of range exception, happened in sqlite
		      /*  result = "java.lang.Integer";
		        break;*/

		      case Types.BIGINT:
		        result = Long.class;
		        break;

		      case Types.REAL:
		        result = Float.class;
		        break;

		      case Types.FLOAT:
		      case Types.DOUBLE:
		        result = Double.class;
		        break;

		      case Types.BINARY:
		      case Types.VARBINARY:
		      case Types.LONGVARBINARY:
		        result = Byte[].class;
		        break;

		      case Types.DATE:
		        result = java.sql.Date.class;
		        break;

		      case Types.TIME:
		        result = java.sql.Time.class;
		        break;

		      case Types.TIMESTAMP:
		        result = java.sql.Timestamp.class;
		        break;
		    }

		    return result;
		  }
}
