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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.function;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

/**
 * Interface for Transformers o implement.
 * A Transformer is a Sub-Component of a {@link com.jedox.etl.core.transform.ITransform Pipeline}, which is able to accept one or more columns of data as input and delivers one output based on the implemented transformation. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IFunction extends IColumn, IComponent {

	/**
	 * Gets the input Columns of this Transformer as a Row.
	 * @return the Row holding the input columns.
	 */
	public Row getInputs();
	
	/**
	 * Gets the number of actual dispatches to a calculation. This number may be less than the number of input rows processed due to internal caching
	 * @return the number of calculations done.
	 */
	public int getCalculationCount();	
	
	/**
	 * Allows to set an external row count column of an input processor, making the row count of this processor accessible. 
	 * @param rowCountColumn the column holding the row count. Has to be of ValueType Integer.
	 */
	public void setRowCountColumn(IColumn rowCountColumn);
	
	/**
	 * gets the row count of the input processor for this function.
	 * @return
	 */
	public int getRowCount();
	
	/**
	 * closes and frees all resources used by this function
	 */
	public void close();

}
