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
package com.jedox.etl.core.node;

import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

/**
 * A {@link ColumnNode}, which acts as a value in a data vector (e.g. for a Cube)
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ValueNode extends CoordinateNode {

	private String target;
	private Operations operation = Operations.NONE;

	/**
	 * Operations available on ValueNodes.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum Operations {
		/**
		 * The ValueNode should be normalized in processing, so that exactly one value is in a output line
		 */
		NORMALIZE,
		/**
		 * The ValueNode should be denormalized in processing, so that all values sharing the same coordinates should be in a single output line.
		 */
		DENORMALIZE,
		/**
		 * No operation is done on this ValueNode
		 */
		NONE
	}

	public ValueNode(String name) {
		super(name);
	}

	public ValueNode(String name, String target) {
		super(name);
		setTarget(target);
	}

	public ColumnTypes getColumnType() {
		return ColumnTypes.value;
	}

	/**
	 * Sets the name of the {@link CoordinateNode}, this ValueNode refers to for normalization and denormalization
	 * @param target the name of the target CoordinateNode
	 */
	public void setTarget(String target) {
		this.target = target;
	}

	/**
	 * Gets the name of the Target CoordinateNode. See {@link #setTarget(String)}
	 * @return the name.
	 */
	public String getTarget() {
		return target;
	}

	public Object getValue() {
		Object value = super.getValue();
		if ((value instanceof String) && getElementType().equals(ElementType.ELEMENT_NUMERIC)) {
			return new TypeConversionUtil().convertToNumeric(value.toString());
		}
		return value;
	}

	public String getValueAsString() {
		if (getElementType().equals(ElementType.ELEMENT_NUMERIC))
			return new TypeConversionUtil().convertToNumeric(super.getValueAsString());
		return super.getValueAsString();
	}

	/*public String getAggregateFunction() {
		if (getElementType().equals(ElementTypes.TEXT))
			return Aggregations.NONE.toString();
		return super.getAggregateFunction();
	}*/

	public String getValueType() {
		if (getElementType().equals(ElementType.ELEMENT_NUMERIC))
			return Double.class.getCanonicalName();
		else 
			return String.class.getCanonicalName(); 
	}

	/**
	 * sets the operation intended to be performed on this ValueNode on processing
	 * @param operation the operation to be performed
	 */
	public void setOperation(Operations operation) {
		this.operation = operation;
	}

	/**
	 * gets the operation intended to be performed on this ValueNode on processing
	 * @return the operation
	 */
	public Operations getOperation() {
		return operation;
	}

	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof ValueNode) {
			ValueNode n = (ValueNode)source;
			this.target = n.target;
			this.operation = n.operation;
		}
	}

}
