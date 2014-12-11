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
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

import java.util.HashSet;

/**
 * Calculates an aggrgateion of its children
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Aggregation extends Function {
	
	private static enum Modes {
		MIN, AVG, COUNT, SUM, MAX, COUNTDISTINCT
	}
	
	private Modes mode;

	private double getWeight(int child) {
		return 1.0;
	}
	
	private double getDouble(IColumn column) throws FunctionException {
		Object val = column.getValue();
		double value = 0.0;
		try {
			if (val instanceof Double) {
				value = ((Double)val).doubleValue();
			}
			else if (val instanceof Integer) {
				value = ((Integer) val).doubleValue();
			}
			else {
				String val_string = val.toString();
				if(val_string.contains(",") && val_string.indexOf(',') != 0 && val_string.indexOf(',') != val_string.length()-1 
						&& val_string.indexOf(',')== val_string.lastIndexOf(',') ){
					val_string = val_string.replace(',', '.');// so that java will recognize numbers like 55,3 as a double value
				}
				value = Double.parseDouble(val_string);
			}
			return value;
		}
		catch (Exception e) {
			throw new FunctionException(column.getName(),column.getValueAsString(),e.getMessage());
		}
	}

	protected Object transform(Row values) throws FunctionException {
		int last = values.size();
		switch (mode) {
			case MIN: {
				if (last >= 0) { 
					double min = getDouble(values.getColumn(0)) * getWeight(0);
					for (int i=1;i<last;i++) {
						double value = getDouble(values.getColumn(i)) * getWeight(i);
						min = Math.min(min,value);
					}	
					return new Double(min);
				}
			}
			case MAX: {
				if (last >= 0) {
					double max = getDouble(values.getColumn(0)) * getWeight(0);
					for (int i=1;i<last;i++) {
						double value = getDouble(values.getColumn(i)) * getWeight(i);
						max = Math.max(max,value);
					}
					return new Double(max);
				}
			}
			case COUNT: {
				int i = 0;
				for (IColumn c : values.getColumns()) {
					if (c.getValue() != null) i++;
				}
				return new Double(i);
			}
			case COUNTDISTINCT: {
				HashSet<Object> set = new HashSet<Object>();
				for (IColumn c : values.getColumns()) {
					set.add(c);
				}
				return new Double(set.size());
			}
			case SUM: {
				double sum = 0.0;
				for (int i=0;i<last;i++) {
					double value = getDouble(values.getColumn(i)) * getWeight(i);
					sum += value;
				}
				return new Double(sum);
			}
			case AVG: {
				double sum = 0.0;
				for (int i=0;i<last;i++) {
					double value = getDouble(values.getColumn(i)) * getWeight(i);
					sum += value;
				}
				return new Double(sum/last);
			}
			default : return null;
		}
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,true);
	}
	
	protected void validateParameters() throws ConfigurationException {
		String modeString = getParameter("aggregate","NONE");
		try { 
			mode = Modes.valueOf(modeString.toUpperCase());
			setValueType(Double.class);
		}
		catch (Exception e) {
			throw new ConfigurationException("Parameter aggregate has to be set to either sum, min, max, avg or count");
		}	
	}

}
