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
package com.jedox.etl.components.job;

import java.util.List;


import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.job.ScriptJobConfigurator;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.scriptapi.APIExtender;
import com.jedox.etl.core.scriptapi.ScriptExecutor;

public abstract class Script extends Component implements IJob {

	private static final Log log = LogFactory.getLog(Script.class);

	public Script() {
		setConfigurator(new ScriptJobConfigurator());
	}

	public ScriptJobConfigurator getConfigurator() {
		return (ScriptJobConfigurator) super.getConfigurator();
	}

	protected abstract String getScriptingName();

	protected String getMessageText(Exception e) {
		return e.getCause().getMessage().substring(e.getCause().getMessage().indexOf(":")+1);
	}
	
	protected List<String> getScripts() {
		return getConfigurator().getScripts();
	}
		
	public void execute() {
		if (isExecutable()) {
			log.debug("Starting job: " + getName());
			try {
				// run all defined scripts
				for (String s : getScripts()) {
					ScriptExecutor e = new ScriptExecutor(this, s, getScriptingName());
					e.execute();
					e.close();
				}
			} catch (Exception e) {
				log.error("Unable to execute "+getScriptingName()+" job: "+getMessageText(e));
			} finally {
				log.debug("Finishing job " + getName() + ".");
			}
		}
	}

	/*
	 * to be done for scripts public void test() throws RuntimeException {
	 * super.test(); }
	 */

	public void init() throws InitializationException {
		super.init();
		try {
			IManager manager = getConfigurator().getDeclaredDependencies();
			for (IComponent c : APIExtender.getInstance().getGuessedDependencies(this, getScripts()).manager.getAll()) {
				manager.add(c);
			}
			addManager(manager);
		} catch (Exception e) {
			throw new InitializationException(e);
		}
	}
	
	public boolean isExecutable() {
		return getContext().isExecutable();
	}
	
	public boolean isParallel() {
		return false;
	}	
}
