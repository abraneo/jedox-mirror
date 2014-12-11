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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.scriptapi;

import java.util.Map;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.component.RuntimeException;

public class ScriptExecutor {
	
	private ScriptEngine engine;
	private String script;
	private Map<String,IScriptAPI> apis;
	
	private static Log log = LogFactory.getLog(ScriptExecutor.class);
	
	public ScriptExecutor(IComponent component, String script, String engineName) {
		IContext context = (component instanceof IContext) ? (IContext)component : component.getContext();
		this.script = script;
		apis = APIExtender.getInstance().getAPIExtensions(component);
		for (IScriptAPI api : apis.values()) {
			api.setProperties(context.getParameter());
			api.setProperties(context.getVariables());
		}
		ScriptEngineManager factory = new ScriptEngineManager();
	    engine = factory.getEngineByName(engineName);
	}
	
	public ScriptEngine getEngine() {
		return engine;
	}

	public Object execute() throws RuntimeException {
		try {
			for (IScriptAPI api : apis.values()) {
				engine.put(api.getExtensionPoint(), api);
			}
			engine.put("LOG", log);
			Object result = engine.eval(script);
			return result;
		} catch (ScriptException e) {
			throw new RuntimeException(e);
		}
	}
	
	public void close() {
		for (IScriptAPI api : apis.values()) {
			api.close();
		}
	}

}
