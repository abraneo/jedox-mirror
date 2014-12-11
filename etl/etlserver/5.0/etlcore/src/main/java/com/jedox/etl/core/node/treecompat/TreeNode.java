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
package com.jedox.etl.core.node.treecompat;


import com.jedox.etl.core.node.IColumn;
import com.jedox.palojlib.interfaces.IElement.ElementType;


/**
 * Internal representation of a node in a tree. A Node is a wrapper to the underlying {@link TreeElement} allowing the structuring / consolidating of TreeElement into a tree structure. Each TreeElement may be used multiple times in the structure when wrapped by a TreeNode
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TreeNode implements IColumn {
	
	private IColumn parent;
	private IColumn element;
	private double weight;
	private boolean isMasked;
	private int scale;
	
	public TreeNode(IColumn element) {
		this.element = element;
	}
	
	public TreeNode(IColumn parent, IColumn element, double weight) {
		this.parent = parent;
		this.weight = weight;
		this.element = element;
	}

	/**
	 * gets the underlying element, where the operations of this TreeNode are delegated to. This element is the child in the relation induced by this TreeNode 
	 * @return the underlying element
	 */
	public IColumn getElement() {
		return element;
	}
	
	public String getValueType() {
		return getElement().getValueType();
	}

	public String getDefaultValue() {
		return getElement().getDefaultValue();
	}

	public ElementType getElementType() {
		return getElement().getElementType();
	}

	public ElementType getElementType(ElementType defaulttype) {
		return getElementType();
	}
	
	public String getName() {
		return getElement().getName();
	}

	public Object getValue() {
		return getElement().getValue();
	}

	public String getValueAsString() {
		return getElement().getValueAsString();
	}

	public void setValueType(String type) {
		getElement().setValueType(type);
	}

	public void setDefaultValue(String defaultValue) {
		getElement().setDefaultValue(defaultValue);
	}

	public void setElementType(String type) {
		getElement().setElementType(type);
	}

	public void setName(String name) {
		getElement().setName(name);
	}

	public void setValue(Object value) {
		getElement().setValue(value);
	}
	

	public ColumnTypes getColumnType() {
		return getElement().getColumnType();
	}

	public boolean isEmpty() {
		return getElement().isEmpty();
	}
	
	/**
	 * gets the parent element of the structural relation specified by this TreeNode
	 * @return the parent element
	 */
	public IColumn getParent() {
		return parent;
	}
	
	/**
	 * gets the name of the parent of this relation
	 * @return the parent name
	 */
	public String getParentName() {
		return (getParent()==null) ? null : getParent().getName(); 
	}

	/**
	 * sets the parent of this relation
	 * @param parent the parent element
	 */
	public void setParent(IColumn parent) {
		this.parent = parent;
	}

	/**
	 * gets the weight of this relation
	 * @return the weight
	 */
	public double getWeight() {
		return weight;
	}

	/**
	 * sets the weight of this relation
	 * @param weigth the weight
	 */
	public void setWeight(double weigth) {
		this.weight = weigth;
	}
	
	/**
	 * set the masked flag of this relation (useful for filter operations)
	 * @param isMasked the mask status
	 */
	public void mask(boolean isMasked) {
		this.isMasked = isMasked;
	}
	
	/**
	 * determines if this relation is masked. (useful for filter operations)
	 * @return
	 */
	public boolean isMasked() {
		return isMasked;
	}

	public void mimic(IColumn source) {
		getElement().mimic(source);
	}
	
	public int getScale() {
		return scale;
	}
	
	public void setScale(int scale) {
		this.scale=scale;	
	}	

}
