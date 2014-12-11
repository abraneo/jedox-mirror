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

import java.util.regex.Pattern;
//import com.jedox.etl.core.source.filter.AlphaRangeEvaluator.*;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
/**
 * Evaluates if a number lies within a specific numeric range  
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class RangeEvaluator implements IEvaluator {

	private Pattern rangePattern;
	private double startRange = 0;
	private double endRange = -1;
	private boolean includeStart = true;
	private boolean includeEnd = true;
	//private static final Log log = LogFactory.getLog(RangeEvaluator.class);
	
	public RangeEvaluator(String definition) throws ConfigurationException {
		if (definition==null)
			throw new ConfigurationException("Missing value in Expression.");
		String regex = "(\\[|\\()(\\-?\\d*\\.?\\d*),(\\-?\\d*\\.?\\d*)(\\]|\\))";
		rangePattern = Pattern.compile(regex);
		if (rangePattern.matcher(definition).matches() && !definition.equals("[,]")) {
			String startString = definition.split(",")[0];
			String startMode = startString.substring(0,1);
			startString = startString.substring(1);
			String endString = definition.split(",")[1];
			String endMode = endString.substring(endString.length()-1, endString.length());
			endString = endString.substring(0,endString.length()-1);
			startRange = Double.parseDouble(startString);
			endRange = Double.parseDouble(endString);
			if (startMode.equals("(")) includeStart = false;
			if (endMode.equals(")")) includeEnd = false;
			if(includeStart && includeEnd && startRange>endRange){
				throw new ConfigurationException("Range start should be less or equal to the range end.");
			}
		}
		else {
			try {
				new AlphaRangeEvaluator(definition);
			} catch (Exception e) {
				throw new ConfigurationException("Range definition is not valid: "+definition+". The format of a range is: [from,to]");
			}
			throw new ConfigurationException("Range definition "+definition+" contains non-numerical values");
			//log.error("Range definition is not valid: "+definition);
		}	
	}
	
	private boolean evaluateStart(Double value) {
		if (includeStart) return (startRange <= value);
		return (startRange < value);
	}
	
	private boolean evaluateEnd(Double value) {
		if (includeEnd) return (value <= endRange);
		return (value < endRange);
	}
	
	private boolean evaluate(Double value) {
		return evaluateStart(value) && evaluateEnd(value);
	}
	
	public double getStart() {
		return startRange;
	}
	
	public double getEnd() {
		return endRange;
	}
	
	public boolean evaluate(Object value) {
		if (value == null)
			return false;
		if (value instanceof Double)
			return evaluate((Double)value);
		try {
			double v = Double.parseDouble(value.toString());
			return evaluate(v);
		}
		catch (NumberFormatException e) {
			//log.warn("Failed to convert "+value+" to a number. Is ignored.");
		}
		return false;
	}
	
}
