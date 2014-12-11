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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.function;

import java.util.StringTokenizer;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.NamingUtil;

public class Split extends Function {
	
	private String separator;
	private int occurence;
	private String defaultValue;
	
	@Override
	protected Object transform (Row inputs) {
		String input = inputs.getColumn(0).getValueAsString();
		// split() from java.lang.String not used because it's based on regular expressions. Here just Strings are used as separators	
		StringTokenizer stringtokenizer = new StringTokenizer(input, separator);			
		for (int i=1; stringtokenizer.hasMoreElements() && i<=occurence; i++) {
			String s=stringtokenizer.nextToken();
			if (i==occurence)
				return s;				
		}
		return defaultValue;
	}
	
	protected void validateParameters() throws ConfigurationException {
		separator = getParameter("separator", "").replaceAll(NamingUtil.spaceValue(), " ");
		defaultValue = getParameter("default", "");
		occurence = Integer.parseInt(getParameter("occurence","1"));		
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,false);
	}

}
