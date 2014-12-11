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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util.svg;

import java.awt.Color;
import java.awt.Font;
import java.awt.geom.Rectangle2D;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileReader;
import java.io.OutputStream;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.List;
import java.util.Properties;
import java.util.Set;
import javax.swing.SwingConstants;
import org.jdom.Element;
import org.jgraph.JGraph;
import org.jgraph.graph.DefaultCellViewFactory;
import org.jgraph.graph.DefaultEdge;
import org.jgraph.graph.DefaultGraphCell;
import org.jgraph.graph.DefaultGraphModel;
import org.jgraph.graph.GraphCell;
import org.jgraph.graph.GraphConstants;
import org.jgraph.graph.GraphLayoutCache;
import org.jgraph.graph.GraphModel;
import org.jgraph.graph.VertexRenderer;
import org.jgraph.graph.VertexView;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.util.XMLUtil;
import com.jgraph.io.svg.SVGGraphConstants;
import com.jgraph.io.svg.SVGVertexRenderer;
import com.jgraph.layout.JGraphFacade;
import com.jgraph.layout.JGraphLayout;
import com.jgraph.layout.hierarchical.JGraphHierarchicalLayout;
import com.jgraph.layout.tree.JGraphTreeLayout;


public class GraphUtil implements IGraphUtil{
	
	public enum ViewTypes {
		dependencies, dependants, all
	}
	
	private JGraph graph;
	private Properties properties;
	private IProject project;
	private String componentName;
	
	@SuppressWarnings("unchecked")
	public GraphUtil(IProject project, String componentName, Properties graphProperties, List<Locator> invalidComponents) throws ConfigurationException {
		this.project = project;
		this.componentName=componentName;			
		this.properties = getDefaultProperties();
		properties.putAll(graphProperties);
	
		ViewTypes type = parseViewType(properties.getProperty("viewType"));
		
		GraphModel model = new DefaultGraphModel();
		GraphLayoutCache view = new GraphLayoutCache(model,new DefaultCellViewFactory());
		graph = new JGraph(model,view); 
		
		Set<String> rootComponents = new HashSet<String>();
		Map<String, DefaultGraphCell> cellMap = new HashMap<String,DefaultGraphCell>();
		
		// if this is a project
		if(Locator.parse(componentName).isRoot()){
			buildListsView(project,cellMap);
			return;
		}
		
		switch (type) {
		case all: {
			if (properties.getProperty("layout").equalsIgnoreCase("tree")) {
				handleRelations(getComponentDependencies(), false, cellMap);
				handleRelations(getComponentDependents(), true, cellMap);
				rootComponents.add(componentName);
			}
			else {//hierarchy layout
				Map<String,List<String>> dependants = getComponentDependents();
				Map<String,List<String>> dependancies = getComponentDependencies();	
				handleRelations(dependants, true, cellMap);						
				handleRelations(dependancies, false, cellMap);			
				rootComponents = getTerminalDependants(componentName, dependants, rootComponents);				
			}
			break;
		}
		case dependencies : {
			// for connection only consider the dependencies to loads 
			// get Graph for dependencies
			handleRelations(getComponentDependencies(), false, cellMap);
			rootComponents.add(componentName);
			break;
		}
		case dependants : {
			// get Graph for dependants
			Map<String,List<String>> dependants = getComponentDependents();
			handleRelations(dependants, true, cellMap);
			if (properties.getProperty("layout").equalsIgnoreCase("tree")) {
				rootComponents.add(componentName);
			}	
			else {//hierarchy layout
				rootComponents = getTerminalDependants(componentName, dependants, rootComponents);				
			}
			break;
		}
		default: break;
		}
		
		String invalidRoot = null;
		int minY = 0;
		for (int i=0;i<invalidComponents.size();i++) {
			
			addCell(invalidComponents.get(i).toString(),cellMap).getAttributes().put("componentState", "invalid");
			DefaultGraphCell invalidCell  = cellMap.get(invalidComponents.get(i).toString());
			GraphConstants.setLineWidth(invalidCell.getAttributes(), parseFloat(properties.getProperty("invalidLineWidth"),"invalidLineWidth"));
			Rectangle2D map = GraphConstants.getBounds(invalidCell.getAttributes());
			map.setRect(map.getMinX(), minY, map.getWidth(), 40);
			minY+=50;
			if(i==0){
				invalidRoot = invalidComponents.get(0).toString();
			}
			
			properties.setProperty("etlProjectState", "invalid");
		}
		
		//do Layout
		LinkedList<Object> rootCells = new LinkedList<Object>();
		if(invalidRoot!=null){
			rootCells.add(cellMap.get(invalidRoot));
			//verticalSeparatorLine(cellMap.get(invalidRoot).getAttributes().get(""));
		}
		
		for (String s : rootComponents) {
			rootCells.add(cellMap.get(s));
		}

		
		Object[] roots = rootCells.toArray();
		JGraphFacade facade = new JGraphFacade(graph,roots,true,false,false,true); // Pass the facade the JGraph instance
		JGraphLayout layout;
		if (properties.getProperty("layout").equalsIgnoreCase("tree")) {
			JGraphTreeLayout treelayout = new JGraphTreeLayout();
			treelayout.setOrientation(parseInt(properties.getProperty("orientation")));
			treelayout.setCombineLevelNodes(true);
			layout = treelayout;
		}
		else { //hierarchy
			JGraphHierarchicalLayout hlayout = new JGraphHierarchicalLayout();
			hlayout.setOrientation(parseInt(properties.getProperty("orientation")));
			hlayout.setDeterministic(true);
			layout = hlayout;
		}
		layout.run(facade); // Run the layout on the facade.
		Map<?,?> nested = facade.createNestedMap(true, true); // Obtain a map of the resulting attribute changes from the facade
		graph.getGraphLayoutCache().edit(nested); // Apply the results to the actual graph
		
		if (graph.getGraphLayoutCache().getAllViews().length == 0) {
			throw new RuntimeException("Empty component graph");
		}
	}
				
	private void buildListsView(IProject project,
			Map<String, DefaultGraphCell> cellMap) {
		
		/*addProjectCell(project.getName(),cellMap);
		DefaultGraphCell projectcell  = cellMap.get(project.getName());
		GraphConstants.setLineWidth(projectcell.getAttributes(), Float.parseFloat("1.0"));
		Rectangle2D map = GraphConstants.getBounds(projectcell.getAttributes());
		map.setRect(0, 0, map.getWidth(), 40);*/
			
		int minX = 0;
		int minY = 50;
		double maxXInType = 0;
		
		String lastType = null;
			for (IComponent comp:project.getModelComponents()) {
			
				addCell(comp.getLocator().toString(),cellMap);
				DefaultGraphCell cell  = cellMap.get(comp.getLocator().toString());
				
				if(!comp.getLocator().getType().equals(lastType) && lastType!=null){
					minX=(int)(maxXInType+20);
					//addEdge(projectcell,cell,true,true,false);
					minY = 50;
				}
								
				lastType = comp.getLocator().getType();
				GraphConstants.setLineWidth(cell.getAttributes(), Float.parseFloat("1.0"));
				Rectangle2D map = GraphConstants.getBounds(cell.getAttributes());
				map.setRect(minX, minY, map.getWidth(), 40);
				minY+=50;
				maxXInType = Math.max(maxXInType, minX+map.getWidth());	
			}
			
			// check ticket 15759
			// somehow the last component keep showing up in the wrong position otherwise
			 addEmptyVertix(maxXInType+20,minY);
	}

	/**
	 * @param minY
	 */
	@SuppressWarnings("unchecked")
	private void addEmptyVertix(double x,double y) {
		DefaultGraphCell emptycell = createVertex("empty", "empty", x, y, 0, 0, Color.BLACK, Color.BLACK);
		 emptycell.getAttributes().put("componentName","");
		 emptycell.getAttributes().put("componentType","");
		 emptycell.getAttributes().put("componentTypeLabel","");
		 emptycell.getAttributes().put("componentImplementation", "");
		 graph.getGraphLayoutCache().insert(emptycell);
	}

	private boolean hasType(String component, String type) {
		return Locator.parse(component).getType().equals(type);
	}
			
	private void modifyDependencyMap (Map<String,List<String>> map, String fromComp, String toComp) {
		class DepChange {
			String key;
			String value;	
			
			DepChange(String from, String to) {
				this.key=from;
				this.value=to;
			}
		}

		// Switch dependency between Loads and Connections nodes
		if (!hasType(componentName,fromComp) && !hasType(componentName,toComp)) {
			HashSet<DepChange> changeSet = new HashSet<DepChange>();			
			for (Map.Entry<String, List<String>> entry : map.entrySet())
				if (hasType(entry.getKey(),fromComp))
					for (String value : entry.getValue())
			 			if (hasType(value,toComp))
			 				changeSet.add(new DepChange(entry.getKey(),value));
			for (DepChange vp : changeSet)
				map.get(vp.key).remove(vp.value);
						
		}
	}
	
		
	private Map<String,List<String>> getComponentDependencies() {		
		Map<String,List<String>> map = project.getAllDependencies(componentName,false);
		modifyDependencyMap(map, "load","connection");
		return map;
	}

	private Map<String,List<String>> getComponentDependents() {
		Map<String,List<String>> map = project.getAllDependents(componentName,false);		
		modifyDependencyMap(map, "connection","load");
		return map;
	}
	
	
	private Set<String> getTerminalDependants(String componentName, Map<String,List<String>> map, Set<String> terminals) {
		List<String> list = map.get(componentName);
		if (list != null && !list.isEmpty()) {
			for (String s : list) {
				getTerminalDependants(s, map, terminals);
			}
		}
		else {
			terminals.add(componentName);
		}
		return terminals;
	}
	
	private int parseInt(String value) {
		try {
			return Integer.parseInt(value);
		}
		catch (Exception e) {
			return 0;
		}
	}

	private ViewTypes parseViewType(String name) throws ConfigurationException {
		ViewTypes type;
		try { 
			type = ViewTypes.valueOf(name);
		}
		catch (Exception e) {
			throw new ConfigurationException("Invalid View Type "+name);
		}	
		return type;
	}
	
	private float parseFloat(String number, String name) throws RuntimeException {
		try {
			return Float.valueOf(number);
		}
		catch (Exception e) {
			throw new RuntimeException("Parameter "+name+" with value "+number+" must be a number!");
		}
	}
	
	private DefaultGraphCell createVertex(String qname, String name, double x, double y, double w, double h, Color bg, Color linecolor) throws RuntimeException {
		// Create vertex with the given name
		DefaultGraphCell cell = new DefaultGraphCell(name);

		// Set bounds
		GraphConstants.setBounds(cell.getAttributes(), new Rectangle2D.Double(x, y, w, h));
		GraphConstants.setBackground(cell.getAttributes(), bg);
		GraphConstants.setOpaque(cell.getAttributes(), true);	
		GraphConstants.setBorderColor(cell.getAttributes(), linecolor);
		if (qname.equals(componentName))
			// Highlight the focues component
			GraphConstants.setLineWidth(cell.getAttributes(), parseFloat(properties.getProperty("lineWidthFocus"),"lineWidthFocus"));
		else	
			GraphConstants.setLineWidth(cell.getAttributes(), parseFloat(properties.getProperty("lineWidth"),"lineWidth"));
		SVGGraphConstants.setShape(cell.getAttributes(), SVGGraphConstants.SHAPE_ROUNDRECT);

		return cell;
	}
	
	/*private void verticalSeparatorLine(Object width){
		
		
		// Create vertex with the given name
		DefaultGraphCell cell = new DefaultGraphCell("separator");

		// Set bounds
		int cellPadding = parseInt(properties.getProperty("cellPadding"));
		GraphConstants.setBounds(cell.getAttributes(), new Rectangle2D.Double(130, 10, 1, 1000));
		AttributeMap atts = cell.getAttributes();
		atts.put("componentTypeLabel", "");
		atts.put("componentName", "");
		cell.setAttributes(atts);

		graph.getGraphLayoutCache().insert(cell);

	}*/
	 	
	@SuppressWarnings("unchecked")
	private GraphCell addCell(String qname,Map<String, DefaultGraphCell> addedComponents) throws RuntimeException {
		try {
			Locator l = Locator.parse(qname);
			String componentType = l.getType();
			String componentImplementation = ConfigManager.getInstance().get(l).getAttributeValue("type","");
			String componentName = l.getName();
			String componentTypeLabel = componentType.substring(0, 1).toUpperCase()+componentType.substring(1)+": "+componentImplementation;
			Color bgcolor = Color.decode(properties.getProperty(componentType+"Background"));
			Color linecolor = Color.decode(properties.getProperty(componentType+"Border"));
			
			Font typeFont = getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeType"));
			Font labelFont = getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeName"));
			int typeW = graph.getFontMetrics(typeFont).stringWidth(componentTypeLabel);
			int nameW = graph.getFontMetrics(labelFont).stringWidth(componentName);
			int cellPadding = parseInt(properties.getProperty("cellPadding"));
			int width = Math.max(100, Math.max(typeW+cellPadding, nameW+cellPadding));
			
			DefaultGraphCell cell = createVertex(qname,componentName,0, addedComponents.size()*50, width, 40,bgcolor, linecolor);
			GraphConstants.setForeground(cell.getAttributes(), Color.decode(properties.getProperty(componentType+"Foreground")));
			GraphConstants.setLink(cell.getAttributes(), properties.getProperty("linkPrefix")+l.toString());
			cell.getAttributes().put("componentName", componentName);
			cell.getAttributes().put("componentType",componentType);
			cell.getAttributes().put("componentTypeLabel",componentTypeLabel);
			cell.getAttributes().put("componentImplementation", componentImplementation);
			
			graph.getGraphLayoutCache().insert(cell);
			addedComponents.put(qname, cell);
			return cell;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to add cell "+qname+" to graph: "+e.getMessage()); 
		}
	}
	
	@SuppressWarnings("unchecked")
	private GraphCell addProjectCell(String qname,Map<String, DefaultGraphCell> addedComponents) throws RuntimeException {
		try {
			Locator l = Locator.parse(qname);
			String componentType = "project";
			String componentImplementation = "project";
			String componentName = l.getName();
			String componentTypeLabel = componentType.substring(0, 1).toUpperCase()+componentType.substring(1)+": "+componentImplementation;
			Color bgcolor = Color.decode(properties.getProperty(componentType+"Background"));
			Color linecolor = Color.decode(properties.getProperty(componentType+"Border"));
			
			Font typeFont = getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeType"));
			Font labelFont = getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeName"));
			int typeW = graph.getFontMetrics(typeFont).stringWidth(componentTypeLabel);
			int nameW = graph.getFontMetrics(labelFont).stringWidth(componentName);
			int cellPadding = parseInt(properties.getProperty("cellPadding"));
			int width = Math.max(100, Math.max(typeW+cellPadding, nameW+cellPadding));
			
			DefaultGraphCell cell = createVertex(qname,componentName,0, addedComponents.size()*50, width, 40,bgcolor, linecolor);
			GraphConstants.setForeground(cell.getAttributes(), Color.decode(properties.getProperty(componentType+"Foreground")));
			//GraphConstants.setLink(cell.getAttributes(), properties.getProperty("linkPrefix")+l.toString());
			cell.getAttributes().put("componentName", componentName);
			cell.getAttributes().put("componentType",componentType);
			cell.getAttributes().put("componentTypeLabel",componentTypeLabel);
			cell.getAttributes().put("componentImplementation", componentImplementation);
			
			graph.getGraphLayoutCache().insert(cell);
			addedComponents.put(qname, cell);
			return cell;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to add cell "+qname+" to graph: "+e.getMessage()); 
		}
	}
	
	private void addEdge(DefaultGraphCell source, DefaultGraphCell target, boolean withArrow, boolean forward,boolean visible) {
		// Create Edge
		DefaultEdge edge = new DefaultEdge();
		// Fetch the ports from the new vertices, and connect them with the edge
		//	edge.setSource(source.addPort());
		//	edge.setTarget(target.addPort());
		// Set Arrow Style for edge
		int arrow = GraphConstants.ARROW_CLASSIC;
		int noarrow = GraphConstants.ARROW_NONE;
			
		if (forward && !properties.getProperty("layout").equalsIgnoreCase("tree")) {
			edge.setSource(target.addPort());
			edge.setTarget(source.addPort());
		}
		else {
			edge.setSource(source.addPort());
			edge.setTarget(target.addPort());
		}

		if (!withArrow) {			
			GraphConstants.setLineBegin(edge.getAttributes(), noarrow);
			GraphConstants.setLineEnd(edge.getAttributes(), noarrow);			
		}
		else if (forward && properties.getProperty("layout").equalsIgnoreCase("tree")) {
			GraphConstants.setLineBegin(edge.getAttributes(), noarrow);
			GraphConstants.setLineEnd(edge.getAttributes(), arrow);
		}
		else {
			GraphConstants.setLineBegin(edge.getAttributes(), arrow);
			GraphConstants.setLineEnd(edge.getAttributes(), noarrow);		
		}	
		
		GraphConstants.setEndFill(edge.getAttributes(), true);
		if(!visible)
			GraphConstants.setLineWidth(edge.getAttributes(), 0);
		
		graph.getGraphLayoutCache().insert(edge);
	}
	
	private void handleRelations(Map<String,List<String>> map, boolean forward, Map<String, DefaultGraphCell> addedComponents) {
		handleRelations(componentName, map, forward, addedComponents, new HashSet<String>());
	}
	
	
	private void handleRelations(String qname, Map<String,List<String>> map, boolean forward, Map<String, DefaultGraphCell> addedComponents, HashSet<String> componentSet) {
		List<String> list = map.get(qname);
		if (list != null && !componentSet.contains(qname)) {
			componentSet.add(qname);
			if (!addedComponents.containsKey(qname)) {
				addCell(qname,addedComponents);
			}
			for (String s : list) {
				if (!addedComponents.containsKey(s)) {
					addCell(s,addedComponents);
				}
				addEdge(addedComponents.get(qname),addedComponents.get(s),true,forward,true);
				handleRelations(s,map,forward, addedComponents, componentSet);
			}
		}
	}
	
	
	public String getSVG() {
			VertexRenderer oldRenderer = VertexView.renderer;
			ByteArrayOutputStream ba = new ByteArrayOutputStream();
			String result = "";
				try {
					OutputStream out = new BufferedOutputStream(ba);
					// Set all vertices to use the SVG renderer
					VertexView.renderer = new SVGVertexRenderer();
					ETLSVGGraphWriter writer = new ETLSVGGraphWriter(properties);
					writer.write(out, componentName, graph.getGraphLayoutCache(), 35);
					out.flush();
					out.close();
					result = ba.toString("UTF8");
				} catch (Exception e) {
			}
			VertexView.renderer = oldRenderer;
			return result;
		}

	public Element getXML() {
		String str = getSVG();
		Element element=null;
		try {
			element = XMLUtil.stringTojdom(str);
			element.detach();			
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
		return element;
	}
	
	public static Properties getDefaultProperties() {
		Properties properties = new Properties();
		try {
			String fileName = Settings.getConfigDir()+File.separator+"svg.properties";
			properties.load(new FileReader(fileName));
		}
		catch (Exception e) { //do a fallback to hardcoded defaults
			properties.setProperty("viewType", "all");
			properties.setProperty("layout", "hierarchy");
			properties.setProperty("orientation", String.valueOf(SwingConstants.NORTH));
			properties.setProperty("foreground", "#000000");
			properties.setProperty("projectBackground", "#000000");
			properties.setProperty("jobBackground", "#CC4D33");
			properties.setProperty("loadBackground", "#CC9933");
			properties.setProperty("transformBackground", "#B3CC33");
			properties.setProperty("extractBackground", "#66CC33");
			properties.setProperty("connectionBackground", "#FFFF7A");
			properties.setProperty("projectBorder", "#000000");
			properties.setProperty("jobBorder", "#000000");
			properties.setProperty("loadBorder", "#000000");
			properties.setProperty("transformBorder", "#000000");
			properties.setProperty("extractBorder", "#000000");
			properties.setProperty("connectionBorder", "#000000");
			properties.setProperty("projectForeground", "#000000");
			properties.setProperty("jobForeground", "#000000");
			properties.setProperty("loadForeground", "#000000");
			properties.setProperty("transformForeground", "#000000");
			properties.setProperty("extractForeground", "#000000");
			properties.setProperty("connectionForeground", "#000000");
			properties.setProperty("invalidBackground", "#FFFFFF");
			properties.setProperty("invalidForeground", "#000000");
			properties.setProperty("invalidBorder", "#CC2200");
			properties.setProperty("invalidText", "#CC2200");
			properties.setProperty("fontSizeName", "11");
			properties.setProperty("fontSizeType", "9");
			properties.setProperty("fontSizeTitle", "13");
			properties.setProperty("linkPrefix", "http://localhost/");
			properties.setProperty("fontFamily", "Lucida Grande");
			properties.setProperty("cellPadding", "10");
			properties.setProperty("lineWidth", "1");
			properties.setProperty("lineWidthFocus", "3");
		}
		return properties;
	}

	private static int getFontSize(String fontsize) {
		try {
			return Integer.parseInt(fontsize);
		}
		catch (Exception e) {
			return 10;
		}
	}
	
	public static Font getFont(String fontName, int style, String size) {
		return new Font(fontName,style,getFontSize(size));	
	}
		
}
