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
package com.jedox.etl.core.execution;
import java.util.ArrayList;
import java.util.Properties;
import com.jedox.etl.core.component.Locator;

/**
 * Interface for the creation, management and execution of {@link Execution Executions}. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

public interface IExecutor {
	/**
	 * adds an execution to be managed by the executor.
	 * @param execution the execution to be added
	 * @throws ExecutionException 
	 */
	public void addExecution(Execution execution) throws ExecutionException;
	/**
	 * removes an execution from this executor 
	 * @param id the unique id of the execution
	 * @param removeFromHistory if true, then the execution is also removed from the persistent history and no trace is left that it was ever present.
	 * @return the state of the removed execution
	 * @throws ExecutionException
	 */
	public ExecutionState removeExecution(Long id, boolean removeFromHistory) throws ExecutionException;
	/**
	 * gets an execution by its id. 
	 * @param id the unique id of the execution
	 * @return the execution
	 * @throws ExecutionException If the execution was not added by {@link #addExecution(Execution)} before
	 */
	public Execution getExecution(Long id) throws ExecutionException;
	/**
	 * runs /executes an execution. 
	 * @param id the unique id of the execution
	 * @return the execution
	 * @throws ExecutionException If the execution was not added by {@link #addExecution(Execution)} before
	 */
	public Execution runExecution(Long id) throws ExecutionException;
	/**
	 * stops /terminates / aborts an execution. Removes it from the {@link IExecutor Executor} keeping it in persistent history.
	 * @param id the unique id of the execution
	 * @return the execution
	 * @throws ExecutionException If the execution was not added by {@link #addExecution(Execution)} before
	 */
	public ExecutionState stop(Long id) throws ExecutionException;
	/**
	 * gets the state of an execution. 
	 * @param id the unique id of the execution
	 * @param waitForTermination if true and the execution is not yet finished the executor waits for the termination of this execution before returning its state. Otherwise the current execution state is returned.
	 * @return the execution state.
	 * @throws ExecutionException
	 */
	public ExecutionState getExecutionState(Long id, boolean waitForTermination) throws ExecutionException;
	/**
	 * creates an {@link Execution} for the execution of an {@link IExecutable}
	 * @param locator the locator of the executable to create an execution for
	 * @param properties context variables to set in the execution to create
	 * @return a newly created execution
	 * @throws ExecutionException
	 */
	public Execution createExecution(Locator locator, Properties properties) throws ExecutionException;
	/**
	 * creates an execution for the calculation of a component output description   
	 * @param locator the locator of the component to determine the output description.
	 * @param properties context variables to set in the execution to create
	 * @param format a format to render a {@link com.jedox.etl.core.source.ITreeSource tree model} in. If the component does not provide a tree model, this parameter is ignored.
	 * @return a newly created execution.
	 * @throws ExecutionException
	 */
	public Execution createOutputDescription(Locator locator, Properties properties,String format) throws ExecutionException;
	/**
	 * creates an execution for the test of a component
	 * @param locator the locator of the component to be tested in runtime
	 * @param properties context variables to set in the execution to create
	 * @return a newly created execution.
	 * @throws ExecutionException
	 */
	public Execution createTest(Locator locator, Properties properties) throws ExecutionException;
	/**
	 * creates an execution for the production of data
	 * @param locator the locator of the {@link com.jedox.etl.core.source.ISource data source}
	 * @param properties context variables to set in the execution to create
	 * @param format a format to render a {@link com.jedox.etl.core.source.ITreeSource tree model} in. If the component does not provide a tree model, this parameter is ignored.
	 * @return a newly created execution.
	 * @throws ExecutionException
	 */
	public Execution createData(Locator locator, Properties properties, String format) throws ExecutionException;
	/**
	 * creates an execution for the production of connection metadata
	 * @param locator the locator of the {@link com.jedox.etl.core.connection.IConnection connection}
	 * @param contextProps context variables to set in the execution to create
	 * @param internalProps internal variables to set for the connection metadata function
	 * @return a newly created execution.
	 * @throws ExecutionException
	 */
	public Execution createMetadata(Locator locator, Properties contextProps, Properties internalProps) throws ExecutionException;
	/**
	 * gets all executions currently running in this executor
	 * @return the list of running executions
	 */
	public ArrayList<ExecutionState> getRunningExecutions();
	/**
	 * gets all executions currently queued or running in this executor
	 * @return
	 */
	public ArrayList<ExecutionState> getActiveExecutions();
	/**
	 * shuts the executor down and forces termination of all runnning executions.
	 */
	public void shutdown();
	
}
