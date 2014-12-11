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

import java.awt.Font;
import java.awt.geom.Rectangle2D;
import java.io.OutputStream;
import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Iterator;
import java.util.Properties;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.jgraph.graph.CellView;
import org.jgraph.graph.GraphLayoutCache;
import org.jgraph.graph.GraphModel;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.jedox.etl.core.component.Locator;
import com.jgraph.io.svg.SVGGraphConstants;
import com.jgraph.io.svg.SVGGraphWriter;

public class ETLSVGGraphWriter extends SVGGraphWriter {
	
	private Properties properties;
	
	public ETLSVGGraphWriter(Properties properties) {
		this.properties = properties;
		vertexFactory = new ETLSVGVertexWriter(properties);
		edgeFactory = new ETLSVGEdgeWriter(properties);
	}
	
	public void write(OutputStream out, String title, GraphLayoutCache cache,
			int inset) {
		try {
			Document document = DocumentBuilderFactory.newInstance()
					.newDocumentBuilder().newDocument();
			Node root = createNode(document, title, cache, inset);
			//add status invalid message if project is not valid
		
			document.appendChild(root);
			Transformer transformer = TransformerFactory.newInstance().newTransformer();
			transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");
			transformer.setOutputProperty(OutputKeys.METHOD, "xml");
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");
			transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
			transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "4");
			transformer.transform(
					new DOMSource(document), new StreamResult(out));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	
	protected Node createNode(Document document, String title,
			GraphLayoutCache cache, int inset)
			throws ParserConfigurationException {

		// Collects basic geometric details
		Rectangle2D bounds = GraphLayoutCache.getBounds(cache.getAllViews());
		double dx = bounds.getX() - inset;
		double dy = bounds.getY() - inset;
		
		int titleshift = 40;
		double width = Math.max(100, bounds.getWidth() + 2 * inset);
		double height = bounds.getHeight() + 2 * inset + titleshift;
		Node root = createRoot(document, width, height, inset);

		// Adds defs for filters, gradients, markers etc
		Element defs = (Element) document.createElement("defs");
		// Define arrow markers
		Element g = (Element) document.createElement("g");
		g.setAttribute("id", "arrowMarker");
		defs.appendChild(g);
		Element stroke = (Element) document.createElement("g");
		stroke.setAttribute("stroke-width", "0");
		g.appendChild(stroke);
		Element path = (Element) document.createElement("path");
		path.setAttribute("d", "M 4 -2 L 0 0 L 4 2 L 3 1 L 3 -1 L 4 -2");
		stroke.appendChild(path);
		Element marker = (Element) document.createElement("marker");
		marker.setAttribute("id", "startMarker");
		marker.setAttribute("markerWidth", "48");
		marker.setAttribute("markerHeight", "24");
		marker.setAttribute("viewBox", "-4 -4 25 5");
		marker.setAttribute("orient", "auto");
		marker.setAttribute("refX", "0");
		marker.setAttribute("refY", "0");
		marker.setAttribute("markerUnits", "strokeWidth");
		defs.appendChild(marker);
		g = (Element) document.createElement("g");
		marker.appendChild(g);
		Element use = (Element) document.createElement("use");
		use.setAttribute("xlink:href", "#arrowMarker");
		use.setAttribute("transform", "rotate(180)");
		use.setAttribute("stroke-width", "1");
		g.appendChild(use);

		// Also definition for non-rotated arrow
		marker = (Element) document.createElement("marker");
		marker.setAttribute("id", "endMarker");
		marker.setAttribute("markerWidth", "48");
		marker.setAttribute("markerHeight", "24");
		marker.setAttribute("viewBox", "-4 -4 25 5");
		marker.setAttribute("orient", "auto");
		marker.setAttribute("refX", "0");
		marker.setAttribute("refY", "0");
		marker.setAttribute("markerUnits", "strokeWidth");
		defs.appendChild(marker);
		g = (Element) document.createElement("g");
		marker.appendChild(g);
		use = (Element) document.createElement("use");
		use.setAttribute("xlink:href", "#arrowMarker");
		use.setAttribute("stroke-width", "1");
		g.appendChild(use);

		root.appendChild(defs);

		if (title != null && title.length() > 0) {
			
			Font titleFont = GraphUtil.getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeTitle"));
			Font typeFont = GraphUtil.getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeType"));
			Locator locator = Locator.parse(title);
			root.appendChild(createTextNode(document, "Project: "+locator.getRootName(), null, titleFont, properties.getProperty("foreground"), inset / 2, inset / 2 + titleFont.getSize()+2));
			root.appendChild(createTextNode(document, locator.getType().substring(0, 1).toUpperCase()+locator.getType().substring(1)+": "+locator.getName(), null, titleFont, properties.getProperty("foreground"), inset / 2, inset / 2 + (titleFont.getSize()+4)*2));
			DateFormat f = new SimpleDateFormat("dd.MM.yyyy HH:mm");
			root.appendChild(createTextNode(document, f.format(new Date()), null, typeFont, properties.getProperty("foreground"), inset / 2, inset / 2 + (titleFont.getSize()+4) *3));
			if (properties.getProperty("etlProjectState","valid").equalsIgnoreCase("invalid")) {
				root.appendChild(createTextNode(document, "Status: INVALID", null, titleFont, properties.getProperty("invalidText"), inset / 2, inset / 2 + (titleFont.getSize()+4) *4));
				titleshift += 20;
			}
			dy -= titleshift;
		}

		// "Draws" all views, topmost first
		GraphModel model = cache.getModel();
		CellView[] views = cache.getAllViews();
		for (int i = 0; i < views.length; i++) {
			Object cell = views[i].getCell();
			if (!model.isPort(cell)) {

				// Invokes edge- or vertex renderer based on the cell type
				Node node = (model.isEdge(cell)) ? edgeFactory.createNode(this,
						document, views[i], dx, dy) : vertexFactory.createNode(
						this, document, cache, views[i], dx, dy);
				if (node != null) {
					root.appendChild(node);
				}
			}
		}

		return root;
	}

	/**
	 * Creates a rect or ellipse element based on the specified values.
	 * 
	 * @param document
	 * @param shapeType
	 * @param bounds
	 * @param dx
	 * @param dy
	 * @param hexBackground
	 * @param hexGradient
	 * @param hexLineColor
	 * @param lineWidth
	 * @param opacity
	 * @param dropShadow
	 * @return a node detailing the shape on a vertex element
	 */
	public Node createShapeNode(Document document, int shapeType,
			Rectangle2D bounds, double dx, double dy, String hexBackground,
			String hexGradient, String hexLineColor, float lineWidth,
			double opacity, boolean dropShadow) {
		boolean isEllipse = shapeType == SVGGraphConstants.SHAPE_ELLIPSE;
		Element shape = (Element) document
				.createElement((isEllipse) ? "ellipse" : "rect");
		double w = bounds.getWidth();
		double h = bounds.getHeight();
		if (isEllipse) {
			shape
					.setAttribute("cx", String.valueOf(bounds.getX() + w / 2
							- dx));
			shape
					.setAttribute("cy", String.valueOf(bounds.getY() + h / 2
							- dy));
			shape.setAttribute("rx", String.valueOf(w / 2));
			shape.setAttribute("ry", String.valueOf(h / 2));
		} else {
			shape.setAttribute("x", String.valueOf(bounds.getX() - dx));
			shape.setAttribute("y", String.valueOf(bounds.getY() - dy));
			if (shapeType == SVGGraphConstants.SHAPE_ROUNDRECT) {
				shape.setAttribute("rx", "5");
				shape.setAttribute("ry", "5");
			}
			shape.setAttribute("width", String.valueOf(w));
			shape.setAttribute("height", String.valueOf(h));
		}

		// Draws the background
		shape.setAttribute("fill", hexBackground);
		shape.setAttribute("opacity", String.valueOf(opacity));

		// Draws the border
		shape.setAttribute("stroke", hexLineColor);
		shape.setAttribute("stroke-width", String.valueOf(lineWidth));
		return shape;
	}

	/**
	 * Creates a new text element for the specified details.
	 * 
	 * @param document
	 * @param label
	 * @param align
	 * @param font
	 * @param hexFontColor
	 * @param middleX
	 * @param y
	 * @return a node detailing the label on a cell element
	 */
	public Node createTextNode(Document document, String label, String align,
			Font font, String hexFontColor, int middleX, int y) {
		Element text = (Element) document.createElement("text");
		text.appendChild(document.createTextNode(label));
		int size = 11;
		if (font != null) {
			text.setAttribute("font-family", font.getFamily());
			text.setAttribute("font-size", String.valueOf(font.getSize2D()));
			size = font.getSize();
		} else {
			text.setAttribute("font-family", "Dialog");
			text.setAttribute("font-size", "11");
		}
		text.setAttribute("fill", hexFontColor);
		text.setAttribute("text-anchor", align);
		text.setAttribute("x", String.valueOf(middleX));
		text.setAttribute("y", String.valueOf(y));
		return text;
	}
	

}
