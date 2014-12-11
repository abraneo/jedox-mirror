/**
*   @brief <Description of Class>
*
*   @file
*
*   Copyright (C) 2008-2014 Jedox AG
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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.extract;

import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;

/**
 * @author khaddadin
 *
 */
public class TableFilterConfig {
	
	public static Object[] getFilters(Element xml) throws ConfigurationException{
			
		Element filtersElement = xml.getChild("filters");
		if(filtersElement==null)
			return new Object[0];
		
		List<?> filters = filtersElement.getChildren("filter");
		String[] filtersDef = new String[filters.size()];
		for(int i=0;i<filters.size();i++){
			Element column = (Element) filters.get(i);
			String columnName = column.getAttributeValue("name");
			Element filter = column.getChild("condition");
			String operand=filter.getAttributeValue("operator");
			String filtervalue=filter.getAttributeValue("value");
			
			boolean isNull = filtervalue.equalsIgnoreCase("null");
			if (operand==null || filtervalue==null)
				throw new ConfigurationException("Error in Filter Expression");
			
			switch(operand){
				case "Equals":
					filtersDef[i] = "(" +columnName + " = " + (!isNull?"'":"") + filtervalue + (!isNull?"'":"") +")"; 	
					break;
				case "Not equals":
					filtersDef[i] = "(" +columnName + " != " + (!isNull?"'":"") + filtervalue + (!isNull?"'":"") + ")"; 	
					break;
				case "Less than":
					filtersDef[i] = "(" +columnName + " < '" + filtervalue + "')"; 	
					break;
				case "Less or equal":
					filtersDef[i] = "(" +columnName + " <= '" + filtervalue + "')"; 	
					break;
				case "Greater than":
					filtersDef[i] = "(" +columnName + " > '" + filtervalue + "')"; 	
					break;
				case "Greater or equal":
					filtersDef[i] = "(" +columnName + " >= '" + filtervalue + "')"; 	
					break;
				case "Like":
					filtersDef[i] = "(" +columnName + " LIKE '" + filtervalue + "')"; 	
					break;
				case "IN":
					filtersDef[i] = "(" +columnName + " IN (" + filtervalue + "))"; 	
					break;
				case "NOT IN":
					filtersDef[i] = "(" +columnName + " NOT IN (" + filtervalue + "))"; 	
					break;
				case "INCLUDES":
					filtersDef[i] = "(" +columnName + " includes (" + filtervalue + "))"; 	
					break;
				case "EXCLUDES":
					filtersDef[i] = "(" +columnName + " excludes (" + filtervalue + "))"; 	
					break;
				default:
					throw new ConfigurationException("Operator " + operand + " not allowed.");
			}
			
		}
		return filtersDef;
	}

		
	public static String evaluateFiltersLogic(String filtersLogic, int conditionsCount) throws ConfigurationException {
		if (filtersLogic==null || filtersLogic.isEmpty()) {
			return null;
		}	
		filtersLogic = filtersLogic.trim().toUpperCase();
		if (filtersLogic.isEmpty()) {
			return null;
		}	
		if (!filtersLogic.matches("(\\(|\\)|\\d|NOT|AND|OR|\\s)*")) {
			throw new ConfigurationException("Invalid filter logic \""+filtersLogic+"\". Valid symbols are only: [0-9] ( ) AND OR NOT");
		}
		if (!filtersLogic.matches(".*\\d.*"))
			throw new ConfigurationException("Filter logic \""+filtersLogic+"\" does not contain any row index");
		if(conditionsCount==0)
			throw new ConfigurationException("Filterlogic is given but no conditions exist.");
					
		filtersLogic = " "+filtersLogic;
		Pattern p = Pattern.compile("\\d+");
		Matcher m = p.matcher(filtersLogic);
		String result= filtersLogic;
		
		while (m.find()) {
			int index = Integer.parseInt(m.group());
			if (index<0 || index>conditionsCount) 
				throw new ConfigurationException("Invalid row index "+index+" in filter logic \""+filtersLogic+"\"");
			result = result.replaceFirst("[\\(\\)\\s]" + m.group() , Matcher.quoteReplacement(filtersLogic.charAt(m.start()-1) + "%" + m.group() + "$s"));
		}
		return result;
	}
	
	public static String getWhereClause(Element xml) throws ConfigurationException{
	
		Object[] conditions = getFilters(xml);
		
		String filterLogic = evaluateFiltersLogic(xml.getChildText("filterslogic"),conditions.length);
		
		if(filterLogic!=null && !filterLogic.isEmpty() && conditions.length==0){
			throw new ConfigurationException("Filterlogic is given but no conditions exist.");
		}
		
		if((filterLogic==null || filterLogic.isEmpty()) && conditions.length!=0){
			// create default filter logic with AND
			filterLogic = "";
			for (int i=0; i<conditions.length; i++) {
				filterLogic = filterLogic + "%"+(i+1)+"$s" + ((i<conditions.length-1)?" AND ":"");
			}
		}
		
		if(conditions.length>0)
			return " where " + String.format(filterLogic, conditions);
		else 
			return "";
		
		
	}

}
