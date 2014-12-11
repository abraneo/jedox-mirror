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

import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.List;
import java.util.LinkedList;

import com.jedox.etl.core.component.RuntimeException;

/**
 * Holds and evaluates a set of conditions. Evaluation is delegated to the IEvaluator objects contained in each condition.  
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Conditions {
	
	private HashMap<String, LinkedList<Condition>> conditions;

	public static final String scriptKey = "#script";
	
	public static enum Modes {
		/**
		 * Accept the input, if evaluation returns true
		 */
		ACCEPT,
		/**
		 * deny / reject the input, if evaluation returns true
		 */
		DENY
	}
	
	/**
	 * Holds and evaluates a single condition. Evaluation is delegated to the IEvaluator objects contained in the condition.  
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public class Condition {
		private String  fieldName;
		private IEvaluator evaluator;
		private Modes mode = Modes.ACCEPT;
		
		/**
		 * gets the name of an input field this condition matches for
		 * @return the input field name
		 */
		public String getFieldName() {
			return fieldName;
		}
		/**
		 * gets the evaluator doing the basic evaluation for this condition 
		 * @return the evaluator
		 */
		public IEvaluator getEvaluator() {
			return evaluator;
		}
		/**
		 * gets the mode this condition operates in
		 * @return the operation mode
		 */
		public Modes getMode() {
			return mode;
		}
		
		/**
		 * evaluates if the given object matches the condition
		 * @param value the object to evaluate
		 * @return true, if the object matches, false otherwise
		 */
		public boolean evaluate(Object value) throws RuntimeException {
			return evaluator.evaluate(value);
		}
	}
	
	public Conditions() {
		conditions = new HashMap<String, LinkedList<Condition>>();
	}
	
	protected Condition createCondition() {
		return new Condition();
	}
	
	protected Condition setupCondition(String alias, String mode, IEvaluator evaluator) {
		Condition condition = createCondition();
		condition.fieldName = alias;
		condition.mode = Modes.valueOf(mode.toUpperCase());
		condition.evaluator = evaluator;
		return condition;
	}
	
	/**
	 * adds a condition.
	 * @param mode the {@link Modes mode} to operate the condition in.
	 * @param evaluator the evaluator to use for the condition
	 * @param fieldName the input field name this condition affects
	 * @return
	 */
	public Condition addCondition(String mode, IEvaluator evaluator, String fieldName) {
		Condition condition =  setupCondition(fieldName,mode,evaluator);
		LinkedList<Condition> list = conditions.get(fieldName);
		if (list == null) {
			list = new LinkedList<Condition>();
			conditions.put(fieldName, list);
		}
		list.add(condition);
		return condition;
	}
	
	/**
	 * evaluates all conditions in chained form for a given input field. Operates analogous to a firewall rule chain.
	 * @param fieldName the input field, which conditions have to affect 
	 * @param value the value to evaluate via the conditions
	 * @return true, if the value passes the 
	 */
	public boolean evaluate(String fieldName, Object value) throws RuntimeException {
		List<Condition> l = getCondition(fieldName);
		boolean accept = false;
		boolean deny = false;
		for (Condition condition : l) {
			boolean result = condition.evaluate(value);
			if (result) {
				if (condition.mode.equals(Modes.DENY)) {
					accept = false;
					deny = true;
				}
				else { 
					accept = true;
					deny = false;
				}
			}
		}
		return (accept && !deny);
	}
	
	/**
	 * gets all the input field names, where there are conditions defined for. 
	 * @return a set of input field names
	 */
	public Set<String> getFieldNames() {
		Set<String> result = new HashSet<String>();
		result.addAll(conditions.keySet());
		result.remove(scriptKey);
		return result;
	}
	
	protected List<Condition> getCondition(String key) {
		List<Condition> result = conditions.get(key);
		return (result != null) ? result : new LinkedList<Condition>();
	}

}
