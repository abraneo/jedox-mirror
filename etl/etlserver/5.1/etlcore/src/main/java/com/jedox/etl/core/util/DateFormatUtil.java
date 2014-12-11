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

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Locale;

import com.jedox.etl.core.component.ConfigurationException;

public class DateFormatUtil {

	protected class MyCalendar extends GregorianCalendar {

		/**
		 *
		 */
		private static final long serialVersionUID = 6093354951161486316L;
		private Locale locale;

		public MyCalendar(Locale locale) {
			super(locale);
			this.locale = locale;
		}

		public Locale getLocale() {
			return locale;
		}

		public int get(int field) {
			if (field == Calendar.WEEK_OF_YEAR) {
				if (super.get(Calendar.MONTH) == Calendar.DECEMBER && super.get(Calendar.WEEK_OF_YEAR) <= 2)
					return 52+super.get(Calendar.WEEK_OF_YEAR);
				if (super.get(Calendar.MONTH) == Calendar.JANUARY && super.get(Calendar.WEEK_OF_YEAR) >= 52)
					return 0;
			}
			return super.get(field);
		}
	}

	public DateFormatUtil() {
	}
	
	private static enum Weekdays {
		sunday, monday, tuesday, wednesday, thursday, friday, saturday
	}	

	private boolean isQuoted(int pos, String format) {
		int before = 0;
		int after = 0;
		for (int i=0; i<pos; i++)
			if (format.charAt(i)=='\'')
				before++;
		for (int i=pos; i<format.length(); i++)
			if (format.charAt(i)=='\'')
				after++;
		if ((before % 2 == 1) && (after % 2 == 1))
			return true;
		else
			return false;
	}

	private int getQuarter(Calendar calendar) {
		return calendar.get(Calendar.MONTH) / 3 + 1;
	}

	private String matchQuarter(Calendar calendar, String format) {
		StringBuffer result = new StringBuffer();
		int start = 0;
		int k = format.indexOf('Q', start);
		while (k >= 0) {
			if (!isQuoted(k,format)) {
				result.append(format.substring(start, k));
				if (result.length() == 0 || result.charAt(result.length()-1) != '\'')
					result.append("'");
				else
					result.deleteCharAt(result.length()-1);
				result.append(getQuarter(calendar)+"'");
			}
			else {
				result.append(format.substring(start, k+1));
			}
			start = k+1;
			k = format.indexOf('Q', start);
		}
		result.append(format.substring(start, format.length()));
		//remove all quotes in direct neighbor sections and join them
		return result.toString().replace("''","");
	}

	private int getHalfYear(Calendar calendar) {
		return calendar.get(Calendar.MONTH) / 6 + 1;
	}

	private String matchHalfYear(Calendar calendar, String format) {
		StringBuffer result = new StringBuffer();
		int start = 0;
		int k = format.indexOf('q', start);
		while (k >= 0) {
			if (!isQuoted(k,format)) {
				result.append(format.substring(start, k));
				if (result.length() == 0 || result.charAt(result.length()-1) != '\'')
					result.append("'");
				else
					result.deleteCharAt(result.length()-1);
				result.append(getHalfYear(calendar)+"'");
			}
			else {
				result.append(format.substring(start, k+1));
			}
			start = k+1;
			k = format.indexOf('q', start);
		}
		result.append(format.substring(start, format.length()));
		//remove all quotes in direct neighbor sections and join them
		return result.toString().replace("''","");
	}
	
	
	
	/*changed by kais,bug: 0005430*/
	public String format(Calendar calendar, String format) {
		return formater(calendar,format, ((MyCalendar)calendar).getLocale());
	}

	/*changed by kais,  bug: 0005430*/
    public String format(Calendar calendar, String format, Locale AttributeLocale) {
		return formater(calendar,format, AttributeLocale);
	}
    
    public String format(Calendar calendar, String format,boolean completeWeek)  {
    	if (!completeWeek) {
    		return format(calendar,format);
    	}
    	// remove all text between ' '
    	String formatCleaned = format;
    	while (formatCleaned.contains("'")) {
    		int pos1 = formatCleaned.indexOf('\'', 0);
    		int pos2 = formatCleaned.indexOf('\'', pos1+1);
    		if (pos1!=-1 && pos2==-1)
    			throw new RuntimeException("Invalid date format "+format+". Missing closing character ' ");
    		formatCleaned=formatCleaned.substring(0, pos1)+formatCleaned.substring(pos2+1);
    	}
    	// format without weeks
    	if (!formatCleaned.contains("w"))  {
    		return format(calendar,format);
    	}
    	if (formatCleaned.contains("d") || formatCleaned.contains("D") || formatCleaned.contains("E"))  {
    		throw new RuntimeException("Invalid date format \""+format+"\": Symbols d, D and E are not allowed in combination with symbol w if option completeWeek is used.");
    	}    	
    	String result=null;    	
    	long savedDate = calendar.getTimeInMillis();
    	// Check for days at the beginning of the year: Is it in first week?
    	if (format(calendar, "w").equals("0")) {
    		// If week is 0 then return the same week as the last day in the previous year
    		calendar.set(Calendar.DAY_OF_YEAR, 1);
			calendar.add(Calendar.DAY_OF_YEAR, -1);
			result = format(calendar, format);
    	}
    	else {
        	// Check for days at the end of each month: Is it in last week?
    		calendar.set(Calendar.DAY_OF_WEEK,calendar.getFirstDayOfWeek());
    		int dayInMonth = Integer.parseInt(format(calendar, "d"));
    		int dayNumberInCurrentMonth = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);
    		if (1+dayNumberInCurrentMonth-dayInMonth < 7) {
    			// set it to the first day in the next month
    			calendar.set(Calendar.DAY_OF_MONTH, calendar.getActualMaximum(Calendar.DAY_OF_MONTH));	    		
    			calendar.add(Calendar.DAY_OF_YEAR,1);
        		// If week is 1 then month is last month of year, use week 1 also for days at the end of the last year
    			if (format(calendar,"w").equals("1")) {
    				result=format(calendar, format);
    			}	    	
    		}    		
    	}	
		calendar.setTimeInMillis(savedDate);
		return result==null ? format(calendar,format) : result;
	}

    
    /*changed by kais, bug: 0005430*/
	private String formater(Calendar calendar, String format, Locale neededlocale) {

		format = matchHalfYear(calendar,format);
		format = matchQuarter(calendar,format);
		Locale locale = (calendar instanceof MyCalendar) ? neededlocale : Locale.getDefault();
		SimpleDateFormat f = new SimpleDateFormat(format,locale);
		f.setCalendar(calendar);
		return f.format(calendar.getTime());
	}

	public GregorianCalendar getCalendar(Locale locale) {
		return new MyCalendar(locale);
	}
	
	public int getWeekday(String day) throws ConfigurationException {
		try {
			Weekdays wd = Weekdays.valueOf(day);
			switch (wd) {
				case sunday : return Calendar.SUNDAY;
				case monday : return Calendar.MONDAY;
				case tuesday : return Calendar.TUESDAY;
				case wednesday : return Calendar.WEDNESDAY;
				case thursday : return Calendar.THURSDAY;
				case friday : return Calendar.FRIDAY;
				case saturday : return Calendar.SATURDAY;
				default: return -1;
				}
			}
			catch (IllegalArgumentException e) {
				// Numerical values for day no longer supported, are converted in ConfigConversion
				throw new ConfigurationException("Illegal day of week "+day);
		}
	}
	
	public String getWeekday(int dayIndex) throws ConfigurationException {
		try {
			return Weekdays.values()[dayIndex-1].toString();
		}
		catch (Exception e) {
			// Numerical values for day no longer supported, are converted in ConfigConversion
			throw new ConfigurationException("Illegal day of week "+dayIndex);
		}
	}	
	
}
