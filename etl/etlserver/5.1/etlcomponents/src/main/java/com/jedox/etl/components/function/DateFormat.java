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
package com.jedox.etl.components.function;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Calendar;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.DateFormatUtil;

import java.util.Locale;

public class DateFormat extends Function {

	private String sourceformat;
	private String targetformat;
	private int minimalDaysInWeek;
	private int firstDayOfWeek;
	private Locale locale = Locale.getDefault();
	private DateFormatUtil formater = new DateFormatUtil();
	private Calendar calendar;
	private boolean completeWeek;
	
	/**
	 * Gets the resolved value according to the template and the actual input.
	 * @return the resolved value 
	 */	
	public Object transform(Row input) throws FunctionException {
		try {
			Object date = input.getColumn(0).getValue();
			if (date != null) {
				Date d;
				if (!(date instanceof Date)) {
					//assume a String or numeric format
					SimpleDateFormat in = new SimpleDateFormat(sourceformat,locale);
					d = in.parse(date.toString());
				}
				else
					d = (Date)date;
				calendar.setTime(d);
				return formater.format(calendar, targetformat, completeWeek);
				//SimpleDateFormat out = new SimpleDateFormat(targetformat,locale);
				//return out.format(date);
			}
			return date;
		} catch (ParseException e) {
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
		targetformat = getParameter("targetformat","yyyy-MM-dd");
		String language = getParameter("language",null);
		if (language != null) {
			locale = new Locale(language);
		}
		calendar = formater.getCalendar(locale);
		
		String firstDayString = getParameter("firstDayOfWeek","");
		
		// Set default with value "Standard" as in Calendar Extract
		if (firstDayString.isEmpty() || firstDayString.equalsIgnoreCase("Standard"))
			firstDayOfWeek=calendar.getFirstDayOfWeek();
		else {
			// Integer values only allowed for 5.0 backwards compatibility
			try {
				firstDayOfWeek=Integer.parseInt(firstDayString);
			} catch (NumberFormatException e) {
				firstDayOfWeek=new DateFormatUtil().getWeekday(firstDayString);
			}
		}
		minimalDaysInWeek = Integer.parseInt(getParameter("minimalDaysInWeek",String.valueOf(calendar.getMinimalDaysInFirstWeek())));
		calendar.setFirstDayOfWeek(firstDayOfWeek);
		calendar.setMinimalDaysInFirstWeek(minimalDaysInWeek);
		completeWeek=Boolean.parseBoolean(getParameter("completeWeek","false"));
	}

}
