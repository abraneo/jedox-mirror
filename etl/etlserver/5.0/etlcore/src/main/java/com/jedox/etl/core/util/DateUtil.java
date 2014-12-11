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
package com.jedox.etl.core.util;

import java.util.GregorianCalendar;
import java.util.Calendar;
import java.util.Date;
import java.text.DateFormat;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class DateUtil {
	private GregorianCalendar calendar;
	private static final DateUtil instance = new DateUtil();
	private static final Log log = LogFactory.getLog(DateUtil.class);
	
	DateUtil() {
		calendar = new GregorianCalendar();
	}

	public static DateUtil getInstance() {
		return instance;
	}
	
	private int getQuarter(int month) {
		switch (month) {
			case 1: return 1;
			case 2: return 1;
			case 3: return 1;
			case 4: return 2;
			case 5: return 2;
			case 6: return 2;
			case 7: return 3;
			case 8: return 3;
			case 9: return 3;
			case 10: return 4;
			case 11: return 4;
			case 12: return 4;
		}
		return 0;
	}
	
	public String getActual(String format) {
		return parseDate(new Date(),format);
	}
	
	public String getPrevious(String target, String format) {
		calendar.setTime(new Date());
		boolean years = target.contains("Y");
		boolean quarters = target.contains("Q");
		boolean months = target.contains("M");
		boolean weeks = target.contains("W");
		boolean days = target.contains("D");
		if (years) 
			calendar.add(Calendar.YEAR,-1);
		if (quarters) 
			calendar.add(Calendar.MONTH,-3);
		if (months) 
			calendar.add(Calendar.MONTH,-1);
		if (weeks)
			calendar.add(Calendar.WEEK_OF_YEAR,-1);
		if (days)
			calendar.add(Calendar.DAY_OF_YEAR,-1);
		return parseDate(calendar.getTime(),format);
	}
	
	
	public String parseDate(Date date, String format) {
		StringBuffer result = new StringBuffer();
		calendar.setTime(date);
		int weekFormat = Calendar.WEEK_OF_YEAR;
		int dayFormat = Calendar.DAY_OF_MONTH;
		for (int i=0; i<format.length(); i++) {
			char c = format.toUpperCase().charAt(i);
			switch (c) {
				case 'Y' : result.append(calendar.get(Calendar.YEAR)); break;
				case 'Q' : result.append(getQuarter(calendar.get(Calendar.MONTH)+1)); break;
				case 'M' : result.append(calendar.get(Calendar.MONTH)+1); weekFormat = Calendar.WEEK_OF_MONTH; break;
				case 'W' : result.append(calendar.get(weekFormat)); dayFormat = Calendar.DAY_OF_WEEK; break;
				case 'D' : result.append(calendar.get(dayFormat)); break;
				case 'H' : result.append(calendar.get(Calendar.HOUR_OF_DAY)); break;
				case 'N' : result.append(calendar.get(Calendar.MINUTE)); break;
				case 'S' : result.append(calendar.get(Calendar.SECOND)); break;
				default: result.append(c); 
			}
		}
		//result.deleteCharAt(result.length()-1);
		return result.toString();
	}
	
	public String parseDate(String datestring, String format) {
		try {
			DateFormat df = DateFormat.getDateInstance();
			Date date = df.parse(datestring);
			return parseDate(date,format);
		} catch (Exception e) {
			log.error("Failed to parse date: "+e.getMessage());
		}
		return null;
	}
}
