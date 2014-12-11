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
package com.jedox.etl.components.job;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.List;
import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.job.DefaultJobConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.job.JobManager;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.LoadManager;
import com.jedox.etl.core.util.NamingUtil;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Default extends Component implements IJob {
	
	private static final Log log = LogFactory.getLog(Default.class);
	
	protected List<IExecutable> executables = new ArrayList<IExecutable>();
		
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
				//build executions for jobs now, unless we would suffer from config changes during long-running executions
				Map<IExecutable,Execution> executionLookup = new HashMap<IExecutable, Execution>();
				for (IExecutable e : executables) {
					if (e instanceof IJob) {
						//push Variables top down, except for default context variable values, which may be overwritten anywhere at deeper level.
						Properties subJobEnv = new Properties();
						subJobEnv.putAll(e.getContext().getExternalVariables());
						Properties ownVariables = e.getConfigurator().getOwnVariables();
						for (Object key : ownVariables.keySet()) {
							if (!subJobEnv.containsKey(key)) subJobEnv.put(key, ownVariables.get(key));
						}
						Execution execution = null;
						
						subJobEnv.setProperty(NamingUtil.internal("syncMode"), SyncModes.SINGLE.toString());
						execution = Executor.getInstance().createExecution(e.getLocator(), subJobEnv,getContext().getState());
				
						executionLookup.put(e, execution);
					}
				}
				//execute all executions / executables in given order
				for (int i=0;i<executables.size();i++) {
					
					IExecutable e = executables.get(i);
					if (e instanceof IJob) {
						if (isExecutable()) {
							Execution execution = executionLookup.remove(e);
							execution.execute(Thread.currentThread().getName());
							if (execution.isFailOnError() && (Codes.statusInvalid.equals(execution.getExecutionState().getStatus()) || execution.getExecutionState().getErrors() > 0)) break;
						}
					}
					else { //execute loads directly in own environment
						if (isExecutable()) e.execute();
					}
				}
				//free all resources of executions nor executed because some previous execution failed
				for (Execution e : executionLookup.values()) {
					//e.getExecutable().getState().setExecutable(false);
					e.abort();
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
		for (IJob job : getJobManager().getAll()) {
			job.test();
		}		
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new LoadManager());
			addManager(new JobManager());
			LinkedList<Locator> locators = getConfigurator().getLocators();
			
			for (Locator loc : locators) {
				IExecutable e = (IExecutable)getContext().getComponent(loc);						
				executables.add(e);
				if (e instanceof IJob) {
					getJobManager().add(e);					
				} 
				else if (e instanceof ILoad) {
					getLoadManager().add(e);
				}
			}
			// Jobs with a parallel type cannot be included in standard job
			if (!isParallel()) {
				for (IJob job : getJobManager().getAll()) {
					if (job.isParallel())
						throw new ConfigurationException("The parallel job "+job.getName()+" cannot be a subjob of job "+getName());
				}	
			}
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
		
		
	}

	@Override
	public boolean isExecutable() {
		return getContext().isExecutable();
	}
	
	public boolean isParallel() {
		return false;
	}
	
}
