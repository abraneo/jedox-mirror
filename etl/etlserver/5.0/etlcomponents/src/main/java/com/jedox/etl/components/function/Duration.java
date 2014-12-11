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
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

import java.text.ParseException;

public class Duration extends Function {

    private static final long MILLISECS_PER_SECOND = 1000;
	private static final long MILLISECS_PER_MINUTE = 60*MILLISECS_PER_SECOND;
    private static final long MILLISECS_PER_HOUR   = 60*MILLISECS_PER_MINUTE;
    private static final long MILLISECS_PER_DAY    = 24*MILLISECS_PER_HOUR;
    private static final long MILLISECS_PER_WEEK   = 7*MILLISECS_PER_DAY;
    
	private static enum Units {
		SECONDS, MINUTES, HOURS, DAYS, WEEKS, MONTHS, YEARS
	}

	private String sourceformat;
	private Units unit;
	
	private Calendar getCalendar(IColumn column) throws FunctionException {
		Calendar cal = new GregorianCalendar();
		try {
			Object value = column.getValue();
			if (value instanceof Date) {
				cal.setTime((Date) value);
			}
			else {
				SimpleDateFormat sdf = new SimpleDateFormat(sourceformat);
				cal.setTime(sdf.parse(column.getValueAsString()));
			}
			return cal;
		}
		catch (ParseException e) {
			throw new FunctionException(column.getName(),column.getValueAsString(),e.getMessage());
		}
	}
	
	private boolean isEarlyDate(Calendar cal) {
		return cal.get(Calendar.YEAR)<1900 || (cal.get(Calendar.YEAR)==1900 && cal.get(Calendar.MONTH)<2);		
	}
	
	private long getTimestamp (Calendar cal) {
		// include time zone offset from UTC for timestamp 
		long time = cal.getTimeInMillis() + cal.getTimeZone().getOffset(cal.getTimeInMillis());
		// Calendar ignores that 1900 has been a leap year. Subtract one day from timestamp for dates before 28.02.1900
		if (isEarlyDate(cal)) {
			time=time-MILLISECS_PER_DAY;
		}		
		return time;
	}
	
	private long calculateDuration (Calendar start, Calendar end) {
		long time = getTimestamp(end) - getTimestamp(start);				
		if (unit.equals(Units.YEARS) || unit.equals(Units.MONTHS)) {
			// Add n months/years to start date and test if after end date, if not increase n+1
			// Difference between 31.01.2012 and 29.02.2012 is 1 month, but 31.03.2012 and 29.04.2012 is 0 month   
			
			long startTime = start.getTimeInMillis();
			int count=0;
			int counttest=0;
			do {
				count=counttest;
				counttest = (time>0) ? count+1 : count-1;
				start.setTimeInMillis(startTime);
				start.add(getTimeField(),counttest);
			} while ((time>=0 && getTimestamp(start)<=getTimestamp(end)) ||
					 (time<0 && getTimestamp(start)>=getTimestamp(end)));
			return count;		
/*			
			long years = end.get(Calendar.YEAR) - start.get(Calendar.YEAR);
			if ((end.get(Calendar.MONTH) < start.get(Calendar.MONTH)) || 
				(end.get(Calendar.MONTH) == start.get(Calendar.MONTH) && (end.get(Calendar.DAY_OF_MONTH) < start.get(Calendar.DAY_OF_MONTH))) ) {
				years -= 1;
			}
			return years;
*/			
		}
		else {			
			switch (unit) {
			case SECONDS: return time/MILLISECS_PER_SECOND;
			case MINUTES: return time/MILLISECS_PER_MINUTE;
			case HOURS: return time/MILLISECS_PER_HOUR;
			case DAYS: return time/MILLISECS_PER_DAY;
			case WEEKS: return time/MILLISECS_PER_WEEK;
			}
			return time;
		}	
	}
	
	private int getTimeField() {
		switch (unit) {
		case SECONDS: return Calendar.SECOND;
		case MINUTES: return Calendar.MINUTE;
		case HOURS: return Calendar.HOUR_OF_DAY;
		case DAYS: return Calendar.DAY_OF_MONTH;
		case WEEKS: return Calendar.WEEK_OF_YEAR;
		case MONTHS: return Calendar.MONTH;
		case YEARS: return Calendar.YEAR;
		}
		return 0;
	}
	
	private String calculateNewDate (Calendar cal, Integer amount) {
		boolean isStartEarly = isEarlyDate(cal);
		cal.add(getTimeField(),amount);
		// Calendar ignores that 1900 has been a leap year. Subtract or one one day from new date if it's skipped the 28.02.1900 barrier
		boolean isEndEarly = isEarlyDate(cal);
		if (isStartEarly && !isEndEarly)
			cal.add(Calendar.DAY_OF_MONTH, -1);
		else if (!isStartEarly && isEndEarly)
			cal.add(Calendar.DAY_OF_MONTH, 1);		
		SimpleDateFormat f = new SimpleDateFormat(sourceformat);
		f.setCalendar(cal);
		return f.format(cal.getTime());
	}	
	
	protected Object transform(Row values) throws FunctionException {
		Calendar s = getCalendar(values.getColumn(0));

		int amount=0;
		// If 2nd input is a numerical value: Add this amount to date.
		// Otherwise the 2nd input is supposed to be a date and the duration between 2 days is returned
		boolean isAdd=true;
		try {
			amount = Integer.parseInt(values.getColumn(1).getValueAsString());
		}
		catch (NumberFormatException e) {
			isAdd=false;			
		}
		
		if (isAdd) {
			// Add the amount to date
			return calculateNewDate(s,amount);
		} else {
			// calculate duration between 2 days
			Calendar e = getCalendar(values.getColumn(1));		
			return calculateDuration(s,e);
		}	
	}
	
	public String getValueType() {
		return "java.lang.String";
	}

	public void validateInputs() throws ConfigurationException {
		checkInputSize(2,false);
	}

	protected void validateParameters() throws ConfigurationException {
		sourceformat = getParameter("sourceformat",getParameter("format", "yyyy-MM-dd"));
		String unitString = getParameter("unit","seconds");
		try { 
			unit = Units.valueOf(unitString.toUpperCase());
		}
		catch (Exception e) {
			throw new ConfigurationException("Parameter unit has to be set to either SECONDS, MINUTES, HOURS, DAYS or YEARS");
		}	
	}


}
