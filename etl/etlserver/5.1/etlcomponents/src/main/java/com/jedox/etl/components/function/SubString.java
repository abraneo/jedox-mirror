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

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.node.Row;

public class SubString extends Function {
	
	private int beginindex;
	private int endindex;
	private String fromSide;
	private boolean negation;
	
	@Override
	protected Object transform (Row inputs) {
		String input = inputs.getColumn(0).getValueAsString();
		int length = input.length();
		int from;
		int to;
		if (fromSide.equals("left")) {
			from = (beginindex-1>length) ? length : beginindex-1;
			to = (endindex>length) ? length : endindex;
		}	
		else // orientation.equals("right")
		{
			from = (endindex>length) ? 0 : length-endindex;
			to = (beginindex-1>length) ? 0 : length-beginindex+1;
		}	
		
		if(negation == true){
			String result = "";
			if(from>0){
				result = input.substring(0, from);
			}
			if(to<length){
				result = result.concat(input.substring(to, length));
			}
			return result;
		}

		return input.substring(from, to);
			
	}
	
	protected void validateParameters() throws ConfigurationException {
		try { 
			String maximumEnd = ((Integer)Integer.MAX_VALUE).toString();
			beginindex = Integer.parseInt(getParameter("begin","1"));
			endindex = Integer.parseInt(getParameter("end",maximumEnd));
			fromSide = getParameter("fromSide", "left");
			negation = Boolean.parseBoolean(getParameter("negation", "false"));
			
			if(!(fromSide.equals("left") || fromSide.equals("right"))){
				throw new ConfigurationException("Parameter orientation can be set either to \"left\" or \"right\" ");
			}
		}
		catch (Exception e) {
			throw new ConfigurationException("Parameters have to be numerical");
		}
		if (beginindex < 1)
			throw new ConfigurationException("Parameter begin has to be positive");			
		if (endindex < 1)
			throw new ConfigurationException("Parameter end has to be positive");
		if (beginindex>endindex)
			throw new ConfigurationException("Parameter begin has to be less or equal than parameter end");		
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,false);
	}

}
