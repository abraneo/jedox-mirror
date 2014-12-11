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
package com.jedox.etl.components.config.extract;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.util.DateFormatUtil;
import java.util.Locale;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;
import java.util.Calendar;
import java.util.List;
import java.util.LinkedList;
import java.util.GregorianCalendar;

public class CalendarExtractConfigurator extends TreeSourceConfigurator {

	public static enum Levels {
		years, halfyears, quarters, months, weeks, days, hours, minutes, seconds
	}
	
	public class Options {
		private String root;
		private Locale locale = Locale.getDefault();
		private Integer firstDayInWeek;
		private Integer minDaysInWeek;
		private String TTDmode = "toRoot";
		private String standard = "Standard";
		private boolean completeWeek=false;

		public String getRoot() {
			return root;
		}
		public void setRoot(String root) {
			this.root = root;
		}
		public String getTTDmode() {
			return TTDmode;
		}
		public void setTTDmode(String ttdMode) {
			if (ttdMode != null)			
				this.TTDmode = ttdMode;
		}
		public Locale getLocale() {
			return locale;
		}
		public void setLocale(Locale locale) {
			this.locale = locale;
		}
		public Integer getFirstDayInWeek() {
			return firstDayInWeek;
		}
		public void setFirstDayInWeek(Integer firstDayInWeek) {
			this.firstDayInWeek = firstDayInWeek;
		}
		
		public void setFirstDayInWeek(String day) throws ConfigurationException {
			if (day != null && !day.equalsIgnoreCase(standard)) { 
				this.firstDayInWeek=new DateFormatUtil().getWeekday(day);
			}	
		}
		
		public Integer getMinDaysInWeek() {
			return minDaysInWeek;
		}
		public void setMinDaysInWeek(String days) {
			if (days != null  && !days.equalsIgnoreCase(standard)) { 
				this.minDaysInWeek = Integer.parseInt(days);
			}
		}	
		public void setMinDaysInWeek(Integer minDaysInWeek) {
			this.minDaysInWeek = minDaysInWeek;
		}		
		public boolean getCompleteWeek() {
			return completeWeek;
		}
		public void setCompleteWeek(String completeWeekString) {
			if (completeWeekString != null)
				this.completeWeek = Boolean.parseBoolean(completeWeekString);
		}
		
	}

	public class LevelAttribute {
		private Locale locale;
		private String pattern;
		private String name;
		private AttributeModes type;

		public String getName() {
			return name;
		}
		public void setName(String name) {
			this.name = name;
		}
		public Locale getLocale() {
			return locale;
		}
		public void setLocale(Locale locale) {
			this.locale = locale;
		}
		public String getPattern() {
			return pattern;
		}
		public void setPattern(String pattern) {
			this.pattern = pattern;
		}
		public AttributeModes getType() {
			return type;
		}
		public void setType(AttributeModes type) {
			this.type = type;
		}
	}

	public class LevelDescriptor {
		private Levels name;
		private int start = 0;
		private int end = Integer.MAX_VALUE;
		private Locale locale;
		private String pattern;
		private String defaultPattern;
		private boolean timeToDate = false;
		private String ttdPattern;
		private List<LevelAttribute> attributes;

		public Levels getName() {
			return name;
		}
		public void setName(Levels name) {
			this.name = name;
		}
		public int getStart() {
			return start;
		}
		public void setStart(int start) throws ConfigurationException {
			this.start = start;
		}
		public int getEnd() {
			return end;
		}
		public void setEnd(int end) throws ConfigurationException {
			if (start>end)
				throw new ConfigurationException("End value "+end+" is lower than start value "+start+" for level "+name);
			this.end = end;
		}
		public Locale getLocale() {
			return locale;
		}
		public void setLocale(Locale locale) {
			this.locale = locale;
		}
		public String getPattern() {
			return (pattern != null) ? pattern : getDefaultPattern();
		}
		public void setPattern(String pattern) {
			this.pattern = pattern;
		}
		public String getDefaultPattern() {
			return defaultPattern;
		}
		public void setDefaultPattern(String pattern) {
			this.defaultPattern = pattern;
		}
		public boolean isTimeToDate() {
			return timeToDate;
		}
		public void setTimeToDate(boolean timeToDate) {
			this.timeToDate = timeToDate;
		}
		public String getTtdPattern() {
			return ttdPattern;
		}
		public void setTtdPattern(String ttdPattern) {
			this.ttdPattern = ttdPattern;
		}
		public List<LevelAttribute> getAttributes() {
			return attributes;
		}
		public void setAttributes(List<LevelAttribute> attributes) {
			this.attributes = attributes;
		}
	}

	private static final Log log = LogFactory.getLog(CalendarExtractConfigurator.class);

	public Options getOptions() throws ConfigurationException {
		Element o = getXML().getChild("options");
		Options options = new Options();
		if (o != null) {
			options.setRoot(o.getChildTextTrim("root"));
			String language = o.getChildTextTrim("language");
			if (language != null)
			options.setLocale(new Locale(language));			
			options.setCompleteWeek(o.getChildTextTrim("completeWeek"));
			options.setFirstDayInWeek(o.getChildTextTrim("firstDayOfWeek"));
			options.setMinDaysInWeek(o.getChildTextTrim("minDaysInWeek"));			
			options.setTTDmode(o.getChildTextTrim("TTDmode"));			
		}
		// set default values of for first/min day in weeks parameters if not configured 
		GregorianCalendar c = new GregorianCalendar(options.getLocale());
		if (options.getFirstDayInWeek() == null)
			options.setFirstDayInWeek(c.getFirstDayOfWeek());
		if (options.getMinDaysInWeek() == null)
			options.setMinDaysInWeek(c.getMinimalDaysInFirstWeek());
		return options;
	}

	private List<LevelAttribute> getAttributes(Element level, Locale locale) {
		LinkedList<LevelAttribute> result = new LinkedList<LevelAttribute>();
		LinkedList<Element> attributes = new LinkedList<Element>();
		attributes.addAll(getChildren(level,"attribute","attributes"));
		attributes.addAll(getChildren(level,"alias","attributes"));
		for (Element a : attributes) {
			LevelAttribute attribute = new LevelAttribute();
			attribute.setName(a.getAttributeValue("name"));
			String language = a.getChildTextTrim("language");
			attribute.setPattern(a.getChildTextTrim("pattern"));
			attribute.setType(AttributeModes.valueOf(a.getName().toUpperCase()));
			if (language != null)
				attribute.setLocale(new Locale(language));
			else
				attribute.setLocale(locale);
			result.add(attribute);
		}
		return result;
	}

	private String getPatternSeparator(Levels parent, Levels level) {
		if (parent == null)
			return "";
		switch (level) {
		case years: return "";
		case halfyears: return ".";
		case quarters: return ".";
		case weeks: return ".";
		case months: return ".";
		case days: return ".";
		case hours: return " ";
		case minutes: return "";
		case seconds: return ":";
		default: return "";
		}
	}

	private String getPattern(Levels parent, Levels level) {
		switch (level) {
		case years: return "yyyy";
		case halfyears: return "q";
		case quarters: return "Q";
		case months: return "MM";
		case weeks: {
			if (Levels.months.equals(parent))
				return "W";
			else
				return "ww";
		}
		case days: {
			if (Levels.weeks.equals(parent))
				return "E";
			else if (Levels.months.equals(parent))
				return "dd";
			else
				return "DD";
		}
		case hours: return "HH";
		case minutes: return "mm";
		case seconds: return "ss";
		default: return "";
		}
	}

	private int getMaximumEnd(Levels parent, Levels level) {
		switch (level) {
		case years: return Integer.MAX_VALUE;
		case halfyears: return 2;
		case quarters: return 4;
		case months: return 12;
		case weeks: {
			if (Levels.months.equals(parent))
				return 5;
			else
				return 54;
		}
		case days: {
			if (Levels.weeks.equals(parent))
				return 7;
			else if (Levels.months.equals(parent))
				return 31;
			else
				return 366;
		}
		case hours: return 24;
		case minutes: return 60;
		case seconds: return 60;
		default: return 0;
		}
	}

	public List<LevelDescriptor> getLevels() throws ConfigurationException {
		LinkedList<LevelDescriptor> result = new LinkedList<LevelDescriptor>();
		Options options = getOptions();
		for (Levels name : Levels.values()) {
			Element l = getXML().getChild("levels").getChild(name.toString());
			if (l != null) {
				LevelDescriptor level = new LevelDescriptor();
				level.setName(name);
				level.setPattern(l.getChildTextTrim("pattern"));
				if (name.equals(Levels.years)) { //override default start and stop year with current year
					GregorianCalendar c = new GregorianCalendar();
					int year = c.get(Calendar.YEAR);
					level.setStart(year);
					level.setEnd(year);
				}
				String start = l.getChildTextTrim("start");
				String end = l.getChildTextTrim("end");
				String language = l.getChildTextTrim("language");
				if (language != null)
					level.setLocale(new Locale(language));
				else
					level.setLocale(options.getLocale());
				if (start != null)
					level.setStart(Integer.parseInt(start));
				if (end != null)
					level.setEnd(Integer.parseInt(end));
				level.setAttributes(getAttributes(l,level.getLocale()));
				Element timetodate = l.getChild("timetodate");
				if (timetodate != null) {
					level.setTimeToDate(true);
					level.setTtdPattern(timetodate.getChildTextTrim("pattern"));
				}
				result.add(level);
			}
		}
		//apply default patterns
		Levels parentLevel = null;
		StringBuffer defaultPatternsStr = new StringBuffer();
		String defaultPatterns = "";
		for (int i=0; i<result.size(); i++) {
			LevelDescriptor d = result.get(i);
			if (defaultPatterns.length()==0) 
				d.setDefaultPattern(getPattern(parentLevel,d.getName()));
			else  
				d.setDefaultPattern(defaultPatterns + getPatternSeparator(parentLevel,d.getName()) + getPattern(parentLevel,d.getName()));
			defaultPatterns= d.getDefaultPattern();
			//d.setDefaultPattern(parentPattern+getPatternSeparator(parentLevel,d.getName())+getPattern(parentLevel,d.getName()));
			defaultPatternsStr.append(" '"+d.getDefaultPattern()+"' for "+d.getName().toString()+",");
			parentLevel = d.getName();
			int maxEnd = getMaximumEnd(parentLevel,d.getName());
			if (d.getStart() > maxEnd)
				log.warn("Extract "+getName()+": Parameter 'start' exeeds allowed maximum of "+maxEnd);
			if (d.getEnd() > maxEnd && d.getEnd() != Integer.MAX_VALUE)
				log.warn("Extract "+getName()+": Parameter 'end' exeeds allowed maximum of "+maxEnd);
		}
		if (defaultPatternsStr.length() > 0)
			log.debug("Extract "+getName()+": Using default pattern"+defaultPatternsStr.substring(0, defaultPatternsStr.length()-1));
		return result;
	}

	public void configure() throws ConfigurationException {
		super.configure();
	}

}
