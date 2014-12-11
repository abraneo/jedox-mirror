/**
 *   @brief <Desc ription of Class>
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
 *   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import com.jedox.etl.components.config.extract.CalendarExtractConfigurator;
import com.jedox.etl.components.config.extract.CalendarExtractConfigurator.Options;
import com.jedox.etl.components.config.extract.CalendarExtractConfigurator.LevelDescriptor;
import com.jedox.etl.components.config.extract.CalendarExtractConfigurator.Levels;
import com.jedox.etl.components.config.extract.CalendarExtractConfigurator.LevelAttribute;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.tree.Consolidation;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.util.DateFormatUtil;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class CalendarExtract extends TreeSource implements IExtract {

	private class WeekMapSetting {
		public String mapTo;
		public boolean moveChildren;
		public boolean firstChildren;
	}

	private Options options;
	private List<LevelDescriptor> levels;
	private DateFormatUtil formater = new DateFormatUtil();
	private static final Log log = LogFactory.getLog(CalendarExtract.class);
	private ArrayList<ITreeElement> uniqueLeaves = new ArrayList<ITreeElement>();

	private String dummyRoot = "#_DUMMYROOT_#";
	private String timeAttribute = "#_DUMMYTIME_#";
	private String levelAttribute = "#_DUMMYLEVEL_#";

	private ITreeManager manager;


	private HashMap<String,WeekMapSetting> weekMap = new HashMap<String,WeekMapSetting>();

	public CalendarExtract() {
		setConfigurator(new CalendarExtractConfigurator());
	}

	public CalendarExtractConfigurator getConfigurator() {
		return (CalendarExtractConfigurator)super.getConfigurator();
	}

	private Options getOptions() {
		return options;
	}

	private LevelDescriptor getLevel(Levels name) {
		for (LevelDescriptor l : getLevels())
			if (l.getName().equals(name))
				return l;
		return null;
	}

	private boolean existsLevel(Levels name) {
		return getLevel(name) != null;
	}

	private List<LevelDescriptor> getLevels() {
		return levels;
	}

	private Calendar getNodeCalendar(ITreeElement n) {
		Levels level = getNodeLevel(n.getName());
		GregorianCalendar c;
		LevelDescriptor d = getLevel(level);
		if (d != null)
			c = getCalendar(getLevel(level));
		else
			c = new GregorianCalendar();
		Date date = (Date) manager.getAttributeValue(timeAttribute, n.getName(), false);
		c.setTime(date);
		return c;
	}

	private GregorianCalendar getCalendar(LevelDescriptor level) {
		GregorianCalendar c = getFormater().getCalendar(level.getLocale());
		c.setFirstDayOfWeek(getOptions().getFirstDayInWeek());
		c.setMinimalDaysInFirstWeek(getOptions().getMinDaysInWeek());
		//c.setMinimalDaysInFirstWeek(7);
		return c;
	}

	private Levels getNodeLevel(String parent) {
		return Levels.valueOf(manager.getAttributeValue(levelAttribute, parent, false).toString());    
	}

	private DateFormatUtil getFormater() {
		return formater;
	}

	private String checkName(Calendar calendar, String pattern, LevelDescriptor level) {
		String name = getFormater().format(calendar, pattern);
		ITreeElement n = manager.getElement(name);
		if (n != null) {
			String oldName = name;
			name = getFormater().format(calendar, pattern+" "+level.getDefaultPattern());
			log.warn("Avoiding multiple consolidation of node "+oldName+" by renaming to "+name+". Please verify your generation pattern!");
		}
		return name;
	}

	protected ITreeManager buildTree() throws RuntimeException {
		manager=getTreeManager();
		manager.setAutoCommit(true);
		//creat dummy root element. It is renamed to real root node if configured or removed
		ITreeElement rootElement = manager.provideElement(dummyRoot,ElementType.ELEMENT_NUMERIC);
		uniqueLeaves.add(rootElement);

		manager.addAttributes(new String[]{timeAttribute,levelAttribute},new ElementType[]{ElementType.ELEMENT_NUMERIC,ElementType.ELEMENT_STRING});

		//add current start of year to root;
		GregorianCalendar c = new GregorianCalendar();
		int year = c.get(Calendar.YEAR);
		c.clear();
		c.set(Calendar.YEAR, year);

		//add time and level attribute to root.
		manager.addAttributeValue(timeAttribute, rootElement.getName(), c.getTime());
		manager.addAttributeValue(levelAttribute, rootElement.getName(), Levels.years.toString());

		//generate Levels
		generateYears(getLevel(Levels.years));
		generateQuarters(getLevel(Levels.quarters));
		generateMonths(getLevel(Levels.months));
		generateWeeks(getLevel(Levels.weeks));
		generateDays(getLevel(Levels.days));
		generateHours(getLevel(Levels.hours));
		generateMinutes(getLevel(Levels.minutes));
		generateSeconds(getLevel(Levels.seconds));


		if (getOptions().getRoot() != null)
			manager.renameElement(dummyRoot,getOptions().getRoot());
		else
			manager.removeElements(new ITreeElement[]{rootElement});

		mapWeekNodes();
		
		//generate timetodate for mode toNext
		generateTTD_ToNext(rootElement);

		manager.removeAttributes(new IAttribute[]{manager.getAttributeByName(levelAttribute),manager.getAttributeByName(timeAttribute)});
		
		manager.setAutoCommit(false);
		return manager;
	}

	private ITreeElement addNode(Calendar calendar, LevelDescriptor level, ITreeElement parent) {
		String name = checkName(calendar, level.getPattern(), level);
		ITreeElement node = manager.provideElement(name,ElementType.ELEMENT_NUMERIC);		
		manager.addConsolidation(parent,node,1);
		for (LevelAttribute a : level.getAttributes()) {
			String avalue = getFormater().format(calendar, a.getPattern(),a.getLocale());
			if (manager.getAttributeByName(a.getName())==null) {
				manager.addAttributes(new String[]{a.getName()},new ElementType[]{ElementType.ELEMENT_STRING});				
			}
			manager.addAttributeValue(a.getName(), node.getName(), avalue);			
		}
		manager.addAttributeValue(levelAttribute, node.getName(), level.getName().toString());
		return node;
	}

	private boolean getCondition(ITreeElement parent, Calendar calendar) {
		Calendar parentCalendar = getNodeCalendar(parent);
		Levels parentLevel = getNodeLevel(parent.getName());
		switch (parentLevel) {
		case years: return parentCalendar.get(Calendar.YEAR) == calendar.get(Calendar.YEAR);
		case quarters: return parentCalendar.get(Calendar.YEAR) == calendar.get(Calendar.YEAR) && parentCalendar.get(Calendar.MONTH)+3 > calendar.get(Calendar.MONTH);
		case months: return parentCalendar.get(Calendar.MONTH) == calendar.get(Calendar.MONTH);
		case weeks: return parentCalendar.get(Calendar.WEEK_OF_YEAR) == calendar.get(Calendar.WEEK_OF_YEAR) && parentCalendar.get(Calendar.YEAR) == calendar.get(Calendar.YEAR) && ((!existsLevel(Levels.months)&&!existsLevel(Levels.quarters)) || parentCalendar.get(Calendar.MONTH) == calendar.get(Calendar.MONTH));
		case days: return parentCalendar.get(Calendar.DAY_OF_YEAR) == calendar.get(Calendar.DAY_OF_YEAR);
		case hours: return parentCalendar.get(Calendar.HOUR_OF_DAY) == calendar.get(Calendar.HOUR_OF_DAY);
		case minutes: return parentCalendar.get(Calendar.MINUTE) == calendar.get(Calendar.MINUTE);
		default : return false;
		}
	}

	private boolean checkTTDPattern(LevelDescriptor l){
		if (l != null && l.isTimeToDate() && getOptions().getTTDmode().equals("toRoot")) {
			if (l.getTtdPattern() == null || l.getPattern().equals(l.getTtdPattern())) {
				l.setTtdPattern(l.getPattern()+" 'TTD'");
				log.warn("Timetodate pattern is identical to pattern on level "+l.getName()+". Resetting to "+l.getTtdPattern());
			}
			return true;
		}
		return false;
	}

	private ITreeElement getElementAgg(ITreeElement newChildNode, ITreeElement lastNode, LevelDescriptor level) {

		String name = checkName(getNodeCalendar(newChildNode),level.getTtdPattern(),level);
		//create a new aggregate node
		ITreeElement agg = manager.provideElement(name,ElementType.ELEMENT_NUMERIC);	
		if(lastNode!=null){
			manager.removeConsolidation(agg);
			manager.addConsolidation(agg, lastNode, 1);
		}
		manager.addConsolidation(agg, newChildNode, 1);
		return agg;
		
	}

	private void generateYears(LevelDescriptor years) {
		if (years != null) {
			Calendar c = getCalendar(years);
			c.clear();
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				for (int i=years.getStart(); i<=years.getEnd(); i++) {
					c.set(Calendar.YEAR, i);
					e = addNode(c,years,parent);
					manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
					newLeaves.add(e);
					if(checkTTDPattern(years)){
						lastNode = getElementAgg(e, lastNode, years);
					}
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateQuarters(LevelDescriptor quarters) {
		if (quarters != null) {
			Calendar c = getCalendar(quarters);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());
				for (int i=1; getCondition(parent,c); i++) {
					if (i >= quarters.getStart() && i <= quarters.getEnd()){
						e = addNode(c,quarters,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(quarters)){
							lastNode = getElementAgg(e, lastNode, quarters);
						}
					}
					c.add(Calendar.MONTH, 3);
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateMonths(LevelDescriptor months) {
		if (months != null) {
			Calendar c = getCalendar(months);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());
				for (int i=1; getCondition(parent,c); i++) {
					if (i >= months.getStart() && i <= months.getEnd()){
						e = addNode(c,months,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(months)){
							lastNode = getElementAgg(e, lastNode, months);
						}
					}
					c.add(Calendar.MONTH, 1);

				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateWeeks(LevelDescriptor weeks) {
		if (weeks != null) {			
			Calendar c = getCalendar(weeks);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();

			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());

				// Check if first week in year has to be mapped
				if (getOptions().getCompleteWeek()) {
					String name=getFormater().format(c, weeks.getPattern());
					String newName=getFormater().format(c,weeks.getPattern(),true);
					if (!name.equals(newName)) {
						WeekMapSetting setting=new WeekMapSetting();
						setting.mapTo=newName;
						setting.firstChildren=false;
						setting.moveChildren=!parent.equals(uniqueLeaves.get(0));		
						weekMap.put(name, setting);
					}	
				}

				for (int i=1; getCondition(parent,c); i++) {
					if (i >= weeks.getStart() && i <= weeks.getEnd()){
						e = addNode(c,weeks,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(weeks)){
							lastNode = getElementAgg(e, lastNode, weeks);
						}
					}
					c.add(Calendar.WEEK_OF_YEAR, 1);
					c.set(Calendar.DAY_OF_WEEK,c.getFirstDayOfWeek());
				}

				// Check if last week in year has to be mapped
				if (getOptions().getCompleteWeek()) {
					c.add(Calendar.YEAR,-1);
					c.set(Calendar.DAY_OF_YEAR,c.getActualMaximum(Calendar.DAY_OF_YEAR));
					// Test with last day of the year
					String name=getFormater().format(c, weeks.getPattern());
					String newName=getFormater().format(c,weeks.getPattern(),true);
					if (!name.equals(newName)) {
						WeekMapSetting setting=new WeekMapSetting();
						setting.mapTo=newName;
						setting.firstChildren=true;
						setting.moveChildren=!parent.equals(uniqueLeaves.get(uniqueLeaves.size()-1));		
						weekMap.put(name, setting);
					}	
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateDays(LevelDescriptor days) {
		if (days != null) {
			Calendar c = getCalendar(days);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());
				for (int i=1; getCondition(parent,c); i++) {
					if (i >= days.getStart() && i <= days.getEnd()){
						e = addNode(c,days,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(days)){
							lastNode = getElementAgg(e, lastNode, days);
						}
					}
					c.add(Calendar.DAY_OF_YEAR, 1);
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateHours(LevelDescriptor hours) {
		if (hours != null) {
			Calendar c = getCalendar(hours);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());
				for (int i=1; getCondition(parent,c); i++) {
					if (i >= hours.getStart() && i <= hours.getEnd()){
						e = addNode(c,hours,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(hours)){
							lastNode = getElementAgg(e, lastNode, hours);
						}
					}
					c.add(Calendar.HOUR_OF_DAY, 1);
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateMinutes(LevelDescriptor minutes) {
		if (minutes != null) {
			Calendar c = getCalendar(minutes);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());
				for (int i=1; getCondition(parent,c); i++) {
					if (i >= minutes.getStart() && i <= minutes.getEnd()){
						e = addNode(c,minutes,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(minutes)){
							lastNode = getElementAgg(e, lastNode, minutes);
						}
					}
					c.add(Calendar.MINUTE, 1);
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	private void generateSeconds(LevelDescriptor seconds) {
		if (seconds != null) {
			Calendar c = getCalendar(seconds);
			ITreeElement lastNode = null;
			ITreeElement e = null;
			ArrayList<ITreeElement> newLeaves = new ArrayList<ITreeElement>();
			for (ITreeElement parent : uniqueLeaves) {
				c.setTime(getNodeCalendar(parent).getTime());
				for (int i=1; getCondition(parent,c); i++) {
					if (i >= seconds.getStart() && i <= seconds.getEnd()){
						e = addNode(c,seconds,parent);
						manager.addAttributeValue(timeAttribute, e.getName(), c.getTime());
						newLeaves.add(e);
						if(checkTTDPattern(seconds)){
							lastNode = getElementAgg(e, lastNode, seconds);
						}
					}
					c.add(Calendar.SECOND, 1);
				}
			}
			uniqueLeaves = newLeaves;
		}
	}

	public void mapWeekNodes() {
		if (!getOptions().getCompleteWeek())
			return;

		for (String mapFrom : weekMap.keySet()) {
			WeekMapSetting setting=weekMap.get(mapFrom);
			ITreeElement mapFromNode = manager.getElement(mapFrom);
			if (mapFromNode!=null) {
				if (setting.moveChildren) {
					String dummy="#Dummy";
					ITreeElement mapToNode = manager.provideElement(setting.mapTo,ElementType.ELEMENT_NUMERIC);		
					ITreeElement dummyNode = manager.provideElement(dummy,ElementType.ELEMENT_NUMERIC);		
					if (setting.firstChildren) {
						for (ITreeElement tn : manager.getDescendants(mapToNode, false)) {
							manager.addConsolidation(dummyNode, tn, 1);
						}
						manager.removeConsolidation(mapToNode);
					}	

					if (mapFromNode!=null) {
						for (ITreeElement tn : manager.getDescendants(mapFromNode, false)) {
							manager.addConsolidation(mapToNode, tn, 1);
						}
					}	
					if (setting.firstChildren) {
						for (ITreeElement tn : manager.getDescendants(dummyNode, false)) {
							manager.addConsolidation(mapToNode, tn, 1);
						}
					}
					manager.removeElements(new ITreeElement[]{dummyNode, mapFromNode});
				}
				else {
					manager.renameElement(mapFrom, setting.mapTo);
				}
			}
		}		
	}

	private ITreeElement aggregate_ToNext(ITreeElement parent, LevelDescriptor level) {
		
		ITreeElement[] children = parent.getChildren();
		int start=1;
		if (children.length > 0) {
			//remove data node from manager
			manager.removeConsolidation(parent);			
			for (int i = children.length-(start); i >=0; i--) {
				ITreeElement c = children[i];
				String name = checkName(getNodeCalendar(c),level.getTtdPattern(),level);
				//create a new aggregate node
				ITreeElement aggNode = manager.provideElement(name,ElementType.ELEMENT_NUMERIC);		
				manager.addConsolidation(parent, aggNode ,1);
				//add data node to newly created aggregate node.
				manager.addSubtree(c, new IConsolidation[]{new Consolidation(aggNode, c, 1)});
				//aggregate node is now new parent to add nodes.
				parent = aggNode;
			}
			ITreeElement c = children[0];
			// manager.removeConsolidation(c);
			manager.addSubtree(c, new IConsolidation[]{new Consolidation(parent, c, 1)});
			parent = c;
		}		
		return parent;
	}
	
	
	public void generateTTD_ToNext(ITreeElement rootElement) {
		if (!getOptions().getTTDmode().equals("toNext"))
			return;
		
		for (LevelDescriptor l : getLevels()) {
			if (l.isTimeToDate()) {
				if (l.getTtdPattern() == null || l.getPattern().equals(l.getTtdPattern())) {
					l.setTtdPattern(l.getPattern()+" 'TTD'");
					log.warn("Timetodate pattern is identical to pattern on level "+l.getName()+". Resetting to "+l.getTtdPattern());
				}
				int pos = getLevels().indexOf(l)-1;
				if (pos >= 0) { //there is a parent level
					LevelDescriptor parentLevel = getLevels().get(pos);
					List<ITreeElement> parents = manager.getElementsByAttribute(levelAttribute,parentLevel.getName().toString());
					for (ITreeElement parent : parents) {
						if (!parent.equals(rootElement))
							aggregate_ToNext(parent, l);
					}
				}
				else //use root level
					aggregate_ToNext(rootElement, l);
			}
		}		
	}

	public void init() throws InitializationException {
		try {
			super.init();
			options = getConfigurator().getOptions();
			levels = getConfigurator().getLevels();
			if (getOptions().getCompleteWeek() && existsLevel(Levels.weeks) && (existsLevel(Levels.months)||existsLevel(Levels.quarters)))
				throw new InitializationException("Level Weeks is not possible in combination with Level Months or Quarters if option CompleteWeeks is set.");
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
