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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.prototype;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.ExecutionException;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.ResultCodes.DataTargets;
import com.jedox.etl.core.node.IColumn;

public class FileToOlapModel {
	
	private String tempProjectName = null;
	
	public static enum ColumnRole { 
		
		MES("Measure column"), ATTR("Attribute column"), DATE ("DATE dimension"), IGNORE ("IGNORE"), DIM ("Dimension"),DRILL ("Drillthrough column");
		private String description;
		public String getDescription(){return description;}
		private ColumnRole(String description){this.description = description;}
		} 
	
	public static class Coordinate {
		Coordinate(String name, String nameref, int columnId) {
			this.name=name;
			this.nameref=nameref;
			this.columnId = columnId;
		}
		public String name;
		public String nameref;
		public int columnId;
	}
		
	
	public class TimeFormat {
		TimeFormat(String columnName, String sourceformat, String targetformat,String language ,String start, String end, int columnId) throws ConfigurationException {
			this.columnName=columnName;
			this.sourceformat=sourceformat.replaceAll("Y", "y").replaceAll("D", "d");
			try{
				new SimpleDateFormat(this.sourceformat);
			}catch(Exception e){
				throw new ConfigurationException("Date format \"" + sourceformat + "\" is not valid: " + e.getMessage());
			}
			this.targetformat=targetformat.replaceAll("Y", "y").replaceAll("D", "d");;	
			try{
				new SimpleDateFormat(this.targetformat);
			}catch(Exception e){
				throw new ConfigurationException("Date format \"" + targetformat + "\" is not valid: " + e.getMessage());
			}

			if(language!=null) {
				this.language = language.toLowerCase();
				try{
					org.apache.commons.lang.LocaleUtils.toLocale(this.language);
				}
				catch(Exception e){
					throw new ConfigurationException("Language " + language + " is not a valid language.");
				}
			}
			
			try{
				if(start!=null)
					Integer.parseInt(start);
			}
			catch(Exception e){
				throw new ConfigurationException("Start year should be an integer");
			}
			
			try{
				if(end!=null)
					Integer.parseInt(end);
			}	
			catch(Exception e){
				throw new ConfigurationException("End year should be an integer");
			}
			this.start=start;
			this.end=end;
			this.columnId = columnId;
		}
		public String columnName;
		public String sourceformat;
		public String targetformat;
		public String language;
		public String start;
		public String end;
		public int columnId;

	}	
	
	public FileToOlapModel(Element project, String source) throws ConfigurationException {
		String origName=project.getAttributeValue("name");
		try{		
			LinkedList<String> columnNames = readFileColumnNames(project, source);
			setupModel(columnNames);
		}finally {
	        	ConfigManager.getInstance().removeElement(Locator.parse(tempProjectName));
	        	project.setAttribute("name", origName);
	        	project.detach();
	    }
	}
	
	
	private List<Coordinate> measures = new ArrayList<Coordinate> ();	
	private LinkedHashMap<String, Dimension> dimensions = new LinkedHashMap<String, Dimension> ();
	private Map<Integer, String> aliasDefaults = new HashMap<Integer, String> (); 
	private Coordinate valueColumn;
	private TimeFormat timeFormat;
	
	public List<Coordinate> getCoordinates() {
		List<Coordinate> list = new ArrayList<Coordinate>();		
	    for (String dimname : dimensions.keySet()) {
	    	Dimension d = dimensions.get(dimname);	    	
	    	Coordinate coord = new Coordinate(dimname,d.getBaseLevel(),d.getBaseLevelColumnId());
	    	list.add(coord);
	    }
		return list;		
	}
	
	public List<Coordinate> getMeasures() {
		return measures;
	}
	
	public  Map<Integer, String> getAliasDefaults() {
		return aliasDefaults;
	}	
	
	public Coordinate getValueColumn() {
		return valueColumn;
	}
	
	public TimeFormat getTimeFormat() {
		return timeFormat;
	}	
		
	public List<String> getDimNames(ColumnRole role) {
		List<String> list = new ArrayList<String>();
		for (String dimname : dimensions.keySet()) {
			if (dimensions.get(dimname).getRole().equals(role))
				list.add(dimname);
		}
		return list;
	}	
	
	public Dimension getDimension(String dimname) {
		return dimensions.get(dimname);
	}
	
	
	public List<Integer> getCubeColumnIds(){
		List<Integer> ids = new ArrayList<Integer>();
				
		for(Coordinate m:measures)
			ids.add(m.columnId);
		
		if(valueColumn!=null)
			ids.add(valueColumn.columnId);
		
		List<Coordinate> cordinates = getCoordinates();
		for(Coordinate co:cordinates)
			ids.add(co.columnId);

		return ids;
	}
	
	public List<Integer> getAllColumnIds(){
		List<Integer> ids = new ArrayList<Integer>();
				
		for(Coordinate m:measures)
			ids.add(m.columnId);
		
		if(valueColumn!=null)
			ids.add(valueColumn.columnId);
		
		for(Dimension dim:dimensions.values())
			ids.addAll(dim.getAllColumnIds());

		return ids;
	}
	
	
/////////////	

		
	private String getBeforeBracket(String column) {
		if (column.endsWith("]"))
			return column.split("\\[")[0];
		else
			return column;
	}	
	
	private String[] getBrackets(String column)  throws ConfigurationException {
		String[] splits = column.split("\\[");
		if (splits.length!=2)
			throw new ConfigurationException("Invalid column name "+column);
		String bracket = splits[1].substring(0,splits[1].length()-1);
		return bracket.split(",");
	}

	private ColumnRole getColumnRole(String column) throws ConfigurationException {
		if (column.endsWith("]")) {
			String index=getBrackets(column)[0];
				try{
					int indexNum = Integer.parseInt(index);
					if(indexNum==0){
						throw new ConfigurationException("Dimension level can not be 0");
					}
					return ColumnRole.DIM;
				}catch(NumberFormatException e1){
					try{
						return ColumnRole.valueOf(index.toUpperCase());
					}catch(Exception e){
						throw new ConfigurationException("Role " + index + " is not a valid role. Possible values are: " + Arrays.asList(ColumnRole.values()));
					}
				}
			
		}
		else return ColumnRole.IGNORE;
		
	}	
	
	private String validateName(String name) throws ConfigurationException{
		if(name.contains(".")){
			throw new ConfigurationException("Column " + name + " is invalid, dot is not allowed in column name");
		}
		return name;
	}
	
	private String getAttributeName(String column) throws ConfigurationException {
		String[] brackets = getBrackets(column);
		String attName = null;
		if (brackets.length<4)
			attName = getBeforeBracket(column);
		else
			attName = brackets[3];
		return validateName(attName);
	}	
	
	private String getMeasureName(String column) throws ConfigurationException {
		String[] brackets = getBrackets(column);
		String measureName = null;
		if (brackets.length==1)
			measureName = getBeforeBracket(column);
		else
			measureName = brackets[1];
		return validateName(measureName);
	}		
	
	private Integer getIndex(String column, ColumnRole role) throws ConfigurationException {
		String brackets[]=getBrackets(column);
		String indexString;
		if (role.equals(ColumnRole.ATTR))
			indexString = brackets[1];
		else 
			indexString = brackets[0];
		try {
			return Integer.valueOf(indexString);				
		} catch (NumberFormatException e) {
			throw new ConfigurationException(e.getMessage());
		}	
	}
	
	private String getDim(String column, ColumnRole role) throws ConfigurationException {
		if (column.endsWith("]")) {
			String[] brackets=getBrackets(column);
			String dimName="";
			if (role.equals(ColumnRole.ATTR) && brackets.length>=3) 
				dimName = brackets[2];
			else if (role.equals(ColumnRole.DIM) && brackets.length>=2)				
				dimName = brackets[1];
			else if (role.equals(ColumnRole.DIM) && brackets.length==1)
				dimName = getBeforeBracket(column);
			else if (role.equals(ColumnRole.DATE) || role.equals(ColumnRole.DRILL))
				dimName = getBeforeBracket(column);	
			
			if (dimName.isEmpty())
				throw new ConfigurationException("No dimension name found in column: "+column);
			return validateName(dimName);
		}
		else
			return column;
	}

	private Integer getConcatenateLevel(String column, ColumnRole role) throws ConfigurationException {
		String brackets[]=getBrackets(column);
		if (role.equals(ColumnRole.DIM) && brackets.length>=3 && brackets[2].equalsIgnoreCase("CONCAT")) {
			if (brackets.length==3)
				return 0;
			else {
				try {
					return Integer.valueOf(brackets[3]);				
				} catch (NumberFormatException e) {
					throw new ConfigurationException(e.getMessage());
				}				
			}
		}
		else
			return null;
	}
	
	
	private void setTimeFormat(String columnName, int columnId) throws ConfigurationException {
		if (timeFormat!=null)
			throw new ConfigurationException("Duplicate time column: "+columnName);					
		String[] brackets = getBrackets(columnName);
		if (brackets.length<2)
			throw new ConfigurationException("Invalid time column: "+columnName);		
		timeFormat = new TimeFormat(columnName, brackets[1], (brackets.length>=3)?brackets[2]:brackets[1], (brackets.length>=4)?brackets[3]:null, (brackets.length>=5)?brackets[4]:null, (brackets.length>=6)?brackets[5]:null,columnId+1);
	}	
	
				
	private LinkedList<String> readFileColumnNames (Element projectConfig, String extract) throws ConfigurationException {
		tempProjectName ="#_GEN_"+new Date().getTime() +"_temp";
	
		projectConfig.setAttribute("name", tempProjectName);		
		ConfigManager.getInstance().add(Locator.parse(tempProjectName), projectConfig);
				
        try {		
        	Execution e = Executor.getInstance().createOutputDescription(Locator.parse(tempProjectName+".extracts."+extract),new Properties(),null);
        	e.getExecutionState().setDataTarget(DataTargets.CSV_INLINE); //write directly to stdout
        	Executor.getInstance().addExecution(e);
        	Executor.getInstance().runExecution(e.getKey());
        	ExecutionState state = Executor.getInstance().getExecutionState(e.getKey(), true);
        	LinkedList<String> columnNames = new LinkedList<String>();
        	if (state.getErrors()==0) {
        		for (IColumn column : state.getMetadata().getColumns()) {
        			//if(column.getName().contains(".")){
        			//	throw new ConfigurationException("Column " + column.getName() + " is invalid, dot is not allowed in column name");
        			//}
       				columnNames.add(column.getName());
        		}
        		return columnNames;
        	} else {
        		throw new ExecutionException("Error in determing file columns: "+state.getFirstErrorMessage());
        	}
        } catch (ExecutionException e) {
        	throw new ConfigurationException(e.getMessage());
        }        	
	}
	
   private Dimension putDimension(String columnName, ColumnRole role) throws ConfigurationException {
	   ColumnRole targetRole = (role.equals(ColumnRole.ATTR) ? ColumnRole.DIM : role);
	   Dimension d = dimensions.get(getDim(columnName, role));
	   if (d==null) {
		   d=new Dimension();
		   dimensions.put(getDim(columnName, role), d);
		   d.setRole(targetRole);
	   }else{
		   if(!d.getRole().equals(targetRole))
			   throw new ConfigurationException("Column " + columnName + "  is already mapped as a " + d.getRole().getDescription());
	   }
	   return d;	   
   }	
		
   public void setupModel(List<String> columnNames) throws ConfigurationException {	   

	   HashSet<String> measuresSet = new HashSet<String>();
	   for (String columnName : columnNames) {
		   ColumnRole role = getColumnRole(columnName);
		   if (role.equals(ColumnRole.MES)) {
			   if(measuresSet.contains(getMeasureName(columnName)))
				   throw new ConfigurationException("Measure " + getMeasureName(columnName) + " is defined in more than one column.");
			   measuresSet.add(getMeasureName(columnName));
		   }   
	   }	   
	   int maxcolumns = columnNames.size();	   
	   if (measuresSet.isEmpty()) {
		   valueColumn = new Coordinate(columnNames.get(maxcolumns-1),columnNames.get(maxcolumns-1),maxcolumns);
		   maxcolumns = columnNames.size()-1;
	   }
	   if (maxcolumns==0)
		   throw new ConfigurationException("No dimension column found.");
	   
	   String blancStr = "(blank)";	   
	   for (int i=0; i<maxcolumns; i++) {
		   String columnName=columnNames.get(i);
		   ColumnRole role = getColumnRole(columnName);	   
		      		   
		   switch (role) { 
		   case DIM: {
			   Dimension d = putDimension(columnName,role);
			   Integer index = getIndex(columnName, role);
			   d.putLevel(index, columnName,i+1);
			   aliasDefaults.put(i+1, blancStr+(index==1?"":" L"+index));
			   d.setConcatenate(getIndex(columnName, role), getConcatenateLevel(columnName, role));
			   break;		   
		   	}
		   case ATTR: {
			   Dimension d = putDimension(columnName,role);			   
			   d.addAttribute(getIndex(columnName, role), new Coordinate(getAttributeName(columnName), columnName,i+1));
			   break;
		   }
		   case DRILL: {
			   Dimension d = putDimension(columnName,role);			   
			   d.putLevel(1, columnName,i+1);
			   break;
		   }
		   case DATE: {
			   Dimension d = putDimension(columnName,role);			   
			   d.putLevel(1, getDim(columnName, role),i+1);
			   setTimeFormat(columnName,i+1);
			   aliasDefaults.put(i+1, blancStr);			   
			   break;
		   }
		   case MES: {
			   measures.add(new Coordinate(getMeasureName(columnName),columnName,i+1));
			   aliasDefaults.put(i+1, "0");			   
			   break;			   
		   }
		   case IGNORE: {
		   }
		   }		   
	   }
	   
	   if (getDimNames(ColumnRole.DIM).isEmpty() && getDimNames(ColumnRole.DATE).isEmpty())
		   throw new ConfigurationException("No dimension is defined. The header line should contain e.g. "+columnNames.get(0)+"[1]");
		   	   
	   for (Dimension d : dimensions.values()) {
		   d.checkConsistency();		   
		   d.addGenericAttribute();		   
	   }
	   
   }
	
}
