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

import com.jedox.etl.core.util.TypeConversionUtil;

/**
 * A {@link ColumnNode}, which acts as a value in a data vector (e.g. for a Cube)
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ValueNode extends ColumnNode {
	
	protected static enum Aggregations {
		MIN, AVG, COUNT, SUM, MAX, NONE, FIRST, LAST, GROUP_CONCAT, SELECTIVITY, STDDEV_SAMP, VAR_SAMP, STDDEV_POP, VAR_POP
	}

	private String target;
	private String valueTarget;
	private Operations operation = Operations.NONE;
	
	
	private Aggregations aggregate = Aggregations.NONE;
	private static final Log log = LogFactory.getLog(ValueNode.class);

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

	protected ValueNode() {
		super();
	}
	
	protected ValueNode(String name) {
		super(name);
	}

	public ValueNode(String name, String target) {
		super(name);
		setTarget(target);
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
		if ((value instanceof String) && getValueType().equals(Double.class)) {
			return TypeConversionUtil.convertToNumericString(value.toString());
		}
		return value;
	}
/*
	public String getValueAsString() {
		if (getElementType().equals(ElementType.ELEMENT_NUMERIC))
			return new TypeConversionUtil().convertToNumeric(super.getValueAsString());
		return super.getValueAsString();
	}

	public String getAggregateFunction() {
		if (getElementType().equals(ElementTypes.TEXT))
			return Aggregations.NONE.toString();
		return super.getAggregateFunction();
	}*/

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
			this.valueTarget = n.valueTarget;
			this.operation = n.operation;
			this.aggregate = n.aggregate;
		}
	}

	public void setValueTarget(String valueTarget) {
		this.valueTarget = valueTarget;
	}

	public String getValueTarget() {
		return valueTarget;
	}
	
	private String printAggregations() {
		StringBuffer output = new StringBuffer();
		for (Aggregations a : Aggregations.values()) {
			output.append(a.toString() +" ");
		}
		return output.toString();
	}
	
	/**
	 * sets an aggregate function indicator for this column. Since this column is a simple proxy, which can only handle one value at a time, the aggregation work has to be done externally by {@link com.jedox.etl.core.transform.ITransforms Transforms} using this column type 
	 * @param aggregate
	 */
	public void setAggregateFunction(String aggregate) {
		try {
			this.aggregate = Aggregations.valueOf(aggregate.toUpperCase());
			switch (this.aggregate) {
			case SUM: setValueType(Double.class); break;
			case AVG: setValueType(Double.class); break;
			case VAR_SAMP: setValueType(Double.class); break;
			case STDDEV_SAMP: setValueType(Double.class); break;
			case VAR_POP: setValueType(Double.class); break;
			case STDDEV_POP: setValueType(Double.class); break;
			case GROUP_CONCAT: setValueType(String.class); break;
			default: //do nothing for min/max/count/selectivity/none, since also OK for strings
			}
		}
		catch (IllegalArgumentException e) {
			log.warn("AggregateFunction has to be set to either "+printAggregations()+". No aggregation is set.");
			this.aggregate = Aggregations.NONE;
		}
	}
	
	/**
	 * gets the aggregate function name, which should be used to (externally) aggregate the values produced by this object in a time series 
	 * @return the aggregate function name
	 */
	public String getAggregateFunction() {
		return aggregate.toString();
	}

	public boolean isDistinctFunction() {
		return aggregate.equals(Aggregations.GROUP_CONCAT);
	}
	
	public boolean hasAggregateFunction() {
		return !aggregate.equals(Aggregations.NONE);
	}

}
