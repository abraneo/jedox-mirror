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



/**
 * Extension of {@link Column}, which allows the setting of an input column. Thus a ColumNode behaves as proxy node to the input column serving as data-backend. With this base class input chaining (passing data input through multiple proxys, possibly doing operations on the data) is made possible. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class ColumnNode extends Column {
	
	private IColumn valueInput;
	
	
	protected ColumnNode() {
		super();
	}
	
	/**
	 * Constructor
	 * @param name the name of the column node
	 */
	protected ColumnNode(String name) {
		super(name);
	} 
	
	/**
	 * Sets an {@link IColumn} as input, which servers as data backend / value input for this ColumnNode.
	 * May be an Column from a Datasource or a Transformation
	 * @param valueInput the column serving as value input
	 */
	public void setInput(IColumn valueInput) {
		if (valueInput != null)
			super.setValue(null);
		this.valueInput = valueInput;
	}
	
	/**
	 * gets the name of the column, where the data is originating. 
	 * @return If an input is set, this the name of the input column is returned. Else the name of this column is returned.
	 */
	public String getInputName() {
		if (valueInput != null) return valueInput.getName();
		return getName();
	}
	
	/**
	 * gets the value input column
	 * @return the value input column
	 */
	public IColumn getInput() {
		return valueInput;
	}
	
	/**
	 * Sets a value
	 * If there is a Column serving as Input for this Node, the set operation is delegated to the input column.
	 * Else the internal constant value is set. 
	 * @param value the constant value to set
	 */
	public void setValue(Object value) {
		super.setValue(value);
		if (getInput() != null) {
			if (getExplicitValueType() == null) setValueType(getInput().getValueType()); //set explicit value type of input if not set explicitly before.
			setInput(null);
		}
	}
	 
	/**
	 * Gets the value 
	 * If there is a Column serving as input for this proxy node, then the get Operation is delegated and the value delivered from the input column is returned.
	 * Else the internal value is returned.
	 */	
	public Object getValue() {
		return (valueInput != null) ? valueInput.getValue() : super.getValue();
	}
	
	/**
	 * Gets the value type (java class name) by delegation to the input
	 */
	public Class<?> getValueType() {
		if (getExplicitValueType() != null) return super.getValueType(); //type was set explicitly and may be different to the type of valueInput 
		if (valueInput != null) return valueInput.getValueType();
		return super.getValueType(); //return the default type of column, which is a String.
	}
	
	
	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof ColumnNode) {
			ColumnNode cn = (ColumnNode)source;
			setValueType(cn.getExplicitValueType());
			this.valueInput = cn.valueInput;
		}
	}
}
