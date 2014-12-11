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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source.filter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.scriptapi.ScriptExecutor;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.etl.core.component.RuntimeException;

public class ScriptEvaluator implements IEvaluator {
	
	private ScriptExecutor executor;
	private static final Log log = LogFactory.getLog(ScriptEvaluator.class);	
	
	public ScriptEvaluator(IContext context, String script, String engineName) {
		executor = new ScriptExecutor(context,script,engineName);
	}

	public boolean evaluate(Object expression) throws RuntimeException {
		int count=1;
		if (expression instanceof Row) {
			Row row = (Row) expression;
			for (IColumn column : row.getColumns()) {
				Object columnconvert;
				try {
					columnconvert = TypeConversionUtil.convert(column);
				} catch (Exception e) {
					columnconvert = column;
					log.debug("Conversion problem in script: "+e.getMessage());
				}
				executor.getEngine().put(column.getName(), columnconvert);
				// Generic aliases for inputs in Script: _input1, _input2,...
				executor.getEngine().put("_input"+Integer.toString(count), columnconvert);
				count=count+1;
			}
		} else {
			executor.getEngine().put("VALUE", expression);
		}
		Object result = null;
		result = executor.execute();
		if (result instanceof Boolean) {
			return ((Boolean) result).booleanValue();
		}
		if (result instanceof String) {
			return ((String) result).equals("1");
		}
		if (result instanceof Number) {
			return ((Number) result).equals(1);
		}
		return result != null;
	}

}
