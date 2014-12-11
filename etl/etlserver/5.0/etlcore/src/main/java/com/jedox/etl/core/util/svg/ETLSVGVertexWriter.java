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
import java.util.Map;
import java.util.Properties;

import org.jgraph.graph.CellView;
import org.jgraph.graph.GraphCell;
import org.jgraph.graph.GraphConstants;
import org.jgraph.graph.GraphLayoutCache;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.jgraph.io.svg.SVGGraphConstants;
import com.jgraph.io.svg.SVGGraphWriter;
import com.jgraph.io.svg.SVGVertexWriter;

public class ETLSVGVertexWriter extends SVGVertexWriter {
	
	
	public static int LINESPACING = 4;
	private Properties properties;
	
	
	public ETLSVGVertexWriter(Properties properties) {
		this.properties = properties;
	}
	
	
	public String getHexEncoding(Color color) {
		String c = "";
		if (color == null) {
			c = "none";
		} else {
			c = Integer.toHexString(color.getRGB() & 0x00ffffff);

			// Pads with zeros
			while (c.length() < 6) {
				c = "0"+c;
			}
			c = "#" + c;
		}
		return c;
	}
	
	/**
	 * Returns the SVG representation of the specified node.
	 * 
	 * @param writer
	 * @param document
	 * @param cache
	 * @param view
	 * @param dx
	 * @param dy
	 * @return an XML node describing the specified node
	 */
	public Node createNode(SVGGraphWriter writer, Document document,
			GraphLayoutCache cache, CellView view, double dx, double dy) {
		Rectangle2D bounds = view.getBounds();
		Map attributes = view.getAllAttributes();
		Element href = (Element) document.createElement("a");
		String link = GraphConstants.getLink(attributes);
		if (link != null) {
			href.setAttribute("xlink:href", link);
		}
		
		Object componentState = attributes.get("componentState");
		boolean invalid = (componentState != null && componentState.toString().equalsIgnoreCase("invalid"));

		// Specifies the shape geometry
		int shapeType = SVGGraphConstants.getShape(attributes);
		Color background = invalid ? Color.decode(properties.getProperty("invalidBackground")) : GraphConstants.getBackground(attributes);
		String hexBackground = null;
		if (background != null) {
			hexBackground = getHexEncoding(background);
		}
		Color gradient = GraphConstants.getGradientColor(attributes);
		String hexGradient = null;		
		if (gradient != null) {
			// TODO need proper definition for gradient colours
			// For now put the gradient colour in the background
			// In future need a proper definition for SVG
			hexGradient = getHexEncoding(gradient);
		}
		
		Color borderColor = invalid ? Color.decode(properties.getProperty("invalidBorder")) : GraphConstants.getBorderColor(attributes);
		String hexLineColor = null;
		if (borderColor != null) {
			hexLineColor = getHexEncoding(borderColor);
		}
		float lineWidth = GraphConstants.getLineWidth(attributes);

		// Adds a drop shadow
		boolean dropShadow = SVGGraphConstants.isShadow(attributes);
		if (dropShadow) {
			int dist = SHADOW_DISTANCE;
			href.appendChild(writer.createShapeNode(document, shapeType,
					bounds, dx - dist, dy - dist, HEXCOLOR_SHADOW, null,
					"none", lineWidth, SHADOW_OPACITY, false));
		}
		href.appendChild(writer.createShapeNode(document, shapeType, bounds,
				dx, dy, hexBackground, hexGradient, hexLineColor, lineWidth,
				1.0, false));

		// Adds the image
		// This is currently not implemented due to bugs in all
		// known SVG viewers, either ignoring the image at all
		// or ignoring the individual sizes for the image (firefox)
		// String imageURL = "http://www.jgraph.co.uk/images/logo.gif";
		// if (imageURL != null) {
		// Element image = (Element) document.createElement("image");
		// image.setAttribute("x", String.valueOf(bounds.getX() - dx));
		// image.setAttribute("y", String.valueOf(bounds.getY() - dy));
		// image.setAttribute("w", String.valueOf(bounds.getWidth()));
		// image.setAttribute("xlink:href", imageURL);
		// image.setAttribute("h", String.valueOf(bounds.getHeight()));
		// image.setAttribute("y", String.valueOf(bounds.getY() - dy));
		// image.setAttribute("preserveAspectRatio", "none");
		// href.appendChild(image);
		// }

		// Draws the labels stored in the user object
		GraphCell cell = (GraphCell)view.getCell();
		String typeLabel = cell.getAttributes().get("componentTypeLabel").toString();
		String nameLabel = cell.getAttributes().get("componentName").toString();
		int x = Double.valueOf(bounds.getX() - dx + bounds.getWidth() / 2).intValue();
		Font typeFont = GraphUtil.getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeType"));
		Font nameFont = GraphUtil.getFont(properties.getProperty("fontFamily"),Font.PLAIN,properties.getProperty("fontSizeName"));
		Color fontColor = invalid ? Color.decode(properties.getProperty("invalidForeground")) :GraphConstants.getForeground(cell.getAttributes());
		String hexFontColor = null;
		if (fontColor != null) {
			hexFontColor = getHexEncoding(fontColor);
		}
		int nameFontsize = nameFont.getSize();
		int textHeight = (nameFontsize + LINESPACING);
		int yOffset = Double.valueOf((bounds.getHeight() - textHeight) / 2).intValue() + nameFontsize + 5;
		
		href.appendChild(writer.createTextNode(document, nameLabel, "middle", nameFont, hexFontColor, x,Double.valueOf(bounds.getY() + yOffset - dy).intValue()));
		href.appendChild(writer.createTextNode(document, typeLabel, "middle", typeFont, hexFontColor, x,Double.valueOf(bounds.getY() + nameFontsize -dy).intValue()));
		
		return href;
	}

}
