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
package com.jedox.etl.core.execution;

import java.util.Hashtable;
import java.util.Properties;
import java.util.ArrayList;
import java.util.concurrent.Future;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigurationChangeEvent;
import com.jedox.etl.core.config.IChangeListener;
import com.jedox.etl.core.execution.Execution.ExecutionTypes;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ResultCodes.DataTargets;


/**
 * Class for the creation, management and execution of {@link Execution Executions}. The executor also takes care of the potentially necessary synchronization between executions.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

public class Executor implements IExecutor, IChangeListener {

	private static final Log log = LogFactory.getLog(Executor.class);
	private Hashtable<Long, Execution> executions = new Hashtable<Long, Execution>();
	private static final Executor instance = new Executor();
	private boolean isExecutable = true;
	private ExecutionController controller = new ExecutionController();
	

	Executor() {
	}

	public static final Executor getInstance() {
		return instance;
	}
	
	public void addExecution(Execution execution) throws ExecutionException {
		//ResultManager.getInstance().save(execution.getExecutionState());
		executions.put(execution.getKey(), execution);
	}

	public ExecutionState removeExecution(Long id, boolean removeFromHistory) throws ExecutionException {
		ExecutionState state = null;
		Execution e = null;
		try {
			e = getExecution(id);
			switch (e.getExecutionState().getStatus()) {
			case statusStopping:
				// so interrupt the execution
				try {
					e.getExecutionState().setStatus(Codes.statusInterrupting);
					e.getExecutionState().addMessageExternal("INFO", "Requested interrupt of execution (ID: "+e.getKey()+")");
					// try to cleanup execution e.g. in order to close all connections					
					e.cleanUp();  
					Thread.sleep(500); // wait in order to let cleanup finish correctly (check if really necessary)					
				}	
				catch (Exception ex) {
					log.error("Failed to cleanup execution "+e.getKey()+": "+ex.getMessage());
				}

				e.interrupt();
				// kill the thread if still not finished
				Future<?> f = controller.getFuture(e.getKey());
				if (f != null) {
					f.cancel(true);
				}
				break;
				
			case statusRunning: 
				e.getExecutionState().addMessageExternal("INFO", "Requested stop of execution (ID: "+e.getKey()+")");
				e.stop(); 
				break;
				
			case statusQueued: 
				e.getExecutionState().addMessageExternal("INFO", "Requested abort of execution (ID: "+e.getKey()+")");
				e.abort(); 
				break;
				
			default:
			}

			controller.remove(e);
			state = getStatus(e,false);
		}
		catch (ExecutionException ex) {
			if (!removeFromHistory) throw new ExecutionException("Execution can not be stoppped, only execution with status \"Running\", \"Queued\" or \"Stopping\" can be stopped.");
		}
		finally {
			if (removeFromHistory) {
				try {
					//remove state from result manager, even if this execution has already been run.
					ExecutionState historicState = StateManager.getInstance().getResult(id);
					if (historicState != null) {
						StateManager.getInstance().remove(historicState);
						if (state == null)
							state = historicState;
				}
				}
				catch (RuntimeException ex) {
					throw new ExecutionException(ex);
				}
			}
			if (e == null)
				throw new ExecutionException("Execution with id "+id+" not found.");
		}
		return state;
	}

	private Execution getExecution(Long id) throws ExecutionException {
		if (id == null)
			throw new ExecutionException("Execution id must not be null.");
		Execution e = executions.get(id);
		if (e == null)
			throw new ExecutionException("Execution with id "+id+" not found.");
		else
			return e;
	}

	public Execution runExecution(Long id) throws ExecutionException {
		Execution e = getExecution(id);
		e.addChangeListener(this);
		if ((isExecutable) && !Codes.statusInvalid.equals(e.getExecutionState().getStatus())) {
			controller.add(e);
			return e;
		}
		else
			throw new ExecutionException("Execution "+id+" is invalid an thus can not be started.");
	}
	
	public Execution createMetadata(Locator locator, Properties contextProps, Properties internalProps) throws ExecutionException {
		return new Execution(ExecutionTypes.METADATA,locator,contextProps,internalProps);
	}

	public Execution createOutputDescription(Locator locator, Properties properties,String format) throws ExecutionException {
		Properties internals = new Properties();
		if (format != null)
			internals.setProperty("format", format);
		return new Execution(ExecutionTypes.OUTPUTDESCRIPTION,locator,properties,internals);
	}

	public Execution createTest(Locator locator, Properties properties) throws ExecutionException {
		return new Execution(ExecutionTypes.TEST,locator,properties,new Properties());
	}

	public Execution createExecution(Locator locator, Properties properties) throws ExecutionException {
		return new Execution(ExecutionTypes.STANDARD,locator,properties,new Properties());
	}
	
	public Execution createExecution(Locator locator, Properties properties, ExecutionState masterState) throws ExecutionException {
		return new Execution(ExecutionTypes.STANDARD,locator,properties,new Properties(),masterState);
	}

	public Execution createData(Locator locator, Properties properties, String format) throws ExecutionException {
		Properties internals = new Properties();
		if (format != null)
			internals.setProperty("format", format);
		return new Execution(ExecutionTypes.DATA,locator,properties,internals);
	}

	/*private void stop(Execution execution, boolean interrupt) throws ExecutionException {

		Codes status = execution.getExecutionState().getStatus();
		switch (status) {
		case statusRunning: execution.stop(); break;
		case statusQueued: execution.abort(); break;
		case statusStopping: execution.interrupt(); break;
		default:
		}
		controller.remove(execution);
		
	}*/

	public ExecutionState stop(Long id) throws ExecutionException {
		return removeExecution(id,false);
	}

	private void waitForTermination(Execution execution) {
		synchronized(execution) {
			while (execution.isActive()) {
				try {
					execution.wait();
				} catch (InterruptedException e) {}
			}
		}
	}

	private ExecutionState getStatus(Execution execution, boolean waitForTermination) throws ExecutionException {
		if (waitForTermination) {
			waitForTermination(execution);
		}
		//remove states with transient inline data from memory, since they are not removed automatically on finish.
		ExecutionState result = execution.getExecutionState();
		if (!execution.isActive() && (DataTargets.CSV_INLINE.equals(result.getDataTarget()) || DataTargets.XML_INLINE.equals(result.getDataTarget()))) {
			removeStateFromMemory(result);
		}
		return result;
	}

	public ExecutionState getExecutionState(Long id, boolean waitForTermination) throws ExecutionException {
		try {
			Execution execution = getExecution(id);
			return getStatus(execution,waitForTermination);
		}
		catch (ExecutionException e) {
			if (id == null) //Exception is thrown because of invalid id.
				throw new ExecutionException(e);
			//execution is thrown because execution is not in memory. look in persistence
			ExecutionState state;
			try {
				state = StateManager.getInstance().getResult(id);
			} catch (RuntimeException e1) {
				throw new ExecutionException(e1);
			}
			if (state != null)
				return state;
			else
				throw new ExecutionException(e);
		}
	}

	public ArrayList<ExecutionState> getActiveExecutions() {
		ArrayList<ExecutionState> result = new ArrayList<ExecutionState>();
		for (Execution e : executions.values()) {
			if (e.isActive()) {
				result.add(e.getExecutionState());
			}
		}
		return result;
	}

	public ArrayList<ExecutionState> getRunningExecutions() {
		ArrayList<ExecutionState> result = new ArrayList<ExecutionState>();
		for (Execution e : executions.values()) {
			if (e.isRunning()) {
				result.add(e.getExecutionState());
			}
		}
		return result;
	}
	
	public ArrayList<ExecutionState> getUnfinishedExecutions() {
		ArrayList<ExecutionState> result = new ArrayList<ExecutionState>();
		for (Execution e : executions.values()) {
			if (!e.isFinished()) {
				result.add(e.getExecutionState());
			}
		}
		return result;
	}

	public synchronized void shutdown() {
		isExecutable = false;
		controller.shutdown();
		//save logs
		log.info("Shutting down executor.");
		for (ExecutionState state : getRunningExecutions()) {
			try {
				stop(state.getId());
				state.setFirstErrorMessage("Failed because of server shutdown.");
				state.setStatus(Codes.statusFailed);
				if (state.isArchive()) StateManager.getInstance().save(state);
			}
			catch (Exception e) {}
		}
		log.info("Executor shutdown finished.");
	}

	@Override
	public void configurationChanged(ConfigurationChangeEvent e) {
		// IGNORE
	}
	
	private void removeStateFromMemory(ExecutionState state) {
		executions.remove(state.getId());
		controller.removeFuture(state.getId());
	}

	@Override
	public void executionFinished(ExecutionState state) {
		if (!(DataTargets.CSV_INLINE.equals(state.getDataTarget()) || DataTargets.XML_INLINE.equals(state.getDataTarget()))) {
			removeStateFromMemory(state);
		}
	}

	@Override
	public void executionStopping(ExecutionState state) {
		//NOTHING TO DO
		
	}

}