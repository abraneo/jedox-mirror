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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.execution;

import java.util.ArrayList;
import java.util.List;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.config.ConfigurationChangeEvent;
import com.jedox.etl.core.config.IChangeListener;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.ResultCodes.Codes;

/**
 * @author Kais Haddadin
 *
 */
public class ParallelExecutor implements IChangeListener {
	
	private static final Log log = LogFactory.getLog(ParallelExecutor.class);
	private ArrayList<Long> executionIds = new ArrayList<Long>();
	//private boolean finished =false;
	private IContext context = null;
		
	public void execute(IContext context,List<Execution> executions) {
		if (context.isExecutable()) {
			try {
				//finished =false;
				context.getState().addChangeListener(this);
				this.context = context;
				//first run all executables	 
				ResultCodes resultCode= new ResultCodes();
				for (Execution execution : executions) {
					if (context.isExecutable()) {
						Executor.getInstance().addExecution(execution);
						//log.info(execution.getExecutable().getContextName());			
						if(!(execution.isFailOnError() && (Codes.statusInvalid.equals(execution.getExecutionState().getStatus()) || execution.getExecutionState().getErrors() > 0))) {
							executionIds.add(execution.getKey());
							execution.addChangeListener(this); //this registers this class as change listener. we will get notified on finished. 
							Executor.getInstance().runExecution(execution.getKey());
							log.info("Starting " + execution.getName() + " in parallel execution (ID: " + execution.getKey() + ")");
						}else{
							log.error("Could not start " + execution.getName() + ", the status of the "+getType(execution.getExecutionState())+" is " + resultCode.getString(execution.getExecutionState().getStatus()));
							execution.abort();
						}
					}
				}
				
				// wait until all parallel execution are either finished or stopped
				/*while(!finished){
					Thread.sleep(2000);
				}*/
				synchronized(this){
					try{
						this.wait();
					}catch (InterruptedException  e) {
						log.info("The parallel executor thread is interrupted.");
					}
				}
				
				log.info("The parallel executor thread is finished successfully.");
			} 
			catch (Exception e) {
				log.error(e.getMessage());
			}
		}
	}
		
	private String getType(ExecutionState e) {
		return e.getType().substring(0,e.getType().length()-1);
	}

	@Override
	public void configurationChanged(ConfigurationChangeEvent e) {
		// ignore
	}

	@Override
	public synchronized void executionFinished(ExecutionState state) {
		if(executionIds.contains(state.getId())){
			synchronized (executionIds){
			ResultCodes resultCode= new ResultCodes();
			String type = state.getType() ;
			String name = state.getName();
			String statusMsg = "Finished " + type + " "+ name + " with status: " + resultCode.getString(state.getStatus());
				if(state.getStatus().equals(Codes.statusWarnings)){
					//context.getState().addErrors(e.getErrors());
					//context.getState().addWarnings(e.getWarnings());
					this.context.getState().addMessageExternal("WARN",statusMsg );
				}else if(state.getStatus().equals(Codes.statusAborted) || state.getStatus().equals(Codes.statusErrors)|| 
						state.getStatus().equals(Codes.statusFailed) || state.getStatus().equals(Codes.statusInvalid)){
					this.context.getState().addMessageExternal("ERROR",statusMsg );
				}else{
					this.context.getState().addMessageExternal("INFO",statusMsg );
				}
				executionIds.remove(state.getId());
				if(executionIds.isEmpty())
					this.notify();
			}
		}
	}

	@Override
	public void executionStopping(ExecutionState state) {
	try{
		@SuppressWarnings("unchecked")
		ArrayList<Long> executionIdsToBeStopped = (ArrayList<Long>) executionIds.clone();
		for (long id : executionIdsToBeStopped) {
			//try to stop the parallel executions
			ExecutionState e = Executor.getInstance().getExecutionState(id, false);
			if(Codes.statusQueued.equals(e.getStatus()) || (Codes.statusRunning.equals(e.getStatus()))) {
				this.context.getState().addMessageExternal("INFO", getType(e) + " "+ e.getName() + " with will be stopped.");
				Executor.getInstance().stop(id);
			}
		}
	} 
	catch (Exception e) {
		this.context.getState().addMessageExternal("ERROR",e.getMessage());
	}
		
	}	
}
