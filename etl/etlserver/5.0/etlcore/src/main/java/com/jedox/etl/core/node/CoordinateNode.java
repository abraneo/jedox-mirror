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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A {@link ColumnNode}, which acts as a coordinate proxy in a vector data format (e.g. for a cube, where each coordinate addresses one cube dimension)
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CoordinateNode extends ColumnNode {
	
	/**
	 * Type of aggregation, which should be performed on the values of this column with the help of the internal persistence 
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	protected static enum Aggregations {
		MIN, AVG, COUNT, SUM, MAX, NONE, FIRST, LAST
	}
	
	private Aggregations aggregate = Aggregations.NONE;
	private static final Log log = LogFactory.getLog(CoordinateNode.class);

	public CoordinateNode(String name) {
		super(name);
	}
	
	public ColumnTypes getColumnType() {
		return ColumnTypes.coordinate; 
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
			case SUM: setValueType("java.lang.Double"); break;
			case AVG: setValueType("java.lang.Double"); break;
			case COUNT: setValueType("java.lang.Double"); break;
			default: //do nothing for min/max/none, since also OK for strings
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
	
	/**
	 * Determines if this Column has an aggregate function name set. 
	 * @return true, if so
	 */
	public boolean hasAggregateFunction() {
		return !aggregate.equals(Aggregations.NONE);
	}
	
	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof CoordinateNode) {
			CoordinateNode n = (CoordinateNode)source;
			this.aggregate = n.aggregate;
		}
	}

}
