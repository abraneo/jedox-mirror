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
import com.jedox.etl.core.util.TypeConversionUtil;

public class Random extends Function {

	private int minimum = Integer.MIN_VALUE;
	private int maximum = Integer.MAX_VALUE;
	private java.util.Random rand = new java.util.Random();
	
	@Override
	protected Object transform (Row inputs) throws FunctionException {
		 try {
			 IColumn convertColumn = getOutputColumn();
			// convertColumn.setValue((rand.nextInt(maximum-minimum+1))+minimum);
			 convertColumn.setValue((rand.nextDouble()*((double)maximum-(double)minimum))+minimum);
			 return TypeConversionUtil.convert(convertColumn);
		 }
		 catch (Exception e) {
			 throw new FunctionException(e);
		 }
	}
	
	public void validateInputs() throws ConfigurationException {
		try { 
			String minimumEnd = ((Integer)Integer.MIN_VALUE).toString();
			String maximumEnd = ((Integer)Integer.MAX_VALUE).toString();
			minimum = Integer.parseInt(getParameter("minimum",minimumEnd));
			maximum = Integer.parseInt(getParameter("maximum",maximumEnd));		}
		catch (Exception e) {
			throw new ConfigurationException("Parameters have to be numerical");
		}
		
		setValueType(getClassnameForType(getParameter("type","String")));
	}

}
