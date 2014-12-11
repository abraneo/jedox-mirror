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
package com.jedox.etl.components.job;

import java.util.List;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.job.DefaultJobConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.job.Job;
import com.jedox.etl.core.job.JobManager;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.LoadManager;
import com.jedox.etl.core.scriptapi.ScriptExecutor;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Default extends Job {
	
	private static final Log log = LogFactory.getLog(Default.class);
	
	private List<IExecutable> executables;
		
	public Default() {
		setConfigurator(new DefaultJobConfigurator());
	}
	
	public DefaultJobConfigurator getConfigurator() {
		return (DefaultJobConfigurator)super.getConfigurator();
	}
	
	protected LoadManager getLoadManager() {
		return (LoadManager)getManager(ITypes.Loads);
	}
	
	protected JobManager getJobManager() {
		return (JobManager)getManager(ITypes.Jobs);
	}
	
	public void execute() {
		if (isExecutable()) {
			log.debug("Starting job: "+getName());
			try {
				//first run all executables
				for (IExecutable e : executables) {
					if (e instanceof IJob) {
						if (isExecutable()) {
							//give all sub-jobs their own environment
							String syncMode = XMLUtil.parseProperties(e.getConfigurator().getXML().getChildren("variable")).getProperty(NamingUtil.internal("syncMode"));
							if ( SyncModes.PARALLEL.toString().equalsIgnoreCase(syncMode)) {
								log.warn("Subjob "+e.getLocator().getName()+" specifies syncMode "+syncMode+" , which will be ignored. Subjobs are always executed serially.");
							}
							//push Variables top down, except for default context variable values, which may be overwritten anywhere at deeper level.
							Properties subJobEnv = new Properties();
							subJobEnv.putAll(e.getContext().getExternalVariables());
							Properties ownVariables = e.getConfigurator().getOwnVariables();
							for (Object key : ownVariables.keySet()) {
								if (!subJobEnv.containsKey(key)) subJobEnv.put(key, ownVariables.get(key));
							}
							Execution execution = Executor.getInstance().createExecution(e.getLocator(), subJobEnv,this.getState());
							execution.execute(Thread.currentThread().getName());
							if (execution.isFailOnError() && (Codes.statusInvalid.equals(execution.getExecutionState().getStatus()) || execution.getExecutionState().getErrors() > 0)) break;
						}
					}
					else { //execute loads directly in own environment
						if (isExecutable()) e.execute();
					}
				}
				// run all defined scripts, if job is a script job.
				for (String s : getConfigurator().getScripts()) {
					ScriptExecutor e = new ScriptExecutor(this,s,"groovy");
					e.execute();
					e.close();
				}	
			} 
			catch (Exception e) {
				log.error(e.getMessage());
			}
			finally {
				log.debug("Finishing job "+getName()+".");
			}
		}
	}
		
	public void test() throws RuntimeException {
		super.test();
		for (ILoad load : getLoadManager().getAll()) {
			load.test();
		}
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new LoadManager());
			addManager(new JobManager());
			executables = getConfigurator().getExecutables();
			for (IExecutable e : executables) {
				if (e instanceof IJob) {
					getJobManager().add(e);
				} 
				else if (e instanceof ILoad) {
					getLoadManager().add(e);
				}
			}
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
