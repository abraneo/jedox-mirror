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
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.scriptapi.APIExtender;
import com.jedox.etl.core.scriptapi.ScriptExecutor;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.etl.core.component.RuntimeException;

public abstract class Script extends Function {

	private ScriptExecutor executor;

	protected abstract String getScriptingName();
	
	protected abstract String getMessageText(RuntimeException e);
		
	protected Object transform(Row values) throws FunctionException {
		try {
			int count=1;
			for (IColumn column : values.getColumns()) {
				Object columnconvert = convert(column);
				executor.getEngine().put(column.getName(), columnconvert);
				// Generic aliases for inputs in Script: _input1, _input2,...
				executor.getEngine().put("_input"+Integer.toString(count), columnconvert);
				count=count+1;
			}
			
			Object result = executor.execute();
			if (result == null)
				result = executor.getEngine().get(getName());
			
			IColumn convertColumn = getOutputColumn();
			convertColumn.setValue(result);
			return TypeConversionUtil.convert(convertColumn);
		}
		catch (RuntimeException e) {
			throw new FunctionException("Unable to execute "+getScriptingName()+" function: "+getMessageText(e));
		}
	}
	
	protected String getScript() throws ConfigurationException {
		return getParameter("script","");
	}


	protected void validateParameters() throws ConfigurationException {
		try {
	        //make API available
	        executor = new ScriptExecutor(this,getScript(),getScriptingName());
		}
		catch (Exception e) {
			throw new ConfigurationException(e.getMessage());
		}
		IManager manager = APIExtender.getInstance().getGuessedDependencies(this, getScript());
		addManager(manager);
		String sources = getParameter("sources",null);
		if (sources != null) {
			for (String sourceName : sources.split(",")) {
				try {
					manager.add(getContext().getComponent(new Locator().add(getLocator().getRootName()).add(ITypes.Sources).add(sourceName.trim())));
				}
				catch (Exception e) {
					throw new ConfigurationException("Source "+sourceName.trim() + " is not a valid dependency: "+e.getMessage());
				}
			}
		}
	}
	
	public void close() {
		if (executor != null) {
			executor.close();
			executor = null;
		}
	}

}
