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

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;


/**
 * Evaluates if an alphanumeric expression lies within a specific range  
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class AlphaRangeEvaluator implements IEvaluator {
	
	private String startString;
	private String endString;
	private boolean includeStart = true;
	private boolean includeEnd = true;
	//private static final Log log = LogFactory.getLog(AlphaRange.class);
	
	public AlphaRangeEvaluator(String definition) throws ConfigurationException {
		if (definition==null)
			throw new ConfigurationException("Missing value in Expression.");		
		String regex = "(\\[|\\()(\\w|[\\.;+\\- ])*,(\\w|[\\.;+\\- ])*(\\]|\\))";
		Pattern rangePattern = Pattern.compile(regex);
		if (rangePattern.matcher(definition).matches() && !definition.equals("[,]")) {
			startString = definition.split(",")[0];
			String startMode = startString.substring(0,1);
			startString = startString.substring(1);
			endString = definition.split(",")[1];
			String endMode = endString.substring(endString.length()-1, endString.length());
			endString = endString.substring(0,endString.length()-1);
			if (startMode.equals("(")) includeStart = false;
			if (endMode.equals(")")) includeEnd = false;
		}
		else 
			throw new ConfigurationException("Alphanumeric range definition is not valid: "+definition);
			//log.error("Alphanumeric range definition is not valid: "+definition);
	}
	
	private boolean evaluateStart(String value) {
		return (includeStart) ? (startString.compareToIgnoreCase(value) <= 0) : (startString.compareToIgnoreCase(value) < 0);
	}
	
	private boolean evaluateEnd(String value) {
		return (includeEnd) ? (endString.compareToIgnoreCase(value) >= 0) : (endString.compareToIgnoreCase(value) > 0);
	}
	
	public boolean evaluate(Object value) {
		return evaluateStart(value.toString()) && evaluateEnd(value.toString());
	}

}
