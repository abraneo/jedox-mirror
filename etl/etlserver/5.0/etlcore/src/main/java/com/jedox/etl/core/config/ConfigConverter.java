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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.config;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.Attribute;
import org.jdom.filter.ElementFilter;
import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Iterator;

import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.project.IProject.Declaration;

/**
 * Converts component configurations from older versions of this software to their actual form. Most changes in configuration markup definition can be automatically fixed by this class. However their is no guarantee that you will not need to do some work manually.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ConfigConverter {

	public static final double currentVersion = Settings.versionetl;
	//private static final Log log = LogFactory.getLog(ConfigConverter.class);
	private HashMap<String, List<?>> contexts = new HashMap<String, List<?>>();
	private HashMap<String,String[]> annexesMap = new HashMap<String, String[]>();
	private static final Log log = LogFactory.getLog(ConfigConverter.class);
	private static enum Actors {
		sources, pipelines, exports, contexts, connection, source, pipeline, transformer, export, job, query, load, function, transform, extract, dimension, dimensions
	}
	private enum Connections {
		palo, file, postgresql, mysql, sapdb, db2, oracle, sqlserver, odbc, derby, ldap
	}
	private enum Functions {
		Date2Dim, DateFormat, Aggregation, Concatenation, Java, JavaScript, Groovy, Map, Replace, Lookup, SubString, UpperLower, NumberFormat
	}

	private void surround(Element parent, List<?> list, String name) {
		//create a local copy to prevent consuming of elements on detach.
		ArrayList<Element> elements = new ArrayList<Element>();
		for (int i=0; i<list.size(); i++) {
			elements.add((Element)list.get(i));
		}
		Element element = new Element(name);
		for (Element e : elements) {
			e.detach();
			element.addContent(e);
		}
		if ((elements.size() > 0) && (parent != null)) {
			parent.addContent(0,element);
		}
	}

	private void substituteName(List<?> list, String newName) {
		for (int i=0; i<list.size(); i++) {
			Element element = (Element) list.get(i);
			element.setName(newName);
		}
	}

	private void setAttributeLowerCase(Attribute attribute) {
		if (attribute != null) {
			attribute.setValue(attribute.getValue().toLowerCase());
		}
	}

	private void setAttribute(Element element, String attributeName, Element child, String childAttributeName) {
		//only set if not already exists and child exists
		if ((element.getAttribute(attributeName) == null) && (child != null)) {
			String value = child.getAttributeValue(childAttributeName);
			if (value != null)
				element.setAttribute(attributeName, value);
		}
	}

	private List<?> setAttribute(List<?> list, String attributeName, String childName, String childAttributeName) {
		for (int i=0; i<list.size(); i++) {
			Element element = (Element) list.get(i);
			Element child = element.getChild(childName);
			if (child != null)
				setAttribute(element, attributeName, child, childAttributeName);
		}
		return list;
	}

	private List<Element> convertParameter(Element element) {
		List<?> list = element.getChildren("parameter");
		ArrayList<Element> result = new ArrayList<Element>();
		//this operation consumes the list!
		while (list.size() > 0) {
			Element e = (Element) list.get(0);
			String name = e.getAttributeValue("name");
			e.detach();
			if (name != null) {
				Element newElement = new Element(name);
				newElement.addContent(e.getTextTrim());
				result.add(newElement);
			}
		}
		if (result.size() > 0) {
			element.addContent(result);
		}
		return result;
	}
	
	private void fixFilterRange (Element dimensionElement) {
		if (dimensionElement!=null) {
			for (int j=0; j<dimensionElement.getChildren().size(); j++) {
				Element filterElement = (Element) dimensionElement.getChildren().get(j);
				if (filterElement.getAttributeValue("operator").equals("between"))
					filterElement.setAttribute("operator", "inRange");
				if (filterElement.getAttributeValue("operator").equals("alpha_range"))
					filterElement.setAttribute("operator", "inAlphaRange");										
			}					
		}	
	}
	
	private void fixFilterRangeSAP (Element dimensionElement) {
		if (dimensionElement!=null) {
			for (int j=0; j<dimensionElement.getChildren().size(); j++) {
				Element filterElement = (Element) dimensionElement.getChildren().get(j);
				if (filterElement.getAttributeValue("operator").equals("like"))
					filterElement.setAttribute("operator", "cp");
			}					
		}	
	}
	
	private boolean fixContext(Element element) {
		element.setName("variables");
		substituteName(element.getChildren("parameter"),"variable");
		List<?> cl = element.getChildren("context");
		for (int i=0; i<cl.size(); i++) {
			Element c = (Element) cl.get(i);
			String cn = c.getAttributeValue("name");
			substituteName(c.getChildren("parameter"),"variable");
			List<?> vl = c.getChildren("variable");
			contexts.put(cn, vl);
		}
		return true;
	}

	private boolean fixConnection(Element connection) {

		if (connection.getAttribute("nameref") == null) { //ignore references
			if ("file".equals(connection.getAttributeValue("type")))
				convertParameter(connection);
			else
				surround(connection,connection.getChildren("parameter"),"parameters");
		}
		//Connection type refactoring
		String conntype = connection.getAttribute("type").getValue();
		if (conntype.equals("oracle:thin"))
			connection.setAttribute("type", "Oracle");
		if (conntype.equals("hsqldb:file"))
			connection.setAttribute("type", "HsqldbFile");
		if (conntype.equals("hsqldb:hsql"))
			connection.setAttribute("type", "HsqldbHsql");
		if (conntype.equals("sap"))
			connection.setAttribute("type", "SAP");
		if (conntype.equals("Sapdb"))
			connection.setAttribute("type", "Maxdb");
		if (conntype.equals("Palo"))
			connection.setAttribute("type", "Jedox");
		if (conntype.equals("PaloGlobal"))
			connection.setAttribute("type", "JedoxGlobal");
		//First letter in Connection type is upper case
		Connections c = Connections.valueOf(conntype);
        if (c != null) {
			connection.setAttribute("type", conntype.substring(0,1).toUpperCase()+conntype.substring(1));
        }
		return true;
	}

	private boolean fixSource(Element source) {
		//set source format to lower case
		setAttributeLowerCase(source.getAttribute("format"));
		if (source.getAttribute("nameref") == null) { //ignore references
			//Tree Source
			surround(source,source.getChildren("constant"),"constants");
			//Join Source
			surround(source,source.getChildren("key"),"keys");
			convertParameter(source);
			source.setName(ITypes.Components.extract.toString());
			//Source type refactoring
			String sourcetype = source.getAttribute("type").getValue();
			if (sourcetype.equals("OLAPCube"))
				source.setAttribute("type", "Cube");
			if (sourcetype.equals("OLAPDimension"))
				source.setAttribute("type", "Dimension");
			if (sourcetype.equals("LDAP"))
				source.setAttribute("type", "Ldap");
			if (sourcetype.equals("SQL")) {
			    String connSource = source.getChild("connection").getAttributeValue("nameref");
			    // Lookup the type of the corresponding connection
			    @SuppressWarnings("unchecked")
				List<Element> connList = source.getParentElement().getParentElement().getChild("connections").getChildren("connection");
				if (connSource != null && connList != null)
					for(Element c:connList){
						if(c.getAttributeValue("name").equals(connSource)) {
							if (c.getAttributeValue("type").equals("file"))
								source.setAttribute("type", "File");
							else
								source.setAttribute("type", "Relational");
							break;
					}
				}
			}
			if (sourcetype.equals("Cube")) {
			    Element query = source.getChild("query");
			    String celltype = query.getAttributeValue("celltype");
			    if (celltype != null) {
			    	if (celltype.equals("numeric"))
			    		query.setAttribute("celltype", "only_numeric");
			    	if (celltype.equals("text"))
			    		query.setAttribute("celltype", "only_string");			    				    	
			    }
			    if (query.getChild("dimensions")!=null) {
				    @SuppressWarnings("unchecked")
			    	List<Element> filterList = query.getChild("dimensions").getChildren("dimension"); 
		    		for (Element filter : filterList) {
		    			fixFilterRange(filter);
		    		}
			    }
			}				
			if (sourcetype.equals("Dimension")) {
			    Element query = source.getChild("query");
			    fixFilterRange(query.getChild("dimension"));				
			}

			if (sourcetype.equals("SAPBIDataStore") || sourcetype.equals("SAPBIMaster")) {
			    Element query = source.getChild("query");
			    if (query.getChild("filters")!=null) {
				    @SuppressWarnings("unchecked")			    	
			    	List<Element> filterList = query.getChild("filters").getChildren("filter");
		    		for (Element filter : filterList) {
		    			fixFilterRangeSAP(filter);
		    		}
			    }	
			}
	
			if (sourcetype.equals("SAPBIHierarchy") || sourcetype.equals("SAPBIMaster")) {
			    Element dimension = source.getChild("query").getChild("dimension");
				if(dimension.getAttribute("format")!= null) {
					if (dimension.getAttributeValue("format").equals("ext"))
						dimension.setAttribute("format", "true"); 
					if (dimension.getAttributeValue("format").equals("int"))
						dimension.setAttribute("format", "false"); 
				}		
			}
			
			if (sourcetype.equals("SAPBICube")) {
			    Element cube = source.getChild("query").getChild("cube");
				if(cube.getAttribute("genericnames")!= null) {
					cube.removeAttribute("genericnames"); 
				}		
			}

		}
		return false;
	}

	@SuppressWarnings(value = "unchecked")
	private List<?> fixLevels(List<?> levels) {
		for (int i=0; i<levels.size(); i++) {
			Element level = (Element) levels.get(i);
			ArrayList<Element> attributes = new ArrayList<Element>();
			attributes.addAll(level.getChildren("alias"));
			attributes.addAll(level.getChildren("attribute"));
			surround(level,attributes,"attributes");
		}
		return levels;
	}

	private boolean fixPipeline(Element pipeline) {
		//rename to functions
		Element functions = pipeline.getChild("transformers");
		if (functions != null) {
			functions.setName("functions");
		}
		//fix target
		Element target = pipeline.getChild("target");
		if (target != null) {
			//fix pipeline name, type and format problem
			setAttribute(pipeline,"name",target,"name");
			setAttribute(pipeline,"format",target,"format");
			setAttributeLowerCase(pipeline.getAttribute("format"));
			//setAttribute(pipeline,"type",target,"type");
			//substitute old names with new ones
			substituteName(target.getChildren("column"),"coordinate");
			substituteName(target.getChildren("data"),"annex");
			Element parent = target;
			//add surrounding elements
			surround(parent,setAttribute(target.getChildren("constant"),"name","input","nameref"),"constants");
			if(!pipeline.getAttributeValue("type").equals("TreeLE"))
				surround(parent,setAttribute(fixLevels(target.getChildren("level")),"name","input","nameref"),"levels");
			surround(parent,setAttribute(target.getChildren("annex"),"name","input","nameref"),"annexes");
			// added for 4.0
			@SuppressWarnings("unchecked")
			List<Element> annexes= target.getChild("annexes").getChildren();
			ArrayList<Element> annexesElements = new ArrayList<Element>();
			String[] annexesNames = new String[annexes.size()];
			for (int i=0; i<annexes.size(); i++) {
				try{
					Element a = (Element)annexes.get(i);
					String outputName = "constant";
					if(a.getChild("input").getAttribute("nameref")!= null)
						outputName = a.getChild("input").getAttributeValue("nameref");
					if(a.getAttribute("name")!=null)
						outputName = a.getAttributeValue("name");
					annexesNames[i]=outputName;
				}catch(Exception e){}
				annexesElements.add((Element)annexes.get(i));
			}
			this.annexesMap.put(pipeline.getAttributeValue("name"), annexesNames);
			@SuppressWarnings("unchecked")
			List<Element> coordinates = target.getChild("coordinates").getChildren();
			
			for(Element e:annexesElements){
				e.setName("coordinate");
				e.detach();
				coordinates.add(e);
			}
			target.removeChild("annexes");
			//end of 4.0
			Element measures = parent.getChild("measures");
			if (measures != null) {
				surround(measures,measures.getChildren("input"),"measure");
				setAttribute(measures.getChildren("measure"),"name","input","nameref");
				setAttribute(measures,"normalize",measures,"name");
				measures.removeAttribute("name");

				@SuppressWarnings("unchecked")
				List<Element> measuresList = measures.getChildren();
				if(measures.getAttribute("aggregate")!= null){
					String defaultAggregate  = measures.getAttribute("aggregate").getValue();
					for(Element m:measuresList){
						if(m.getAttribute("aggregate")== null){
							m.setAttribute("aggregate", defaultAggregate);
						}
					}
				}
				measures.removeAttribute("aggregate");
				if(measures.getAttribute("type")!= null){
					String defaultType  = measures.getAttribute("type").getValue();
					for(Element m:measuresList){
						if(m.getAttribute("type")== null){
							m.setAttribute("type", defaultType);
						}
					}
				}
				measures.removeAttribute("type");

			}
			surround(parent,setAttribute(target.getChildren("coordinate"),"name","input","nameref"),"coordinates");

			target.removeAttribute("name");
			//target.removeAttribute("type");
			target.removeAttribute("format");

		}

		//fix the filtering in TableView
		if(pipeline.getAttributeValue("type").equals("TableView")){
			Element input = pipeline.getChild("filter").getChild("input");
			ArrayList<Element> newFilters = new ArrayList<Element>();
			for(Object filter: input.getChildren()){
				Element newFilter = fixFilter((Element)filter,"retrict",true);
				newFilter.removeAttribute("mode");
				newFilters.add(newFilter);

			}
			input.removeContent();
			input.setContent(newFilters);

			// map the filter operators for ranges			
			Element query = pipeline.getChild("filter");
		    @SuppressWarnings("unchecked")
	    	List<Element> filterList = query.getChildren("input"); 
    		for (Element filter : filterList) {
    			fixFilterRange(filter);
    		}    		
		}
		
		//join type full not supported		
		if(pipeline.getAttributeValue("type").equals("TableJoin")){
			Element joins = pipeline.getChild("joins");
			if (joins != null) {
				@SuppressWarnings("unchecked")
				List<Element> joinList = joins.getChildren();
				for(Element join: joinList ){
					if(join.getAttributeValue("type").equals("full")) {
						join.setAttribute("type","inner");
					}
					/*
					//add new condition element (4.x): postponed
					if (join.getChild("condition") == null) {
						join.setAttribute("left", join.getChild("left").getAttributeValue("nameref"));
						join.setAttribute("right", join.getChild("right").getAttributeValue("nameref"));
						List<?> leftkeys = join.getChild("left").getChildren();
						List<?> rightkeys = join.getChild("right").getChildren();
						join.removeContent();
						for (int i=0; i<leftkeys.size(); i++) {
							Element condition = new Element("condition");
							join.addContent(condition);
							Element leftkey = (Element) leftkeys.get(i);
							Element rightkey = (Element) rightkeys.get(i);
							Element left = new Element("left");
							Element right = new Element("right");
							left.setAttribute("nameref", leftkey.getAttributeValue("nameref"));
							right.setAttribute("nameref", rightkey.getAttributeValue("nameref"));
							condition.addContent(left);
							condition.addContent(right);
						}
					}
					*/
				}
				// add migration for join type right outer to left outer here
			}
		}				
		
		pipeline.setName(ITypes.Components.transform.toString());
		return false;
	}

	private boolean fixTransformer(Element transformer) {
		transformer.setName("function");
		surround(transformer,transformer.getChildren("input"),"inputs");
		surround(transformer,transformer.getChildren("parameter"),"parameters");
		// Convert parameters to tags for all delivered functions (not user specific functions)
		Functions f = Functions.valueOf(transformer.getAttribute("type").getValue());
        if (f != null) {
    		convertParameter(transformer.getChild("parameters"));
        }
        if(transformer.getAttributeValue("type").equals("Date2Dim")){
        	transformer.setAttribute("type", "DateFormat");
        	if(transformer.getChild("parameters").getChild("sourceformat") == null){
        		transformer.getChild("parameters").addContent(new Element("sourceformat").setText("yyyy-MM-dd"));
        	}
        	if(transformer.getChild("parameters").getChild("targetformat")!= null){
        		String targetFormat = transformer.getChild("parameters").getChild("targetformat").getText();
        		targetFormat = targetFormat.replace("Y","yyyy");
        		targetFormat = targetFormat.replace("D", "dd");
        		transformer.getChild("parameters").getChild("targetformat").setText(targetFormat);
        	}
        }
		return true;
	}

	private boolean fixExport(Element export) {
		//fix export naming problem
		setAttribute(export,"name",export.getChild("source"),"nameref");
		//fix mode
		Attribute mode = export.getAttribute("mode");
		if (mode != null) {
			setAttributeLowerCase(export.getAttribute("mode"));
			//convert to element
			Element m = new Element("mode");
			m.setText(mode.getValue());
			export.addContent(m);
			export.removeAttribute(mode);
		}
		convertParameter(export);
		export.setName(ITypes.Components.load.toString());
		if(export.getChild("cube") != null && export.getChild("cube").getChild("drillthrough")!= null){
			String sourceName = export.getChild("source").getAttributeValue("nameref");
			if(this.annexesMap.containsKey(sourceName)){
				// Assume that no Dimensions-tag exists in this case
				Element dimensions = new Element("dimensions");
				String[] annexColumns = this.annexesMap.get(sourceName);
				for(String s:annexColumns){
					Element newDimension = new Element("dimension");
					newDimension.setAttribute("input", s);
					dimensions.addContent(newDimension);
				}				
				export.getChild("cube").addContent(dimensions);
			}
		}
		if(export.getAttributeValue("type").equals("Cube")) {
			Element def = export.getChild("default");
			if (def!=null) {
				if (def.getAttributeValue("type").equals("defaultBase"))
					def.setAttribute("type", "mapToDefault");
				if (def.getAttributeValue("type").equals("defaultConsolidate"))
					def.setAttribute("type", "createUnderDefault");
			}
		}	
		
		return true;
	}

	private boolean fixJob(Element job) {
		//fix default job naming problem
		if (job.getAttributeValue("name") == null) {
			job.setAttribute("name", "default");
		}
		//subsitute context with its parameter
		Element context = job.getChild("context");
		if (context != null) {
			String name = context.getAttributeValue("nameref");
			List<?> variables = contexts.get(name);
			if (variables != null)
				job.addContent(variables);
			job.removeContent(context);
		}
		//change export anchestors to load
		Iterator<?> i = job.getDescendants(new ElementFilter("export"));
		while (i.hasNext()) {
			Element e = (Element) i.next();
			e.setName(ITypes.Components.load.toString());
		}
		// convert Script jobs to Groovy jobs
        if( job.getAttributeValue("type")!=null && job.getAttributeValue("type").equals("Script"))
        	job.setAttribute("type", "Groovy");		
		return true;
	}

	private boolean fixQuery(Element query) {
		List<?> dimensions = query.getChildren("dimension");
		if (dimensions.size() > 1)
			surround(query,query.getChildren("dimension"),"dimensions");
		return false;
	}

	private Element fixFilter(Element filter, String defaultLeafMode, boolean isCubeExtractDimension) {

		// if the filter has parent att. restriction and another restriction (example value) ,
		// the second restriction will dominate
		boolean onlyParent = true;
		if(filter.getAttribute("range") != null){
			filter.setAttribute("operator", "alpha_range" );
			filter.setAttribute("value", filter.getAttribute("range").getValue());
			filter.removeAttribute(filter.getAttribute("range"));
			onlyParent = false;
		}else if(filter.getAttribute("between") != null){
			filter.setAttribute("operator", "between" );
			filter.setAttribute("value", filter.getAttribute("between").getValue());
			filter.removeAttribute(filter.getAttribute("between"));
			onlyParent = false;
		}else if(filter.getAttribute("value") != null && filter.getAttribute("operator") == null){
			filter.setAttribute("operator", "equal" );
			onlyParent = false;
		}else if(filter.getAttribute("element") != null){
			filter.setAttribute("operator", "like" );
			filter.setAttribute("value", filter.getAttribute("element").getValue());
			filter.removeAttribute(filter.getAttribute("element"));
			onlyParent = false;
		}else if(filter.getAttribute("parent") != null && onlyParent == true){//it only has a parent restriction
			filter.setAttribute("operator", "like" );
			filter.setAttribute("value", filter.getAttribute("parent").getValue());
			filter.removeAttribute(filter.getAttribute("parent"));
		}
		//if it the parent was not deleted yet, because it existed with a higher priority attribute
		if(filter.getAttribute("parent") != null ){
			filter.removeAttribute(filter.getAttribute("parent"));
		}

		if(defaultLeafMode.equals("root")){
			filter.setAttribute("mode", "onlyRoots");
		}
		else{
			//case is a dimension filter
			if(filter.getAttribute("mode")== null){
				if(!isCubeExtractDimension){
					filter.setAttribute("mode", "nodesToBases");
				}
				else{//case is a cube filter
					filter.setAttribute("mode", "onlyBases");
				}
			}
		}
		return filter;
	}

	private boolean fixSources(Element sources) {
		if (sources.getParentElement().getName().equalsIgnoreCase("project"))
				sources.setName(ITypes.Extracts);
		return false;
	}

	private boolean fixExports(Element exports) {
		exports.setName(ITypes.Loads);
		return false;
	}

	private boolean fixPipelines(Element pipelines) {
		pipelines.setName(ITypes.Transforms);
		return false;
	}

	private boolean fixDimension(Element dimension) {

		boolean isCubeExtractDimension;
		//check first if it is a dimension extract or a part of a cube extract
		//no conversion for extract SAPBIHierarchy
		Attribute Extracttype = dimension.getParentElement().getParentElement().getAttribute("type");
		if (Extracttype!=null && Extracttype.getValue().equals("Dimension")) {
			isCubeExtractDimension = false;
		}
		else {
			Extracttype = dimension.getParentElement().getParentElement().getParentElement().getAttribute("type");
			if (Extracttype!=null && Extracttype.getValue().equals("Cube")) {
				isCubeExtractDimension = true;
			}
			else return true;
		}

		// it has filters that should be migrated
		// accept all or deny all will be ignored (in most cases it is not needed in the new form of filter)
		String dimensionDefaultLeafMode = "";
		if(dimension.getAttribute("leafs")!= null && dimension.getAttribute("leafs").getValue().equals("root"))
			dimensionDefaultLeafMode = "root";
		else
			dimensionDefaultLeafMode = "restrict";
		dimension.removeAttribute(dimension.getAttribute("leafs"));

		// if it does have a leaf modus and it does not have any children
		//anything other than root will be treated as restrict
		if(dimension.getChildren().size() == 0){
			if(dimensionDefaultLeafMode.equals("root")){
				Element e = new Element("accept");
				e.setAttribute("operator","like");
				e.setAttribute("value", ".");
				e.setAttribute("mode", "onlyRoots");
				dimension.setContent(e);
			}
			return true;

		}
		else{

			ArrayList<Element> newChildren = new ArrayList<Element>();
			for(Object e : dimension.getChildren()){
				if (((Element)e).getAttributes().size() != 0){
					Element copy = (Element)(((Element)e).clone());
					newChildren.add(fixFilter(copy,dimensionDefaultLeafMode,isCubeExtractDimension));
				}
			}
			dimension.removeContent();
			dimension.setContent(newChildren);
		}
		return true;
	}

	private boolean fixDimensions(Element dimension) {
		return false;
	}

	private boolean convertElement(Element element) {
		try {
			Actors c = Actors.valueOf(element.getName());
			switch (c) {
			case sources: return fixSources(element);
			case exports: return fixExports(element);
			case pipelines: return fixPipelines(element);
			case contexts: return fixContext(element);
			case connection: return fixConnection(element);
			case source: return fixSource(element);
			case extract: return fixSource(element);
			case pipeline: return fixPipeline(element);
			case transform: return fixPipeline(element);
			case transformer: return fixTransformer(element);
			case function: return fixTransformer(element);
			case export: return fixExport(element);
			case load: return fixExport(element);
			case job: return fixJob(element);
			case dimension: return fixDimension(element);
			case dimensions: return fixDimensions(element);
			case query: return fixQuery(element);
			default: return false;
			}
		}
		catch (Exception e) {
			return false;
		}
	}

	@SuppressWarnings("unchecked")
	private void walkTree(Element element) {
		if (!convertElement(element)) {
			List<Element> l = element.getChildren();
			for (Element e : l) {
				walkTree(e);
			}
		}
	}

	private double getVersion(String version) {
		try {
			return Double.parseDouble(version);
		}
		catch (Exception e) {
			return 1.0;
		}
	}

	/**
	 * converts xml configurations of components to the current format. if the version is equal to the current version and the format is strict, conversion is skipped.
	 * @param element the xml configuration of the component to convert
	 * @param version the version of the xml configuration, if available.
	 * @param format the format of the xml configuration.
	 * @return the converted xml in the actual processable format
	 */
	public synchronized Element convert(Element element, String version, Declaration format) {
		if (!Declaration.strict.equals(format) || (getVersion(version) < currentVersion) ) {
			walkTree(element);
			//set actual version if element is a project
			if (element.getName().startsWith("project")) {
				element.setAttribute("version", String.valueOf(currentVersion));
				element.setAttribute("format", "strict");
			}
		}
		return element;
	}

	/**
	 * converts an entire project document to the current format if needed.
	 * @param document the project definition
	 * @return the converted document in the actual processable format
	 */
	public synchronized Document convert(Document document) {
		Element root = document.getRootElement();
		String version = root.getAttributeValue("version","1.0");
		Declaration format = Declaration.valueOf(root.getAttributeValue("format",Declaration.lazy.toString()).toLowerCase());
		if ((getVersion(version) < currentVersion) || !format.equals(Declaration.strict)) {
			walkTree(root);
			root.setAttribute("version", String.valueOf(currentVersion));
			root.setAttribute("format", Declaration.strict.toString());
			log.info("Project " + root.getAttributeValue("name") + " is successfully migrated.");
		}
		return document;
	}

}
