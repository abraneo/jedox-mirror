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
package com.jedox.etl.components.function;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.DateFormater;

import java.util.Date;
import java.text.ParseException;
import java.text.SimpleDateFormat;


/**
 * Sets the particular kind of format a date or timestamp value should be converted to match the elements of the dimension addressed by this TimeNode.
 * The format String may contain contain the following letters and arbitrary delimiters between them
 * Y..encodes the year
 * Q..encodes the quarter
 * M..encodes the months
 * W..encodes the week
 * D..encodes the day
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 */
public class DateDimension extends Function {
	private String sourceformat;
	private String targetformat;
	
	/**
	 * Gets the resolved value according to the template and the actual input.
	 * @return the resolved value 
	 */	
	public Object transform(Row input) throws FunctionException {
		try {
			Object date = input.getColumn(0).getValue();
			if (date != null) {
				if (date instanceof Date)
					return DateFormater.getInstance().parseDate((Date) date, targetformat);
				//assume a String or numeric format
				SimpleDateFormat sdf = new SimpleDateFormat(sourceformat);
				return DateFormater.getInstance().parseDate(sdf.parse(date.toString()), targetformat);
			}
			return date;
		}
		catch (ParseException e) {
			throw new FunctionException(input.getColumn(0).getName(),input.getColumn(0).getValueAsString(),e.getMessage());
		}
		
	}
	
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,false);
	}
	
	protected void validateParameters() throws ConfigurationException {
		//get the format the date is encoded in source 
		sourceformat = getParameter("sourceformat","yyyy-MM-dd");
		//get the format the date should be encoded as output 
		targetformat = getParameter("targetformat",getParameter("format","Y.M.D")).toUpperCase();
	}
}
