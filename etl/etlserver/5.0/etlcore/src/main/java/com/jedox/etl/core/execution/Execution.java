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
import java.util.Collections;
import java.util.Date;
import java.util.Properties;
//import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.ViewSource;
import com.jedox.etl.core.logging.ExecutionLog;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;


/**
 * Wrapper for an {@link IExecutable} to run in its own thread, so it can be parallelized and stopped. The Constructor takes the executable and a flag, if this execution is synchronized (not parallelized) be the {@link Executor}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Execution implements Runnable {
	
	public enum ExecutionTypes {
		EXECUTION, OUTPUTDESCRIPTION, TEST, METADATA, DATA
	}

	private IExecutable executable;
	private ExecutionState state;
	private static final Log log = LogFactory.getLog(Execution.class);
	
	private ExecutionTypes type;
	private Locator locator;
	private Properties contextProperties = new Properties();
	private Properties internalProperties = new Properties();
	//private ExecutionLog elog;
	private boolean failOnError;
	private String name;
	private boolean dependent;
	
	
	public Execution(ExecutionTypes type, Locator locator, Properties contextProperties, Properties internalProperties) throws ExecutionException {
		this(type,locator,contextProperties,internalProperties,new ExecutionState(locator));
		dependent = false;
	}
	
	public Execution(ExecutionTypes type, Locator locator, Properties contextProperties, Properties internalProperties, ExecutionState state) throws ExecutionException {
		this.name = locator.getType()+" "+locator.getName();
		this.type = type;
		this.locator = locator;
		this.contextProperties.putAll(contextProperties);
		this.internalProperties.putAll(internalProperties);
		this.state = state;
		ExecutionLog elog = state.getExecutionLog();
		elog.addTargetThreadName(Thread.currentThread().getName());
		switch (type) {
		case EXECUTION: {
			state.setArchive(true);
			break;
		}
		default: {
			state.setArchive(false);
		}
		}
		executable = initExecutable();
		checkDependentStates(executable,getExecutionState());
		setFailOnError(executable.getContext().getParameter().getProperty("#failOnError", "true").equalsIgnoreCase("true"));
		elog.setFailOnError(isFailOnError());
		elog.removeTargetThreadName(Thread.currentThread().getName());
		dependent = true;
	}
	
	private synchronized IContext createContext(IContext parentContext, Properties variables) throws ConfigurationException, CreationException, RuntimeException {
		IContext c = ContextManager.getInstance().provide(parentContext);
		c.addVariables(variables);
		return c;
	}
	
	public static void checkDependentStates(IExecutable executable, ExecutionState state) throws ExecutionException {
		//take care of dependencies 
		ArrayList<IComponent> components = executable.getDependencies();
		components.add(executable);
		Collections.reverse(components);
		for (IComponent component : components) {
			try {
				//set the state of all executables
				if (component instanceof IExecutable) {
					IExecutable e = (IExecutable)component;
					e.setState(state);
				}
			}
			catch (Exception e) {
				log.debug("Failed to prepare execution of "+executable.getLocator().getType()+" "+executable.getName()+".");
				throw new ExecutionException(e);
			}
		}
	}
	
	private int parseLogLimit(IContext context) {
		String value = context.getParameter().getProperty(NamingUtil.internal("logLimit"), Settings.getInstance().getContext(Settings.ExecutionsCtx).getProperty(NamingUtil.internal("logLimit"), "100"));
		try {
			return Integer.parseInt(value);
		}
		catch (NumberFormatException e) {
			log.warn("Parameter logLimit is required to be numeric, but current value is: "+value+". Using default of 100.");
			return 100;
		}
	}
	
	private IExecutable initExecutable() throws ExecutionException {
		IExecutable executable = null;
		try {
			StateManager.getInstance().save(state);
			if (locator.getManager().equals(ITypes.Jobs)) { //directly add override variables defined in a job.
//				if (ConfigManager.getInstance().get(locator.getRootLocator())!=null && ConfigManager.getInstance().get(locator)!=null) { // check if project and job is existing
				ConfigManager.getInstance().checkComponent(locator);	
				Properties jobProperties = XMLUtil.parseProperties(ConfigManager.getInstance().get(locator).getChildren("variable"));
			    // overwrite static job variables with dynamical context value 
				for (Object key : jobProperties.keySet()) {
					if (contextProperties.containsKey(key)) {
						if (!contextProperties.get(key).equals(jobProperties.get(key)))
							log.warn("Statical variable "+key+" of job "+locator.getName()+" with value "+jobProperties.get(key)+" is dynamically overwritten with value "+contextProperties.get(key));							
					}
					else
						contextProperties.put(key,jobProperties.get(key));						
				}							
	
			}
				
			//create new context based on given variables
			IContext context = createContext(ConfigManager.getInstance().getContext(locator.getRootName(), IContext.defaultName),contextProperties);
			state.getExecutionLog().setLogLimit(parseLogLimit(context));
			IComponent locatable = ConfigManager.getInstance().getComponent(locator, context.getName());
			switch (type) {
			case EXECUTION: {
				if (locatable instanceof IExecutable) {
					executable = (IExecutable)locatable;
					break;
				}
				else throw new ExecutionException(locator.getType() +" "+locator.getName()+" is not executable.");	
			}
			case METADATA: {
				executable = new MetadataWrapper(locatable,internalProperties);
				break;
			}
			case DATA: {
				if ((locatable instanceof ITreeSource) /* && (internalProperties.getProperty("format") != null) */) { //CSCHW: always convert to tree, since there may be errors not detected by simply using base source table format
					locatable = new ViewSource(locator,context,internalProperties.getProperty("format"));
				}
				if (locatable instanceof IExecutable) {
					executable = (IExecutable)locatable;
					break;
				}
				else throw new ExecutionException(locator.getType() +" "+locator.getName()+" is not executable.");	
			}
			case TEST: {
				executable = new TestWrapper(locatable);
				break;
			}
			case OUTPUTDESCRIPTION: {
				if ((internalProperties.getProperty("format")  != null) && (locatable instanceof ITreeSource)) {
					locatable = new ViewSource(locator,context,internalProperties.getProperty("format") );
				}
				executable = new OutputDescriptionWrapper(locatable);
				break;
			}
			}
			executable.setState(state);
			return executable;	
		}
		catch (Exception e) {
			try {
				state.setExecutable(false);
				state.setStatus(Codes.statusInvalid);
				state.setFirstErrorMessage(e.getMessage());
				StateManager.getInstance().save(state);
			}
			catch (RuntimeException re) {
				log.error("Exception recovery of execution init failed. State may have not been persisted. "+re.getMessage());
			}
			throw new ExecutionException(e);
		}
	}
	

	/**
	 * gets the unique id of this execution
	 * @return
	 * @throws ExecutionException
	 */
	public Long getKey() {
		return getExecutionState().getId();
	}

/*
	public List<String> getOutgoingConnections() {
		return outgoingConnections;
	}
*/
	/**
	 * runs this execution in its own thread
	 */
	public synchronized void run() {
		//Thread.currentThread().setName(getName());
		//execute(getName());
		execute(Thread.currentThread().getName());
		notifyAll();
	}
	
	private void cleanUp() {
		//do some cleanup.
		if (executable != null) {
			executable.getContext().clear();
			//also remove now empty context from context manager
			ContextManager.getInstance().remove(executable.getContextName());
		}
		executable = null;
		contextProperties = null;
		internalProperties = null;
		if (!dependent)
			state.getExecutionLog().close();
	}
	
	protected void endExecution(Date startDate) {
		try {
			if (Thread.interrupted()) log.debug("Iterrupted info is cleared to allow cleanup.");
			Codes code = getExecutionState().close(isDependent());
			Date stopDate = new Date();
			long diff = stopDate.getTime() - startDate.getTime();
			log.info("Finished execution of "+(dependent ? "sub" : "") + locator.getManager().substring(0,locator.getManager().length()-1)+" "+locator.getName()+" after " +( (double)diff/1000) + "s with status: "+getExecutionState().getString(code));
			StateManager.getInstance().save(getExecutionState());
		}
		catch (Exception e) {
			try {
				log.error("Exception recovery of execution cleanup failed. State may have not been persisted. Retrying... "+e.getMessage());
				StateManager.getInstance().save(getExecutionState());
			}
			catch (Exception ex) {}
		}
		cleanUp();
	}
		
	/**
	 * adds an Info-Message on omitted Error and Warning Messages
	 */
	public void addInfoOmittedLines() {
		if (state.getExecutionLog().getLinesOmitted() > 0) {
			log.info("Number of messages exceeded the display limit by "+state.getExecutionLog().getLinesOmitted()+" messages. All messages can be found in the ETL Log files.");
		}
	}		
	
	/**
	 * runs the execution logic in the callers thread
	 * @param threadName
	 */
	public void execute(String threadName) {
		Date startDate = new Date();
		try {
			try {
				getExecutionState().getExecutionLog().addTargetThreadName(threadName);
				log.info("Starting execution of "+(dependent ? "sub" : "")+locator.getManager().substring(0,locator.getManager().length()-1)+" "+locator.getName());
				getExecutionState().open(getExecutable().getContext(),isDependent());
				StateManager.getInstance().save(getExecutionState());
				getExecutable().execute();
				endExecution(startDate);
			}
			catch (Exception e) {
				try {
					endExecution(startDate);
					log.error(e.getMessage());
				}
				catch (Exception ex) {}
			}
		}
		catch (OutOfMemoryError e1) { //catch heap space overflow
			System.err.println("FATAL: Execution "+getKey()+" run out of memory: "+e1.getMessage()); //log on system out. should always still work..
			try { //try graceful finish and cleanup
				String errorString = "Abnormally terminated execution of "+(dependent ? "sub" : "") + locator.getManager().substring(0,locator.getManager().length()-1)+" "+locator.getName();
				log.fatal(errorString+": "+e1.getMessage());
				cleanUp();
				getExecutionState().setExecutable(false);
				getExecutionState().close(isDependent());
				StateManager.getInstance().save(getExecutionState());
			}
			catch (Exception ex) {}
			catch (Error error) {}
			throw e1; //re-throw for virtual machine to handle
		}
	}
	
	/**
	 * gets the state of this execution
	 */
	public ExecutionState getExecutionState() {
		return state;
	}

	/**
	 * aborts this execution stopping it and setting status aborted.
	 */
	public synchronized void abort() throws ExecutionException {
		try {
			addInfoOmittedLines();			
			getExecutionState().abort();
			cleanUp();
			StateManager.getInstance().save(getExecutionState());
		}
		catch (RuntimeException e) {
			throw new ExecutionException(e);
		}
	}

	/**
	 * stops this execution.
	 */
	public void stop() throws ExecutionException {
		try {
			getExecutionState().stop();
			StateManager.getInstance().save(getExecutionState());
		} catch (RuntimeException e) {
			throw new ExecutionException(e);
		}		
	}
		
	/**
	 * interrupts and stops this execution.
	 */
	public void interrupt() {
		getExecutionState().stop();
		//log.debug("Stopping execution of: "+getName());
		//super.interrupt();
	}

	/**
	 * gets the executable to be executed by this execution
	 * @return the executable
	 * @throws ExecutionException
	 */
	public IExecutable getExecutable() throws ExecutionException {
		if (executable == null)
			throw new ExecutionException("Execution "+getName()+" has been terminated.");
		return executable;
	}
	
	public boolean isFinished() {
		return executable == null;
	}

	/**
	 * determines if this execution is currently running
	 * @return true if running, false otherwise
	 */
	public boolean isRunning() {
		return getExecutionState().getStatus().equals(Codes.statusRunning);
	}
	
	public Properties getContextProperties() {
		return contextProperties;
	}
	
	public ExecutionTypes getExecutionType() {
		return type;
	}
	
	public String getName() {
		return name;
	}
	
	public boolean isActive() {
		Codes status = getExecutionState().getStatus();
		return (status.equals(Codes.statusRunning) || (status.equals(Codes.statusQueued)));
	}

	public void setFailOnError(boolean failOnError) {
		this.failOnError = failOnError;
	}

	public boolean isFailOnError() {
		return failOnError;
	}

	public void setDependent(boolean dependent) {
		this.dependent = dependent;
	}

	public boolean isDependent() {
		return dependent;
	}

}
