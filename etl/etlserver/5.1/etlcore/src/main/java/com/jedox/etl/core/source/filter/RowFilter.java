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
package com.jedox.etl.core.source.filter;


import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import java.util.List;
import java.util.ArrayList;

/**
 * Filter, which may impose a chain of conditions on every column of a row and finally evaluates if a the row of data is to be accepted or rejected dependent on the junction of evaluations of these conditions. junction may be a conjunction or a disjunction.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class RowFilter {

	public static String AND = "AND";
	public static String OR = "OR";

	public static enum Operators {
		equal, like, between, alpha_range, inRange, inAlphaRange, isEmpty, subset, script
	}

	private Conditions conditions = new Conditions();

	private Log aggLog = new MessageHandler(LogFactory.getLog(RowFilter.class));
	private String combination = AND;
	private List<IColumn> filterColumns;
 
	public RowFilter(String combination) {
		this.combination = combination;
	}

	private IColumn getColumn(Row row, String name) {
		IColumn column = row.getColumn(name);
		if (column == null) {
			aggLog.error("Filter references to unknown alias and thus is ignored: "+name);
		}
		return column;
	}
	
	private boolean evaluateConditions(Row row) throws Exception {		
	 	
	 // todo: check problem with global filterColumns (#14166) 	

		if (combination.equalsIgnoreCase(OR)) {
			for (IColumn column :  filterColumns) {
				String value = column.getValueAsString();
				boolean result = getConditions().evaluate(column.getName(), value);
				if (result) return true;
			}
			return false;
		}
		else { //AND
			for (IColumn column :  filterColumns) {
				String value = column.getValueAsString();
				boolean result = getConditions().evaluate(column.getName(), value);
				if (!result) return false;
			}
			return true;
		}
	}
	
	private boolean evaluateScript(Row row) throws Exception {
		if (!getConditions().getCondition(Conditions.scriptKey).isEmpty()) //there are script conditions
			return getConditions().evaluate(Conditions.scriptKey, row);
		else //there are no script conditions. return true in mode AND and false in mode OR
			return combination.equalsIgnoreCase(AND);
	}

	/**
	 * evaluates if the given row should be accepted or rejected.
	 * @param row the row to evaluate
	 * @return true, if the row should be accepted, false otherwise
	 */
	public boolean evaluate(Row row) {
		try {
			if (combination.equalsIgnoreCase(OR))
				return evaluateConditions(row) || evaluateScript(row);
			else
				return evaluateConditions(row) && evaluateScript(row);
		} catch (Exception e) {
			aggLog.error("Failed to evaluate filter expression: "+e.getMessage());
			aggLog.debug(e);
		}
		return true;
	}

	/**
	 * gets the conditions imposed on this row filter.
	 * @return
	 */
	public Conditions getConditions() {
		return conditions;
	}
	
	public void init(Row row) {
		filterColumns = new ArrayList<IColumn>();
		for (String key :  getConditions().getFieldNames()) {
			IColumn column = getColumn(row,key);
			if (column != null) filterColumns.add(column);
		}
	}

}
