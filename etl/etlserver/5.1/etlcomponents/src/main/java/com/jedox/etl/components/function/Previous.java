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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*/
package com.jedox.etl.components.function;

import java.util.HashMap;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.node.Row;

public class Previous extends Function {
	
	private int offset;
	private String defaultValue;
	
	private HashMap<Integer,Object> lastValues = new HashMap<Integer,Object>();
	
	@Override
	protected Object transform (Row inputs) {	
	    Object value = inputs.getColumn(0).getValue();
		// Keep the value type of the input column
		setValueType(inputs.getColumn(0).getValueType());
		
		int rowCount = getRowCount();
		Object lastValue = lastValues.get(rowCount-offset);		
		lastValues.put(rowCount, value);
		lastValues.remove(rowCount-offset-1);
		
		if (lastValue==null)
			return defaultValue;
		else
			return lastValue;
	}
	
	protected void validateParameters() throws ConfigurationException {
		try { 
			offset = Integer.parseInt(getParameter("offset","1"));
		}
		catch (Exception e) {
			throw new ConfigurationException("Parameter offset has to be numerical");
		}
		if (offset<=0)
			throw new ConfigurationException("Parameter offset has to be positive");			
		defaultValue = getParameter("default","");
		
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,false);
	}

}
