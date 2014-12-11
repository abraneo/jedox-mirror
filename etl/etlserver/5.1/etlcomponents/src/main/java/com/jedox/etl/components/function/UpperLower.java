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
package com.jedox.etl.components.function;

import java.util.HashMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.node.Row;

public class UpperLower extends Function {
	
	private static final Log log = LogFactory.getLog(Lookup.class);
	
	private HashMap<String, String> lookup;
	
	private static enum CaseModes {
		UPPER, LOWER, FIRSTCASE, 
	}
	
	private CaseModes caseMode;
	private boolean verifyCase;
	
	/* Checks if in preceding rows a value has occurred which is case-insensitively equal to the input value (but not equal to it).
       In that case a Warning is raised and the first value which is case-insensitively equal is returned. 
       Otherwise the input value is returned.  
	 */
	private String getFirstCase(String input, boolean verifyCase) {
		if (lookup==null) {
			lookup=new HashMap<String,String>();
		}
		String result = lookup.get(input.toLowerCase());
		if (result!=null) {
			if (verifyCase && !result.equals(input))
				log.warn("In Function "+getName()+" the values "+result+" and "+input+" differ only in case.");
			return result;
		}	
		lookup.put(input.toLowerCase(), input);
		return input;
	}
	
	
	@Override
	protected Object transform (Row inputs) {
		String input = inputs.getColumn(0).getValueAsString();
		
		String firstCase="";
		if (caseMode.equals(CaseModes.FIRSTCASE) || verifyCase)
			firstCase=getFirstCase(input,verifyCase);

		switch (caseMode) {
			case UPPER: return input.toUpperCase();
			case LOWER: return input.toLowerCase();
			case FIRSTCASE: return firstCase;
			default: return "";
		}		
	}
	
	protected void validateParameters() throws ConfigurationException {
		String caseString = getParameter("case","");
		try { 
			caseMode = CaseModes.valueOf(caseString.toUpperCase());
		}
		catch (Exception e) {
			throw new ConfigurationException("Invalid parameter case: "+caseString);
		}	
		verifyCase = getParameter("verifycase","false").equalsIgnoreCase("true");
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,false);
	}
	

}
