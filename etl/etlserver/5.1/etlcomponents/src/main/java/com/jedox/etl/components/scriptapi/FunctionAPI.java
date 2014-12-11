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
package com.jedox.etl.components.scriptapi;

import java.util.HashSet;
import java.util.Set;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.scriptapi.BaseAPI;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Enhancements of common Scripting API for usage in scripting functions.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class FunctionAPI extends BaseAPI {

	private IFunction function;
	private Set<String> usedSources = new HashSet<String>();
	private MessageHandler aggLog = new MessageHandler(log);	
	
	public boolean isUsable(IComponent component) {
		if (component instanceof IFunction && super.isUsable(component)) {
			function = (IFunction) component;
			return true;
		}
		return false;
	}

	/** gets the number of times a function has been calculated in the source.
	 * a calculation of the function is not necessary for identical input values 
	 * @return the number of calculations
	 */
	public int getCalculationCount() {
		return function.getCalculationCount();
	}
	
	/** gets the current number of the row in the source
	 * @return the row number
	 * @deprecated   	  
	 */
	public int getRowCount() {
		// As Script functions are buffered it is not called for every row, this can lead to wrong results for identical script inputs
		aggLog.warn("Method API.getRowCount() is deprecated. Use function RowNumber as input of the Script instead.");
		return function.getRowCount();
	}
	
	public IProcessor setupProcessor(String name, String format, int size) throws RuntimeException {
		IManager manager = function.getManager(ITypes.Any);
		if (manager != null && manager.get(name) == null) {
			log.debug("Function "+function.getName()+" implicitly depends on source "+name+" via script, but does not explicitly declare it.");
		}
		usedSources.add(name);
		IProcessor processor = super.setupProcessor(name, format, size);
		//getComponentContext().registerProcessor(processor);
		function.getUsedProcessorList().add(processor);
		return processor;
	}
	
	@SuppressWarnings("deprecation")
	public void close() {
		IManager manager = function.getManager(ITypes.Any);
		if (manager != null) {
			for (IComponent c : manager.getAll()) {
				if (!usedSources.contains(c.getName())) {
					log.debug("Function "+function.getName()+" declares unused dependency on source "+c.getName()+".");
				}
			}
		}
		super.close();
		function = null;
	}

}
