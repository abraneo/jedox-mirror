package com.jedox.etl.core.util.svg;
import java.io.File;
import java.io.FileReader;
import java.io.StringWriter;
import java.text.DateFormat;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;

import javax.swing.JFrame;
import javax.swing.SwingConstants;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.VariablesDependencyFind;
import com.jedox.etl.core.config.XMLReader;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.ExecutionDetail;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.execution.StateManager;
import com.jedox.etl.core.project.IProject;
//import com.jedox.etl.core.util.XMLUtil;
import com.mxgraph.canvas.mxICanvas;
import com.mxgraph.canvas.mxSvgCanvas;
import com.mxgraph.layout.hierarchical.mxHierarchicalLayout;
import com.mxgraph.model.mxICell;
import com.mxgraph.swing.mxGraphComponent;
import com.mxgraph.util.mxCellRenderer;
import com.mxgraph.util.mxCellRenderer.CanvasFactory;
import com.mxgraph.util.mxConstants;
import com.mxgraph.util.mxDomUtils;
import com.mxgraph.util.mxRectangle;
import com.mxgraph.util.mxUtils;
import com.mxgraph.view.mxCellState;
import com.mxgraph.view.mxGraph;
import com.mxgraph.view.mxStylesheet;

	
class GraphUtilNG {
	
	public enum ViewTypes {
		dependencies, dependants, all
	}
	
	private static final String STYLE_GROUP_INVALID = "INVALID";
	private static final String STYLE_GROUP_LABEL_NAME = "LABEL_NAME";
	private static final String STYLE_GROUP_LABEL_TYPE = "LABEL_TYPE";
	private static final String STYLE_GROUP_EDGE = "EDGE";
	
	private class MyCellObject {
		private String link;
		private String title;
		
		public MyCellObject(String link, String title) {
			setLink(link);
			setTitle(title);
		}

		public void setLink(String link) {
			this.link = link;
		}

		public String getLink() {
			return link;
		}

		public void setTitle(String title) {
			this.title = title;
		}

		public String getTitle() {
			return title;
		}
	}
	
	private class SVGCanvasFactory extends CanvasFactory {
		public mxSvgCanvas createCanvas(int width, int height)
		{
			return new mxSvgCanvas(mxDomUtils.createSvgDocument(width,height));
		}
	}
	
	private class Graph extends mxGraph {
		private Map<Object,List<Object>> subelements = new HashMap<Object,List<Object>>();
		private Object parentElement;
		
		protected String getLinkForCell(Object cell)
		{
			mxICell c = (mxICell)cell;
			if (c.getValue() instanceof MyCellObject) {
				MyCellObject cellObject = (MyCellObject)c.getValue();
				return cellObject.getLink();
			}
			/*
			if (c.getChildCount() > 0 && c.getValue() != null && (c.getValue().toString().startsWith("http") || c.getValue().toString().startsWith("#") || c.getValue().toString().startsWith("javascript"))) {
				return c.getValue().toString();
			}
			*/
			return null;
		}
		
		public String getToolTipForCell(Object cell)
		{
			mxICell c = (mxICell)cell;
			if (c.getValue() instanceof MyCellObject) {
				MyCellObject cellObject = (MyCellObject)c.getValue();
				return cellObject.getTitle();
			}
			return null;
		}
		
		//regroup dom nodes, so that child nodes are grouped with parent node within same anchor node (link)
		protected void cellDrawn(mxICanvas canvas, mxCellState state,
                Object element, Object labelElement)
        {
            super.cellDrawn(canvas, state, element, labelElement);
            Node n = ((Element) element).getParentNode();
            Element e = (Element) n;
            if(e.getNodeName().equals("a") && e.getAttribute("xlink:href").contains("javascript")){ 
            	if(!e.getAttribute("xlink:title").isEmpty()){
	            	Element newLink = e.getOwnerDocument().createElement("a");
	            	newLink.setAttribute("xlink:title", e.getAttribute("xlink:title"));
	            	newLink.appendChild(e.getElementsByTagName("rect").item(0));
	            	e.appendChild(newLink);
            	}
	            e.setAttribute("style", "cursor: pointer;");
	            e.setAttribute("onclick", e.getAttribute("xlink:href").replace("javascript:", ""));
	            e.removeAttribute("xlink:href");
	            e.removeAttribute("xlink:title");
	            n.getOwnerDocument().renameNode(n, n.getOwnerDocument().getNamespaceURI(), "g");
            }
            
			mxICell c = (mxICell) state.getCell();
			if (!c.getParent().equals(this.getDefaultParent())) {
				List<Object> children = subelements.get(parentElement);
				if (children == null) {
					children = new ArrayList<Object>();
					subelements.put(parentElement, children);
				}
				children.add(element);
				children.add(labelElement);
			} else {
				for (Object parent : subelements.keySet()) {
					if (parent instanceof Element) {
						Element p = (Element) parent;
						for (Object child : subelements.get(parent)) {
							Node parentNode = p.getParentNode();
							parentNode.appendChild((Element)child);
						}
					}
				}
				subelements.clear();
				parentElement = element;
			}
		}
	}
	
	private Properties properties;
	private String componentName;
	private mxGraph graph;
	private int ListMaxX = 0;
	private IProject project;
	private List<String> invalidComponents = new ArrayList<String>();
	private HashMap<String, ExecutionDetail> executionDetailEntryMap = new HashMap<String, ExecutionDetail>();
	private ExecutionState executionState;
	private boolean modifiedAfterExecution;

	public GraphUtilNG(IProject project, String componentName, Properties graphProperties, List<Locator> invalidComponentsLocs, Long id) throws ConfigurationException, RuntimeException {
		this(project,componentName, graphProperties, invalidComponentsLocs);
		
		if(id>=0){
			// add Execution Detail information
			List<ExecutionDetail> details = StateManager.getInstance().getExecutionDetails(id);
			// long overallTime = 0;
			for (int i=0; i<details.size(); i++) {
				executionDetailEntryMap.put(details.get(i).getLocator(), details.get(i));
			}			
			executionState = StateManager.getInstance().getResult(id);
			Date endDate = StateManager.getInstance().getResult(id).getStopDate();
			Date modified = new Date(Long.valueOf(ConfigManager.getInstance().get(project.getLocator()).getAttributeValue("modified")));
			modifiedAfterExecution = endDate.before(modified);
		}		
	}
	
	
	public GraphUtilNG(IProject project, String componentName, Properties graphProperties, List<Locator> invalidComponentsLocs) throws ConfigurationException, RuntimeException {
		
		this.project = project;
		this.componentName=componentName.trim();			
		this.properties = getDefaultProperties();
		properties.putAll(graphProperties);		
		
		invalidComponents = new ArrayList<String>();
		boolean isValid=true;
		for(Locator loc:invalidComponentsLocs) {
			if(!loc.toString().equals(componentName))
				invalidComponents.add(loc.toString());
			else {
				isValid=false;
				invalidComponents.add(0,loc.toString());
			}
		}
		// Only if the focused component is invalid, all invalid components are shown
		if (isValid)
			invalidComponents.clear();
		
	}
	
	public void generate() throws ConfigurationException, RuntimeException {
		ViewTypes viewType = parseViewType(properties.getProperty("viewType"));		
		graph = new Graph(); 
		setStyles();
				
		Map<String, mxICell> cellMap = new HashMap<String,mxICell>();
		
		// if this is a project
		if(Locator.parse(componentName).isRoot()){
			buildListsView(cellMap);
		}
		else if(Locator.parse(componentName).getManager().equals(ITypes.Managers.variables.toString())){
			buildVariablesView(cellMap);
		} else {
			
			Set<String> rootComponents = new HashSet<String>();
			List<Object> rootCells = new ArrayList<Object>();
			
			graph.getModel().beginUpdate();
			try
			{
				switch (viewType) {
				case all: {
					Map<String,List<String>> dependants = getComponentDependents();
					Map<String,List<String>> dependancies = getComponentDependencies();	
					handleRelations(dependants, true, cellMap);						
					handleRelations(dependancies, false, cellMap);			
					rootComponents = getTerminalDependants(componentName, dependants, rootComponents);	
					break;
				}
				case dependencies : {
					// get Graph for dependencies
					Map<String,List<String>> dependancies = getComponentDependencies();
					handleRelations(dependancies, false, cellMap);
					rootComponents.add(componentName);
					break;
				}
				case dependants : {
					// get Graph for dependants
					Map<String,List<String>> dependants = getComponentDependents();
					handleRelations(dependants, true, cellMap);
					rootComponents = getTerminalDependants(componentName, dependants, rootComponents);				
					break;
				}
				default: break;
				}
			}
			finally
			{
				graph.getModel().endUpdate();
			}
			
			//handle invalid cells
			int minY = 0;
			for (int i=0;i<invalidComponents.size();i++) {
				mxICell invalidCell = addCell(invalidComponents.get(i),cellMap,STYLE_GROUP_INVALID);
				invalidCell.getGeometry().setY(minY);
				minY+=50;
				if(i==0){
					rootCells.add(invalidCell);
				}
				properties.setProperty("etlProjectState", "invalid");
			}
			
			for (String root : rootComponents) {
				rootCells.add(cellMap.get(root));
			}
			
			applyLayout(rootCells);
		}
	}
	
	public mxGraph getGraph() {
		return graph;
	}
	
	private void applyLayout(List<Object> rootCells) {
		int orientation = parseInt(properties.getProperty("orientation","1"));
		mxHierarchicalLayout graphLayout = new mxHierarchicalLayout(graph,orientation);
		graphLayout.execute(graph.getDefaultParent(), rootCells);
	}
		
/*	
	private int addLineToTitle (mxSvgCanvas canvas, int inset, Map<String,Object> style, mxRectangle titleLabelSize, int currentMaxWidth, String text) {
		mxRectangle titleTextSize = mxUtils.getLabelSize(text, style, true, 1.0);
		canvas.drawText(text, inset, inset+(int)titleLabelSize.getHeight()*3+12, (int)titleTextSize.getWidth(), (int)titleTextSize.getHeight(), style);
		return Math.max(currentMaxWidth, (int)titleTextSize.getWidth());		
	}	
*/
	
	private int createTitle(mxSvgCanvas canvas, int inset) {
		int maxWidth = 0;
		Map<String, Object> style = new HashMap<String,Object>();
		style.put(mxConstants.STYLE_FONTFAMILY, properties.getProperty("fontFamily","Lucida Grande"));
		style.put(mxConstants.STYLE_FONTCOLOR, properties.getProperty("foreground","#000000"));
		style.put(mxConstants.STYLE_FONTSIZE, parseInt(properties.getProperty("fontSizeTitle","11")));
		boolean invalid = properties.getProperty("etlProjectState","valid").equalsIgnoreCase("invalid");
		Locator locator = Locator.parse(componentName);
		String title = "Project: "+locator.getRootName() + (invalid?" (INVALID)":"");
		mxRectangle titleLabelSize = mxUtils.getLabelSize(title, style, true, 1.0);
		maxWidth = Math.max(maxWidth, (int)titleLabelSize.getWidth()+10);
		canvas.drawText(title, inset, inset, (int)titleLabelSize.getWidth(), (int)titleLabelSize.getHeight(), style);
		String titleDetail = locator.getType().substring(0, 1).toUpperCase()+locator.getType().substring(1)+": "+locator.getName();
		if(titleDetail.equals(title)){
			titleDetail = "Components overview graph";
		}
		mxRectangle titleDetailSize = mxUtils.getLabelSize(titleDetail, style, true, 1.0);
		maxWidth = Math.max(maxWidth, (int)titleDetailSize.getWidth());
		canvas.drawText(titleDetail, inset, inset+(int)titleLabelSize.getHeight()+5, (int)titleDetailSize.getWidth(), (int)titleDetailSize.getHeight(), style);

		/* write current time/date
		style.put(mxConstants.STYLE_FONTSIZE, parseInt(properties.getProperty("fontSizeType","9")));
		DateFormat f = new SimpleDateFormat("dd.MM.yyyy HH:mm");
		String dateString = f.format(new Date());
		mxRectangle titleDateSize = mxUtils.getLabelSize(dateString, style, true, 1.0);
		canvas.drawText(dateString, inset, inset+(int)titleLabelSize.getHeight()*2+10, (int)titleDateSize.getWidth(), (int)titleDateSize.getHeight(), style);
		maxWidth = Math.max(maxWidth, (int)titleDateSize.getWidth());
		*/
				
		style.remove(mxConstants.STYLE_FONTCOLOR);
		style.put(mxConstants.STYLE_FONTCOLOR, properties.getProperty("foregroundDetail","#CC2200"));
		style.put(mxConstants.STYLE_FONTSIZE, parseInt(properties.getProperty("fontSizeTitle","11")));
		
		// add Execution Info to Title
		if (executionState!=null) {
			String text;
			int offset=15;
			text = "Execution ID: " + executionState.getId()+" ("+executionState.getStatus().toString()+")";
			mxRectangle titleTextSize = mxUtils.getLabelSize(text, style, true, 1.0);
			canvas.drawText(text, inset, inset+(int)titleLabelSize.getHeight()*3+offset, (int)titleTextSize.getWidth(), (int)titleTextSize.getHeight(), style);
			maxWidth=Math.max(maxWidth, (int)titleTextSize.getWidth());		

			text = "Start Date: " + new SimpleDateFormat().format(executionState.getStartDate());
			titleTextSize = mxUtils.getLabelSize(text, style, true, 1.0);
			canvas.drawText(text, inset, inset+(int)titleLabelSize.getHeight()*3+offset+18, (int)titleTextSize.getWidth(), (int)titleTextSize.getHeight(), style);
			maxWidth=Math.max(maxWidth, (int)titleTextSize.getWidth());		
			
			// it is in ms, so x1000
			text = "Duration: " +  getSeconds((executionState.getStopDate()==null) ? 0 : ((executionState.getStopDate().getTime() - executionState.getStartDate().getTime()))*1000); 
			titleTextSize = mxUtils.getLabelSize(text, style, true, 1.0);
			canvas.drawText(text, inset, inset+(int)titleLabelSize.getHeight()*3+offset+36, (int)titleTextSize.getWidth(), (int)titleTextSize.getHeight(), style);
			maxWidth=Math.max(maxWidth, (int)titleTextSize.getWidth());		

			if (modifiedAfterExecution) { 
				style.put(mxConstants.STYLE_FONTSTYLE,mxConstants.FONT_BOLD);
				text = "WARNING: Project was modified after the execution has been finished!";
				titleTextSize = mxUtils.getLabelSize(text, style, true, 1.0);
				canvas.drawText(text, inset, inset+(int)titleLabelSize.getHeight()*3+offset+54, (int)titleTextSize.getWidth(), (int)titleTextSize.getHeight(), style);
				maxWidth=Math.max(maxWidth, (int)titleTextSize.getWidth());
			}	
		}
		return maxWidth;
	}
	
	public String getSVG() {
		int titleWidth = createTitle(new SVGCanvasFactory().createCanvas(1024, 768), 0); //draw title on dummy canvas to determine max width.
		mxRectangle boundary = graph.getView().getGraphBounds();
		int translateX = 17;
		int translateY = 70;
		boundary.setY(boundary.getY()-translateY);
		boundary.setX(boundary.getX()-translateX);
		boundary.setWidth(Math.max(ListMaxX,Math.max(boundary.getWidth()+translateX,titleWidth+translateX)));
		boundary.setHeight(boundary.getHeight()+translateY);
		mxSvgCanvas canvas = (mxSvgCanvas)mxCellRenderer.drawCells(graph, graph.getChildCells(graph.getDefaultParent()), 1, boundary, new SVGCanvasFactory());
		createTitle(canvas,translateX);
		//Document document = mxCellRenderer.createSvgDocument(graph, graph.getChildCells(graph.getDefaultParent()), 1, Color.black, boundary);
		Document document = canvas.getDocument();
		TransformerFactory tFactory = TransformerFactory.newInstance();
		Transformer transformer;
		try {
			transformer = tFactory.newTransformer();
			DOMSource source = new DOMSource(document);
			StringWriter writer = new StringWriter();
			StreamResult resultStream = new StreamResult(writer);
			transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");
			transformer.setOutputProperty(OutputKeys.METHOD, "xml");
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");
			transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
			transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "4");
			transformer.transform(source, resultStream);
			writer.close();
			return writer.toString();
		}
		catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
	private void setStyles() throws RuntimeException {
		mxStylesheet stylesheet = graph.getStylesheet();
		Map<String, Object> commonStyle = new HashMap<String,Object>();
		commonStyle.put(mxConstants.STYLE_FONTFAMILY, properties.getProperty("fontFamily","Lucida Grande"));
		commonStyle.put(mxConstants.STYLE_STROKEWIDTH, parseFloat(properties.getProperty("lineWidth","1"),"lineWidth"));
		commonStyle.put(mxConstants.STYLE_FONTCOLOR, properties.getProperty("foreground","#000000"));
		commonStyle.put(mxConstants.STYLE_STROKECOLOR, "#000000");
		commonStyle.put(mxConstants.STYLE_SHAPE, mxConstants.SHAPE_RECTANGLE);
		commonStyle.put(mxConstants.STYLE_ROUNDED, true);
		commonStyle.put(mxConstants.STYLE_OPACITY, 100);
		commonStyle.put(mxConstants.STYLE_OVERFLOW, "fill");
		commonStyle.put(mxConstants.STYLE_NOLABEL, true);
		//add component specific styles
		for (ITypes.Components c : ITypes.Components.values()) {
			Map<String, Object> style = new HashMap<String, Object>();
			style.putAll(commonStyle);
			style.put(mxConstants.STYLE_FONTCOLOR, properties.getProperty(c.toString()+"Foreground","#000000"));
			style.put(mxConstants.STYLE_FILLCOLOR, properties.getProperty(c.toString()+"Background","#FFFFFF"));
			style.put(mxConstants.STYLE_STROKECOLOR, properties.getProperty(c.toString()+"Border","#000000"));
			stylesheet.putCellStyle(c.toString(), style);
		}
		//add invalid style
		Map<String, Object> invalidStyle = new HashMap<String, Object>();
		invalidStyle.putAll(commonStyle);
		invalidStyle.put(mxConstants.STYLE_FONTCOLOR, properties.getProperty("invalidForeground","#000000"));
		invalidStyle.put(mxConstants.STYLE_FILLCOLOR, properties.getProperty("invalidBackground","#FFFFFF"));
		invalidStyle.put(mxConstants.STYLE_STROKECOLOR, properties.getProperty("invalidBorder","#000000"));
		stylesheet.putCellStyle(STYLE_GROUP_INVALID, invalidStyle);
		//add component name label style
		Map<String, Object> nameStyle = new HashMap<String, Object>();
		nameStyle.putAll(commonStyle);
		nameStyle.put(mxConstants.STYLE_STROKEWIDTH,0);
		nameStyle.put(mxConstants.STYLE_OPACITY, 0);
		nameStyle.put(mxConstants.STYLE_FONTSIZE, parseInt(properties.getProperty("fontSizeName","11")));
		nameStyle.put(mxConstants.STYLE_NOLABEL, false);
		stylesheet.putCellStyle(STYLE_GROUP_LABEL_NAME, nameStyle);
		//add component type label style
		Map<String, Object> typeStyle = new HashMap<String, Object>();
		typeStyle.putAll(commonStyle);
		typeStyle.put(mxConstants.STYLE_STROKEWIDTH,0);
		typeStyle.put(mxConstants.STYLE_OPACITY, 0);
		typeStyle.put(mxConstants.STYLE_FONTSIZE, parseInt(properties.getProperty("fontSizeType","9")));
		typeStyle.put(mxConstants.STYLE_NOLABEL, false);
		stylesheet.putCellStyle(STYLE_GROUP_LABEL_TYPE, typeStyle);
		//add edge styles
		Map<String, Object> edgeStyle = new HashMap<String, Object>();
		edgeStyle.put(mxConstants.STYLE_STARTARROW,mxConstants.ARROW_CLASSIC);
		edgeStyle.put(mxConstants.STYLE_ENDARROW,"none");
		edgeStyle.put(mxConstants.STYLE_STROKECOLOR,"#000000");
		stylesheet.putCellStyle(STYLE_GROUP_EDGE, edgeStyle);
	}
	
	public Properties getDefaultProperties() {
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
			properties.setProperty("fontSizeTitle", "11");
			properties.setProperty("linkPrefix", "http://localhost/");
			properties.setProperty("fontFamily", "Lucida Grande");
			properties.setProperty("cellPadding", "10");
			properties.setProperty("lineWidth", "1");
			properties.setProperty("lineWidthFocus", "3");	
			properties.setProperty("foregroundDetail", "#000000");
			properties.setProperty("projectForegroundDetail", "#000000");
			properties.setProperty("jobForegroundDetail", "#000000");
			properties.setProperty("loadForegroundDetail", "#000000");
			properties.setProperty("transformForegroundDetail", "#000000");
			properties.setProperty("extractForegroundDetail", "#000000");
			properties.setProperty("connectionForegroundDetail", "#000000");						
			properties.setProperty("tooltip", "none");						
		}
		return properties;
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
	
	private int parseInt(String value) {
		try {
			return Integer.parseInt(value);
		}
		catch (Exception e) {
			return 0;
		}
	}
	
	private float parseFloat(String number, String name) throws RuntimeException {
		try {
			return Float.valueOf(number);
		}
		catch (Exception e) {
			throw new RuntimeException("Parameter "+name+" with value "+number+" must be a number!");
		}
	}
	
	private boolean hasType(String component, String type) {
		return Locator.parse(component).getType().equals(type);
	}
	
	private void buildListsView(Map<String, mxICell> cellMap) throws RuntimeException{
		int minX = 0;
		int minY = 0;
		double maxXInType = 0;
		
		String lastType = null;
			for (IComponent comp:project.getModelComponents()) {
			
				mxICell cell = addCell(comp.getLocator().toString(),cellMap,null);
				
				if(!comp.getLocator().getType().equals(lastType) && lastType!=null){
					minX=(int)(maxXInType+20);
					minY = 0;
				}
								
				lastType = comp.getLocator().getType();
				cell.getGeometry().setY(minY);
				cell.getGeometry().setX(minX);
				minY+=50;
				maxXInType = Math.max(maxXInType, minX+cell.getGeometry().getWidth());
			}
			
			ListMaxX=(int) maxXInType + 20;
	}
	
	private void buildVariablesView(Map<String, mxICell> cellMap) throws RuntimeException, ConfigurationException{
		int minX = 0;
		int minY = 0;
		double maxXInType = 0;
		
		Locator loc = Locator.parse(componentName);
		ArrayList<Locator> locLst = new ArrayList<Locator>();
		locLst.add(loc);
		HashMap<String, LinkedHashSet<String>> dependents =  new VariablesDependencyFind(locLst).getDependents();
			for (String depStr:dependents.get(loc.getName())) {
				Locator dep = Locator.parse(depStr);
				IComponent comp = project.getContext().getComponent(dep);
				mxICell cell = addCell(comp.getLocator().toString(),cellMap,null);
				cell.getGeometry().setY(minY);
				cell.getGeometry().setX(minX);
				minY+=50;
				maxXInType = Math.max(maxXInType, minX+cell.getGeometry().getWidth());
			}
			
			ListMaxX=(int) maxXInType + 20;
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

		// Remove dependencies between connections and load, except if Flow Graph is done for connection or load 			
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
		//remove the invalid component from the dependents map
		for(String invalid:invalidComponents)
			map.remove(invalid);
		modifyDependencyMap(map, "connection","load");
		return map;
	}
	
	private String getConfigString(String qname) {
		try {
			org.jdom.Element element = (org.jdom.Element) ConfigManager.getInstance().findElement(Locator.parse(qname)).clone();
			element.removeAttribute("modified");
			element.removeAttribute("modifiedBy");
			StringWriter writer = new StringWriter();
			org.jdom.Document document = new org.jdom.Document();
			document.setRootElement(element);
			XMLOutputter outputter = new XMLOutputter(Format.getPrettyFormat().setOmitDeclaration(true));
			//XMLOutputter outputter = new XMLOutputter(Format.getCompactFormat());
			outputter.output(document, writer);
			return writer.toString();
		} catch (Exception e) {
			return "no config found.";
		} 
	}
	
	
	
	private mxICell addCell(String qname,Map<String, mxICell> addedComponents, String style) throws RuntimeException {
		try {
			Locator l = Locator.parse(qname);
			String componentType = l.getType();
			String componentImplementation = ConfigManager.getInstance().get(l).getAttributeValue("type","");
			String componentNameLabel = l.getName();
			String componentTypeLabel = componentType.substring(0, 1).toUpperCase()+componentType.substring(1)+": "+componentImplementation;

			String timeLabel="";
			String outputCallsLabel="";
			String outputRowsLabel="";
			double detailMaxWidth = 0;
			ExecutionDetail detail = executionDetailEntryMap.get(l.toString());
			if(detail!=null){
				timeLabel = "Time: " + getSeconds(detail.getRuntime());
				if (componentType.equals("load")) {
					outputCallsLabel = "Calls: "+ detail.getInputCalls();
					outputRowsLabel = "Rows: "+ detail.getProcessedInputRows();
				} else {
					outputCallsLabel = "Calls: "+ detail.getOutputCalls();
					outputRowsLabel = "Rows: "+ detail.getProcessedOutputRows();					
				}							
				double timeLabelWidth = mxUtils.getLabelSize(outputRowsLabel, graph.getStylesheet().getStyles().get(STYLE_GROUP_LABEL_TYPE), true, 1.0).getWidth();
				double outputCallsLabelWidth = mxUtils.getLabelSize(outputRowsLabel, graph.getStylesheet().getStyles().get(STYLE_GROUP_LABEL_TYPE), true, 1.0).getWidth();
				double outputRowsLabelWidth = mxUtils.getLabelSize(outputRowsLabel, graph.getStylesheet().getStyles().get(STYLE_GROUP_LABEL_TYPE), true, 1.0).getWidth();
				detailMaxWidth = Math.max(timeLabelWidth, Math.max(outputCallsLabelWidth, outputRowsLabelWidth));				
			}
			
			boolean externalStyle = style != null;
			if (!externalStyle) style = componentType;
			if (qname.equals(this.componentName)) {
				style = style + ";"+mxConstants.STYLE_STROKEWIDTH+"="+properties.getProperty("lineWidthFocus");
			}
			
			double typeLabelWidth = mxUtils.getLabelSize(componentTypeLabel, graph.getStylesheet().getStyles().get(STYLE_GROUP_LABEL_TYPE), true, 1.0).getWidth();
			double nameLabelWidth = mxUtils.getLabelSize(componentNameLabel, graph.getStylesheet().getStyles().get(STYLE_GROUP_LABEL_NAME), true, 1.0).getWidth();
			double containerWidth = Math.max(detailMaxWidth,Math.max(typeLabelWidth,nameLabelWidth)) + 2 * parseInt(properties.getProperty("callPadding", "10"));
			//
			String onClick = properties.getProperty("onClick","").replace("%", l.toString()); 
			String link = properties.getProperty("linkPrefix")+l.toString(); //add custom rendered link
			String containerValue = onClick.isEmpty() ? link : "javascript:"+onClick;//+";return false;";
			// title used for tooltip
			String title = (properties.getProperty("tooltip").equalsIgnoreCase("xml")) ? getConfigString(qname) : "";
			MyCellObject cellObject = new MyCellObject(containerValue,title);
			mxICell cellContainer = (mxICell)graph.insertVertex(graph.getDefaultParent(), null, cellObject, 0, addedComponents.size()*50, containerWidth,40,style);
			//apply labels
			String color = externalStyle ? "" : ";"+mxConstants.STYLE_FONTCOLOR+"="+properties.getProperty(componentType+"Foreground");
			graph.insertVertex(cellContainer, null, componentTypeLabel, 0, 5, containerWidth, 15,STYLE_GROUP_LABEL_TYPE+color);
			graph.insertVertex(cellContainer, null, componentNameLabel, 0, 20,containerWidth, 20,STYLE_GROUP_LABEL_NAME+color);
			
			if(detail!=null){		
				String colorDetail = externalStyle ? "" : ";"+mxConstants.STYLE_FONTCOLOR+"="+properties.getProperty(componentType+"ForegroundDetail");				
				graph.insertVertex(cellContainer, null, timeLabel , 0, 37,containerWidth, 20,STYLE_GROUP_LABEL_TYPE+colorDetail);
				graph.insertVertex(cellContainer, null, outputCallsLabel, 0, 52,containerWidth, 20,STYLE_GROUP_LABEL_TYPE+colorDetail);
				graph.insertVertex(cellContainer, null, outputRowsLabel, 0, 67,containerWidth, 20,STYLE_GROUP_LABEL_TYPE+colorDetail);
			}
		
			addedComponents.put(qname, cellContainer);
			return cellContainer;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to add cell "+qname+" to graph: "+e.getMessage()); 
		}
	}

	/**
	 * @param detail
	 * @return
	 */
	private String getSeconds(long time) {
		return new DecimalFormat("###.##").format(((double)time/1000000)) + "s";
	}
	
	private void addEdge(mxICell source, mxICell target, boolean withArrow, boolean forward,boolean visible) {
		// Create Edge
		mxICell edge = null;
		if (!forward) {
			edge = (mxICell)graph.insertEdge(graph.getDefaultParent(), null, "", source, target, STYLE_GROUP_EDGE);
		}
		else {
			edge = (mxICell)graph.insertEdge(graph.getDefaultParent(), null, "", target, source, STYLE_GROUP_EDGE);
		}
		
		if(!visible)
			edge.setVisible(false);
	}
	
	private void handleRelations(Map<String,List<String>> map, boolean forward, Map<String, mxICell> addedComponents) throws RuntimeException {
		handleRelations(componentName, map, forward, addedComponents, new HashSet<String>());
	}
	
	
	private void handleRelations(String qname, Map<String,List<String>> map, boolean forward, Map<String, mxICell> addedComponents, HashSet<String> componentSet) throws RuntimeException{
		List<String> list = map.get(qname);
		if (list != null && !componentSet.contains(qname)) {
			componentSet.add(qname);
			if (!addedComponents.containsKey(qname)) {
				addCell(qname,addedComponents,null);
			}
			for (String s : list) {
				if (!addedComponents.containsKey(s)) {
					addCell(s,addedComponents,null);
				}
				addEdge(addedComponents.get(qname),addedComponents.get(s),true,forward,true);
				handleRelations(s,map,forward, addedComponents, componentSet);
			}
		}
	}
	
	private Set<String> getTerminalDependants(String componentName, Map<String,List<String>> map, Set<String> terminals) {
		List<String> list = map.get(componentName);
		if (list != null && !list.isEmpty()) {
			for (String s : list) {
				getTerminalDependants(s, map, terminals);
			}
		}
		else {
			if(!invalidComponents.contains(componentName))
				terminals.add(componentName);
		}
		return terminals;
	}
		
	/*public org.jdom.Element getXML() throws RuntimeException {
		String str = getSVG();
		org.jdom.Element element=null;
		try {
			element = XMLUtil.stringTojdom(str);
			element.detach();			
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
		return element;
	}*/


/**
 * @param args
 */
public static void main(String[] args) {
	if (args.length != 1) {
		System.err.println("Component name must be given as argument in locator form.");
		System.exit(-1);
	} else {
		String componentName = args[0];
		String projectName = Locator.parse(componentName).getRootName();
		try {
			XMLReader reader = new XMLReader();
			org.jdom.Document document = reader.readDocument("./samples/"+projectName+".xml");
			org.jdom.Element root = (org.jdom.Element) document.getRootElement().clone();
			projectName = root.getAttributeValue("name");
			ConfigManager.getInstance().add(Locator.parse(projectName), root);
			IProject project = ConfigManager.getInstance().getProject(projectName);
			List<Locator> invalids = ConfigManager.getInstance().initProjectComponents(project, IContext.defaultName, true);
			Properties graphProperties = new Properties();
			graphProperties.setProperty("viewType", ViewTypes.dependencies.toString());
			graphProperties.setProperty("orientation",String.valueOf(SwingConstants.NORTH));
			graphProperties.setProperty("linkPrefix", "#");
			//graphProperties.setProperty("onClick", "Jedox.studio.etl.flowGraphClick('%')");
			//graphProperties.setProperty("onClick", "window.close()");
			Locator componentLocator = Locator.parse(componentName);
			componentLocator.setRootName(projectName);
			JFrame frame = new JFrame();
			GraphUtilNG util = new GraphUtilNG(project,componentLocator.toString(),graphProperties,invalids);
			util.generate();
			System.out.print(util.getSVG());
			System.out.print(util.getSVG());
			mxGraphComponent graphComponent = new mxGraphComponent(util.getGraph());
			frame.getContentPane().add(graphComponent);
			frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
			frame.setSize(800, 600);
			frame.setVisible(true);
		}
		catch (Exception e) {
			e.printStackTrace(); 
		}
	}
}
}
