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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.node;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;


/**
 * A {@link ColumnNode}, which acts as a level proxy for the generation of trees (e.g. for a cube dimension).
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class LevelNode extends ColumnNode {
	
	private static final Log log = LogFactory.getLog(LevelNode.class);
	private ElementType elementType;
	private Row attributes = new Row();
	private CoordinateNode weightNode;

	protected LevelNode() {
		super();
	}
	
	protected LevelNode(String name) {
		super(name);
	}

	
	/**
	 * Adds an Attribute by name and input column.
	 * @param name the name of the attribute
	 * @param input the input column delivering the attribute value
	 */
	public void setAttribute(AttributeNode attribute) {
		attributes.addColumn(attribute);
	}
	
	/**
	 * Evaluates an Attribute
	 * @param name the name of the attribute
	 * @return the column describing the attribute and holding its current value
	 */
	public IColumn getAttribute(String name) {
		return attributes.getColumn(name);
	}

	/**
	 * Gets all Attribute Columns of this LevelNode.
	 * @return the Attributes 
	 */
	public Row getAttributes() {
		return attributes;
	}
	
	private double convertWeight(Object weight) {
		if (weight instanceof Double) return (Double)weight; 
		if (weight != null) {
			try {
				return Double.parseDouble(weight.toString());
			}
			catch (NumberFormatException e) {
				log.warn("Weight of level "+getName()+" is not a number: "+weight);
			}
		} 
		return 1.0;
	}
	
	/**
	 * Sets the name of a weight column delivering the weight for the current value of this level node
	 * @param weight the name of the column delivering the weights for the values
	 * 
	 */
	public void setWeight(String weight) {
		Column weightColumn = new Column(weight);
		weightNode = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.internal("weight"), weightColumn);
	}
	
	/**
	 * sets a constant weight for all values delivered via this LevelNode
	 * @param weight the constant weight
	 */
	public void setWeight(Double weight) {
		weightNode = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.internal("weight"), null);
		weightNode.setValue(weight);
	}
	
	public CoordinateNode getWeightNode() {
		return weightNode;
	}
	
	/**
	 * Calculates the weight of the current value of this LevelNode
	 * @return the calculated weigth value.
	 */
	public double getWeight()  {
		return weightNode != null ? convertWeight(weightNode.getValue()) : convertWeight(null);
	}
	
	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof LevelNode) {
			LevelNode n = (LevelNode)source;
			this.elementType = n.elementType;
			this.attributes = n.attributes.clone();
			this.weightNode = n.weightNode;
		}
	}

	public void setElementType(ElementType elementType) {
		this.elementType = elementType;
	}

	public ElementType getElementType() {
		return (elementType != null) ? elementType : ElementType.ELEMENT_NUMERIC;
	}
	
	
}
