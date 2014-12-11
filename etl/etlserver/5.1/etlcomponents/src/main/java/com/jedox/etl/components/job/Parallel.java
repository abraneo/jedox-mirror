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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.job.ParallelJobConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.execution.ParallelExecutor;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.util.NamingUtil;

public class Parallel extends Default implements IJob {
	
	private static final Log log = LogFactory.getLog(Parallel.class);
			
	public Parallel() {
		setConfigurator(new ParallelJobConfigurator());
	}
	
	public ParallelJobConfigurator getConfigurator() {
		return (ParallelJobConfigurator)super.getConfigurator();
	}
	
	public void execute() {
		if (isExecutable()) {
			log.debug("Starting job: "+getName());
			try {
				//build executions for jobs now, unless we would suffer from config changes during long-running executions
				Map<IExecutable,Execution> executionLookup = new HashMap<IExecutable, Execution>();
				for (IExecutable e : executables) {
					if (e instanceof IJob || getConfigurator().isParallel(e)) {
						//push Variables top down, except for default context variable values, which may be overwritten anywhere at deeper level.
						Properties subJobEnv = new Properties();
						subJobEnv.putAll(e.getContext().getExternalVariables());
						Properties ownVariables = e.getConfigurator().getOwnVariables();
						for (Object key : ownVariables.keySet()) {
							if (!subJobEnv.containsKey(key)) subJobEnv.put(key, ownVariables.get(key));
						}
						Execution execution = null;
						if( getConfigurator().isParallel(e)){
							subJobEnv.setProperty(NamingUtil.internal("syncMode"), SyncModes.PARALLEL.toString());
							execution = Executor.getInstance().createExecution(e.getLocator(), subJobEnv);
						}
						else{
							subJobEnv.setProperty(NamingUtil.internal("syncMode"), SyncModes.SINGLE.toString());
							execution = Executor.getInstance().createExecution(e.getLocator(), subJobEnv,getContext().getState());
						}
				
						executionLookup.put(e, execution);
					}
				}
				//execute all executions / executables in given order
				for (int i=0;i<executables.size();i++) {
					
					IExecutable e = executables.get(i);
					if(isExecutable() &&  getConfigurator().isParallel(e)){
						ParallelExecutor executor = new ParallelExecutor();
						List<Execution> parallelSet = new ArrayList<Execution>();
						e = executables.get(i);
						do{
							parallelSet.add(executionLookup.remove(e));
							if((i+1)<executables.size()){
								e = executables.get(i+1);
								if(! getConfigurator().isParallel(e))
									break;
							}
							else
								break;
							i++;
						}while(true);

						executor.execute(getContext(), parallelSet);	
					}else if (e instanceof IJob) {
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

	/**
	 * @param e
	 * @return
	 */

		
	public boolean isParallel() {
		return true;
	}
	
	public void init() throws InitializationException {
		super.init();
		// Set syncMode to Parallel		
		getContext().getParameter().setProperty(NamingUtil.internal("syncMode"),SyncModes.PARALLEL.toString());			
	}
	
		
}
