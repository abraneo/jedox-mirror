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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;

import org.apache.http.NameValuePair;
import org.jasypt.util.text.BasicTextEncryptor;
import org.jdom.Document;
import org.jdom.Element;

import com.jedox.etl.components.prototype.FileToOlapModel.ColumnRole;
import com.jedox.etl.components.prototype.FileToOlapModel.Coordinate;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.prototype.IPrototype;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.URLUtil;

public abstract class FileToOlap extends BasePrototype implements IPrototype {

	protected Properties params = new Properties();
	//private static final Log log = LogFactory.getLog(FileToOlap.class);
	
		
	protected void addJedoxConnection (String name, String user,String password, String database) throws ConfigurationException {
		Element component = addComponent(ITypes.Connections, name, "Jedox");
		addChild(component,"host", Settings.getInstance().getSuiteConnectionConfiguration().getHost());
		addChild(component,"port",  Settings.getInstance().getSuiteConnectionConfiguration().getPort());
		addChild(component,"user", user);
		try {
			BasicTextEncryptor crypt = new BasicTextEncryptor();
			crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));
			password = crypt.encrypt(password);
		}
		catch (Exception e) {
			throw new ConfigurationException("Encryption of password for connection "+name+" failed.");
		}
		Element child = addChild(component,"password", password);
		child.setAttribute("encrypted", "true");
		addChild(component,"database", database);
	}
	
	protected void addJedoxGlobalConnection (String name, String globalReference, String database) {
		Element component = addComponent(ITypes.Connections, name, "JedoxGlobal");
		addChild(component,"globalReference", globalReference);
		addChild(component,"database", database);
	}
	
	protected void addConstantTreeExtract (String name, String[][] rows) {
		Element component = addComponent(ITypes.Extracts, name, "ConstantTree");
		Element data=addChild(component,"data");
		Element headerEl=new Element("header");
		data.addContent(headerEl);
		String[] headerRow = new String[]{":level",":element",":weight",":nodetype"};
		for (String value : headerRow) {
			addChild(headerEl,"value",value);
		}
		for (String[] dataRow : rows) { 
			Element dataEl=new Element("row");
			data.addContent(dataEl);
			for (String value : dataRow) {
				addChild(dataEl,"value",value);
			}			
		}
	}
	
	protected void addCalendarExtract (String name, String root, String years, String quarters, String months, String days, String start, String end, String monthsYTD, String language) {
		Element component = addComponent(ITypes.Extracts, name, "Calendar");
		Element options = addChild(component,"options");
		addChild(options,"root",root);
		if(language!=null)
			addChild(options,"language",language);
		Element levels = addChild(component, "levels");
		if (years!=null) {
			Element yearsEl = addChild(levels,"years");
			addChild(yearsEl, "pattern", years);
			addChild(yearsEl, "start", start);
			addChild(yearsEl, "end", end);			
		}	
		if (quarters!=null)
			addChild(addChild(levels, "quarters"), "pattern", quarters);
		if (months!=null) {
			Element monthsEl = addChild(levels, "months");
			addChild(monthsEl, "pattern", months);
			if (monthsYTD!=null) {
				addChild(addChild(monthsEl, "timetodate"), "pattern", monthsYTD);	
				addChild(options, "TTDmode", "toNext");
			}
		}	
		if (days!=null)
			addChild(addChild(levels, "days"), "pattern", days);		
	}

	protected void addFieldTransform (String name, String nameref) {
		Element component = addComponent(ITypes.Transforms, name, "FieldTransform");
		Element sources=new Element("sources");
		addNameref(sources, ITypes.Sources, nameref);
		component.addContent(sources);
	}
	
	protected void addFieldTransform (String name, String nameref, List<Coordinate> coordinates, Coordinate constantCoordinate, Coordinate valueCoordinate) {
		Element component = addComponent(ITypes.Transforms, name, "FieldTransform");
		Element sources=addChild(component,("sources"));
		addNameref(sources, ITypes.Sources, nameref);
		Element target=addChild(component,("target"));
		Element coordinatesEl=addChild(target,("coordinates"));
		for (Coordinate coordinate : coordinates) {
			Element element=addChild(coordinatesEl,"coordinate");
			element.setAttribute("name", coordinate.name);
			addInput(element, true, coordinate.nameref);						
		}
		if (constantCoordinate!=null) {
			Element element=addChild(coordinatesEl,"coordinate");
			element.setAttribute("name", constantCoordinate.name);
			addInput(element, false, constantCoordinate.nameref);									
		}
		if (valueCoordinate!=null) {
			Element element=addChild(coordinatesEl,"coordinate");
			element.setAttribute("name", valueCoordinate.name);
			addInput(element, true, valueCoordinate.nameref);									
		}
	}

	protected void addFunctionDateFormat (String name, String transformName, String inputNameref, String sourceformat, String targetformat, String language) {
		Element function = addFunction(name, transformName, "DateFormat");
		addInput(function.getChild("inputs"), true, inputNameref) ;
		addChild(function.getChild("parameters"),"sourceformat",sourceformat);
		addChild(function.getChild("parameters"),"targetformat",targetformat);
		if(language!=null)
			addChild(function.getChild("parameters"),"language",language);
	}

	protected void addFunctionConcatenation (String name, String transformName, List<String> concatList) {
		Element function = addFunction(name, transformName, "Concatenation");
		for (String concat : concatList) {
			addInput(function.getChild("inputs"), true, concat);
		}	
		addChild(function.getChild("parameters"),"delimiter","-");
	}
	
	
	protected void addTreeFHTransform (String name, String nameref, String rootLevel, Collection<String> levelNamerefs, Collection<String> attributeNames, List<List<Coordinate>> attributes) {
		Element component = addComponent(ITypes.Transforms, name, "TreeFH");
		Element sources=addChild(component,("sources"));
		addNameref(sources, ITypes.Sources, nameref);
		Element target=addChild(component,("target"));
		Element levels=addChild(target,"levels");
		Element level=addChild(levels, "level");		
		addInput(level, false, rootLevel);
		int index=0;
		for (String levelNameref : levelNamerefs) {
			level=addChild(levels,"level");
			addInput(level, true, levelNameref);
			List<Coordinate> attributeList = attributes.get(index);
			if (attributeList!=null) {
				Element attrs = addChild(level, "attributes");
				for (Coordinate coord : attributeList) {
					Element attr=addChild(attrs,"attribute");
					attr.setAttribute("name", coord.name);
					addInput(attr,true,coord.nameref);
				}					
			}		
			index++;
		}
		if (!attributeNames.isEmpty()) {
			Element attrs=addChild(target,"attributes");
			for (String attributeName : attributeNames) {
				Element attr=addChild(attrs,"attribute");	
				attr.setAttribute("name", attributeName);
			}
		}		
	}

	protected void addTableNormalization (String name, String nameref, List<Coordinate> coordinates, Coordinate constantCoordinate, List<Coordinate> measures, String measureName) {
		Element component = addComponent(ITypes.Transforms, name, "TableNormalization");
		Element sources=addChild(component,("sources"));
		addNameref(sources, ITypes.Sources, nameref);
		Element target=addChild(component,("target"));
		Element coordinatesEl=addChild(target,("coordinates"));
		for (Coordinate coordinate : coordinates) {
			Element element=addChild(coordinatesEl,"coordinate");
			element.setAttribute("name", coordinate.name);
			addInput(element, true, coordinate.nameref);						
		}
		if (constantCoordinate!=null) {
			Element element=addChild(coordinatesEl,"coordinate");
			element.setAttribute("name", constantCoordinate.name);
			addInput(element, false, constantCoordinate.nameref);									
		}
		Element measuresEl=addChild(target,("measures"));
		measuresEl.setAttribute("normalize", measureName);
		for (Coordinate measure : measures) {
			Element element=addChild(measuresEl,"measure");
			element.setAttribute("name", measure.name);
			addInput(element, true, measure.nameref);						
		}
	}
		
	protected void addDimensionLoad (String name, String nameref, String connection, String dimname, boolean addMode) {
		Element component = addComponent(ITypes.Loads, name, "Dimension");
		addNameref(component, ITypes.Sources, nameref);
		addNameref(component, ITypes.Connections, connection);
		Element dimension=addChild(component,"dimension");
		dimension.setAttribute("name", dimname);
		Element step=addChild(dimension,"elements");
		step.setAttribute("mode", (addMode) ? "add" : "create");
		step=addChild(dimension,"consolidations");
		step.setAttribute("mode", (addMode) ? "add" : "update");
		step=addChild(dimension,"attributes");
		step.setAttribute("mode", (addMode) ? "add" : "update");
	}
	
	protected void addCubeExtract (String name,String connection,String cubeName,String valueName, boolean drillthrough,boolean ignoreEmptyCells,boolean onlyBasisAsDefault,boolean useRules) {
		Element component = addComponent(ITypes.Extracts, name, "Cube");
		addNameref(component, ITypes.Connections, connection);
		Element query=addChild(component,"query");
		query.setAttribute("drillthrough", String.valueOf(drillthrough));
		query.setAttribute("ignoreEmptyCells", String.valueOf(ignoreEmptyCells));
		query.setAttribute("onlyBasisAsDefault", String.valueOf(onlyBasisAsDefault));
		query.setAttribute("useRules", String.valueOf(useRules));
		Element cube=addChild(query,"cube");
		cube.setAttribute("name", cubeName);
		cube.setAttribute("valuename", valueName);
	}


	protected void addCubeLoad (String name, String nameref, String connection, String cubename, List<String> columnNames, List<String> drillNames, boolean isAdd) {
		Element component = addComponent(ITypes.Loads, name, "Cube");
		addNameref(component, ITypes.Sources, nameref);
		addNameref(component, ITypes.Connections, connection);
		Element cube=addChild(component,"cube");
		cube.setAttribute("name", cubename);
		addChild(component,"mode",(isAdd?"insert":"create"));
		if (!drillNames.isEmpty()) {
			Element drillthrough=addChild(cube,"drillthrough");
			drillthrough.setAttribute("aggregate", "false");
			Element dimensions=addChild(cube,"dimensions");
			for (String columnName : columnNames) {
				Element dimension=addChild(dimensions,"dimension");
				dimension.setAttribute("input", columnName);
				if (!drillNames.contains(columnName))
					dimension.setAttribute("name", columnName);
			}			
		}
	}
		
	protected void addStandardJob (String name, String[] loads) {
		Element component = addComponent(ITypes.Jobs, name, "Standard");
		for (String load : loads) {
			Element execution=addChild(component,"execution");
			execution.setAttribute("type", "load");
			execution.setAttribute("nameref", load);			
		}
	}
		
	public abstract Element addExtract (String extractName);
	public abstract void  addTargetDateFormat(Element extract, String targetDateFormat);
	public abstract Element addConnection();
	public abstract String addExtractWithColumnIds(Element extract,List<Integer> columnIds,String newName);
	
	public void addAliasMapToExtract(Element extract, Map<Integer, String> aliasDefaults) {
	   if(extract.getChild("alias_map")!=null)
		   extract.removeChild("alias_map");
	   Element aliasMap = new Element("alias_map"); 
	   for (Integer index : aliasDefaults.keySet()) {
			Element aliasMapEntry = new Element("alias");
			aliasMapEntry.setText(String.valueOf(index));
			aliasMapEntry.setAttribute("default", aliasDefaults.get(index));
			aliasMap.addContent(aliasMapEntry);
		}
	   extract.addContent(aliasMap);		
	}
	
	
	protected void setObligatoryParameter(String name, Properties parameters) throws ConfigurationException {
		if (!parameters.containsKey(name))
			throw new ConfigurationException("Parameter "+name+" is missing.");
		params.setProperty(name, parameters.getProperty(name));		
	}

	
	// override user/password from the url information if it is possible
	// the USER & PASSWORD from the parameters have higher priority	
	protected void setUserFromFile(Properties parameters, String filename) throws ConfigurationException {
		if (!(parameters.contains("USER") || parameters.contains("PASSWORD"))
				&& FileUtil.isURL(filename)) {
			String urlUser = null;
			String urlPassword = null;
			try {
				List<NameValuePair> urlParameters = URLUtil.getInstance().getParameters(filename, "UTF-8");
				for (NameValuePair p : urlParameters) {
					if (p.getName().equals("user"))
						urlUser = p.getValue();
					else if (p.getName().equals("pass"))
						urlPassword = Settings.getInstance().decrypt(p.getValue());
				}
			} catch (Exception e) {
				throw new ConfigurationException("Error in determination of user credentials from "+filename+": "+e.getMessage());
			}
			if (urlUser!=null)
				params.setProperty("USER", urlUser);
			else if (!parameters.contains("USER")) // Exception if user no found in filename and not set as parameter  
				throw new ConfigurationException("Could not determine user from "+filename);
			if (urlPassword!=null)
				params.setProperty("PASSWORD",urlPassword);
			else if (!parameters.contains("PASSWORD")) // Exception if password no found in filename and not set as parameter
				throw new ConfigurationException("Could not determine user from "+filename);
		}	
	}
	
	protected void setParameters(Properties parameters) throws ConfigurationException {
		// default values for optional parameters
		params.setProperty("DATABASE", getClass().getSimpleName());
		params.setProperty("CUBE","Data");
		params.setProperty("HASDATATYPESDIM","true");
		params.setProperty("STARTYEAR","2010");
		params.setProperty("ENDYEAR","2015");
		params.setProperty("YTD","true");
		params.setProperty("ISADD","false");
		params.setProperty("USER","");
		params.setProperty("PASSWORD","");
		// for CSV only
		params.setProperty("DELIMITER",";");
		// for Excel only
		params.setProperty("RANGE","");
		params.setProperty("QUOTE","");
		params.setProperty("ENCODING","");
		
		// set obligatory parameter
		String filename=parameters.getProperty("FILENAME");
		if (filename==null)
			throw new ConfigurationException("Parameter FILENAME is missing.");
		params.setProperty("FILENAME", filename);	
		
		setUserFromFile(parameters, filename);			
		
		// Overwrite default values
		for (Object par : parameters.keySet()) {
			String parstr = (String)par;
			if (!params.containsKey(par))
				throw new ConfigurationException("Invalid parameter: "+parstr);
			String value=parameters.getProperty(parstr);
			if (!value.isEmpty())
				params.setProperty(parstr, parameters.getProperty(parstr));
		}
		
		try{
			Integer.parseInt(params.getProperty("STARTYEAR"));
		}
		catch(Exception e){
			throw new ConfigurationException("Start year should be an integer");
		}
		
		try{
			Integer.parseInt(params.getProperty("ENDYEAR"));
		}	
		catch(Exception e){
			throw new ConfigurationException("End year should be an integer");
		}
		
		String lang = null;
		try{
			lang = params.getProperty("LANGUAGE");
			if(lang!=null)
				new Locale(lang);
		}
		catch(Exception e){
			throw new ConfigurationException("Language " + lang + " is not a valid language.");
		}
		
		
	}
	
	protected FileToOlapModel model = null;
	
	@Override
	public Document generate(String projectName, Properties parameters) throws ConfigurationException {
		setParameters(parameters);
		initEmptyProject(projectName, "ETL project for "+getClass().getSimpleName()+", generated on "+new Date().toString());
	    addConnection();
	    Element extract = addExtract("E_Data");
	    model= new FileToOlapModel(project, "E_Data");
	    addTargetDateFormat(extract, model.getTimeFormat()!=null?model.getTimeFormat().sourceformat:null);
	    addAliasMapToExtract(extract, model.getAliasDefaults());	    
	    
	    boolean isAdd = Boolean.valueOf(params.getProperty("ISADD"));
		addJedoxConnection("C_Olap", params.getProperty("USER"),params.getProperty("PASSWORD"), params.getProperty("DATABASE"));		

		for (String dimname : model.getDimNames(ColumnRole.DIM)) {
	
			Dimension d = model.getDimension(dimname);
			//String extractName = addExtractWithColumnIds(extract, d.getAllColumnIds(), "E_Data_" +dimname);
			addFieldTransform("FT_"+dimname, "E_Data");
			
			List<String> treeLevels = new ArrayList<String>();
			if (d.hasConcatenatedLevel()) {
				int count=2;
				for (String levelNameref : d.getLevelNamerefs()) {
					List<String> concats = d.getConcatsForLevel(levelNameref);
					if (!concats.isEmpty()) {
						String name="Concat_Level_"+count;
						treeLevels.add(name);
						addFunctionConcatenation(name, "FT_"+dimname, concats);					
					}
					else {
						treeLevels.add(levelNameref);
					}
					count++;
				}				
			}
			else {
				treeLevels=d.getLevelNamerefs();
			}
			
			addTreeFHTransform("Tree_"+dimname,  "FT_"+dimname, "All "+dimname, treeLevels, d.getAttributesNames(), d.getAttributesForLevel());					
			addDimensionLoad("L_"+dimname,  "Tree_"+dimname, "C_Olap", dimname, isAdd);					
		}
		
		Coordinate constantCoordinate = null;			
		if (params.getProperty("HASDATATYPESDIM").equals("true") && model.getDimension("Datatypes")==null) {
			String[][] dataRows =  { {"1","Variance","1"} , {"2","Actual","1"} , {"2","Budget","-1"}};			
			addConstantTreeExtract("E_Datatypes", dataRows);
			addDimensionLoad("L_Datatypes", "E_Datatypes", "C_Olap", "Datatypes", isAdd);
			constantCoordinate = new Coordinate("Datatypes", "Actual",-1);
		}
		
		String extractName = addExtractWithColumnIds(extract, model.getAllColumnIds(), "E_Data");
		addFieldTransform("FT_Data", extractName);
		
		for (String dimname : model.getDimNames(ColumnRole.DIM)) {
			Dimension d = model.getDimension(dimname);
			List<String> concats = d.getConcatsForLevel(d.getBaseLevel());
			if (!concats.isEmpty()) {
				addFunctionConcatenation(d.getBaseLevel(), "FT_Data", concats);					
			}
		}	
					
		if (model.getTimeFormat()!=null) {
			String format=model.getTimeFormat().targetformat;
			String formatYear = format.replaceAll("[^y]", "");
			String formatMonth = format.replaceAll("[^M]", "");
			
			String years = (format.contains("y")) ? ((format.contains("M")) ? "yyyy" : formatYear) : null;
			String quarters = (format.contains("y") && format.contains("M")) ? "'Q'Q "+formatYear : null;
			String months=null;
			if (format.contains("M"))
				months = ((format.contains("d")) ? ((format.contains("y")) ? formatMonth+" "+formatYear : formatMonth) : format);
			String days = (format.contains("d") ? format : null);
			String root = (format.contains("y")) ? "All Years" : "All";
			String language = (model.getTimeFormat().language!=null ? model.getTimeFormat().language : params.getProperty("LANGUAGE"));
			String start = (model.getTimeFormat().start!=null ? model.getTimeFormat().start : params.getProperty("STARTYEAR"));
			String end = (model.getTimeFormat().end!=null ? model.getTimeFormat().end : params.getProperty("ENDYEAR"));
			addCalendarExtract("E_Date", root, years, quarters,  months, days, start, end, null, language);
			addDimensionLoad("L_Date", "E_Date", "C_Olap", model.getDimNames(ColumnRole.DATE).get(0),isAdd);
			if (params.getProperty("YTD").equals("true") && months!=null) {
				String yearsYTD = years.concat(" 'YTD'");
				String monthsYTD = months.concat(" 'YTD'");
				String rootYTD = root.concat(" YTD");				
				addCalendarExtract("E_Date_YTD", rootYTD, yearsYTD, null, months, null, start,end, monthsYTD, language);
				addDimensionLoad("L_Date_YTD", "E_Date_YTD", "C_Olap", model.getDimNames(ColumnRole.DATE).get(0),true);
			}			
			addFunctionDateFormat(model.getDimNames(ColumnRole.DATE).get(0), "FT_Data", model.getTimeFormat().columnName, model.getTimeFormat().sourceformat, model.getTimeFormat().targetformat, language);			
		}	
				
		String cubeLoadSource;		
		if (!model.getMeasures().isEmpty()) {
			List<String[]> rows = new ArrayList<String[]>();
			for (Coordinate coord : model.getMeasures()) {
				String[] row = new String[3];
				row[0]="1"; row[1]=coord.name; row[2]="1";
				rows.add(row);
			}
			addConstantTreeExtract("E_Measures", rows.toArray((new String[rows.size()][])));			 
			addDimensionLoad("L_Measures", "E_Measures", "C_Olap", "Measure", isAdd);
			
			cubeLoadSource="T_Normalization";
			addTableNormalization(cubeLoadSource, "FT_Data", model.getCoordinates(), constantCoordinate, model.getMeasures(), "Measure");
		 } else {
			cubeLoadSource="FT_Cubedata";
			addFieldTransform(cubeLoadSource, "FT_Data", model.getCoordinates(), constantCoordinate, model.getValueColumn());			 
		 }
		
		List<String> allColumnNames = new ArrayList<String>();
		for (Coordinate coord : model.getCoordinates()) {
			allColumnNames.add(coord.name);
		}
		if (constantCoordinate!=null)
			allColumnNames.add(constantCoordinate.name);
		if (!model.getMeasures().isEmpty())
			allColumnNames.add("Measure");
			
		List<String> drillColumns = model.getDimNames(ColumnRole.DRILL);
		if(drillColumns.size()>0){
			addCubeExtract("E_Drillthrough", "C_Olap", params.getProperty("CUBE"), "", true, false, false, false);
		}
		
		addCubeLoad("L_Cube", cubeLoadSource, "C_Olap", params.getProperty("CUBE"), allColumnNames, drillColumns, isAdd);
		addStandardJob("default",getAllComponentsForType(ITypes.Loads));
				
		Document doc = new Document();
		doc.setRootElement(project);
		return doc;
	}
	
}
