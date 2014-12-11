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

import java.util.Date;
import java.util.Properties;
//import java.util.List;


import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.EventSender;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.ViewSource;
import com.jedox.etl.core.logging.ExecutionLog;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;


/**
 * Wrapper for an {@link IExecutable} to run in its own thread, so it can be parallelized and stopped. The Constructor takes the executable and a flag, if this execution is synchronized (not parallelized) be the {@link Executor}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Execution extends EventSender implements Runnable {
	
	public enum ExecutionTypes {
		STANDARD, OUTPUTDESCRIPTION, TEST, METADATA, DATA
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
	private boolean dependent = false;
	
	
	public Execution(ExecutionTypes type, Locator locator, Properties contextProperties, Properties internalProperties) throws ExecutionException {
		this(type,locator,contextProperties,internalProperties,provideState(type,locator));
		dependent = false;
	}
	
	private static ExecutionState provideState(ExecutionTypes type, Locator locator) throws ExecutionException {
		try {
			return StateManager.getInstance().provideState(type, locator);
		}
		catch (RuntimeException e) {
			throw new ExecutionException(e);
		}
	}
	
	public Execution(ExecutionTypes type, Locator locator, Properties contextProperties, Properties internalProperties, ExecutionState state) throws ExecutionException {
		this.type = type;
		this.locator = locator;
		this.contextProperties.putAll(contextProperties);
		this.internalProperties.putAll(internalProperties);
		this.state = state;
		ExecutionLog elog = state.getExecutionLog();
		elog.addTargetThreadName(Thread.currentThread().getName());
		
		executable = initExecutable();
		setFailOnError(executable.getContext().getParameter().getProperty("#failOnError", "true").equalsIgnoreCase("true"));
		elog.setFailOnError(isFailOnError());
		elog.removeTargetThreadName(Thread.currentThread().getName());
		dependent = true;
	}
	
	private synchronized IContext createContext(IContext parentContext, Properties ownVariables, Properties contextVariables) throws ConfigurationException, CreationException, RuntimeException {
		IContext c = ContextManager.getInstance().provide(parentContext);
		c.setState(getExecutionState());
		c.getVariables().putAll(ownVariables);
		c.addVariables(contextVariables);
		//add session info
		if(locator.getSessioncontext()!=null)
			c.getVariables().setProperty(NamingUtil.hiddenInternalPrefix()+NamingUtil.etlsession, locator.getSessioncontext());
		return c;
	}
	
	private IExecutable initExecutable() throws ExecutionException {
		IExecutable executable = null;
		try {
			Properties ownVariables = new Properties();
			if (locator.getManager().equals(ITypes.Jobs)) { //directly add override variables defined in a job.
//				if (ConfigManager.getInstance().get(locator.getRootLocator())!=null && ConfigManager.getInstance().get(locator)!=null) { // check if project and job is existing
				ConfigManager.getInstance().checkComponent(locator);	
				ownVariables = XMLUtil.parseProperties(ConfigManager.getInstance().get(locator).getChildren("variable"));
			    // overwrite static job variables with dynamical context value 
				for (Object key : ownVariables.keySet()) {
					if (contextProperties.containsKey(key)) {
						if (!contextProperties.get(key).equals(ownVariables.get(key)))
							log.warn("Statical variable "+key+" of job "+locator.getName()+" with value "+ownVariables.get(key)+" is dynamically overwritten with value "+contextProperties.get(key));							
					}
					else
						contextProperties.put(key,ownVariables.get(key));						
				}							
	
			}
				
			//create new context based on given variables
			IContext context = createContext(ConfigManager.getInstance().getContext(locator.getRootName(), StringUtils.isEmpty(locator.getContext()) ? IContext.defaultName : locator.getContext()),ownVariables,contextProperties);
			IComponent locatable = context.getComponent(locator);
			switch (type) {
			case STANDARD: {
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
			return executable;	
		}
		catch (Exception e) {
			try {
				log.error(e.getMessage());
				state.setExecutable(false);
				state.setStatus(Codes.statusInvalid);
				state.setStopDate(state.getStartDate());
				state.setFirstErrorMessage(e.getMessage());
				if(state.getMessageWriter()!=null)
					state.getMessageWriter().flush();
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
	 */
	public Long getKey()  {
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
	
	protected void cleanUp() {
		//do some cleanup.
		if (executable != null) {
			executable.getContext().clear();
			//also remove now empty context from context manager
			//ContextManager.getInstance().remove(executable.getContextName()); cschw: done in context.clear() now
		}
		executable = null;
		contextProperties = null;
		internalProperties = null;
		if (!dependent) {
			state.getExecutionLog().close();
			for (ISource s : state.getSourcesToClean()) {
				s.clearCache();
				//invalidate view sources of this sources
				IManager sourceManager = s.getManager(ITypes.Sources);
				if (sourceManager != null) {
					for (IComponent c : sourceManager.getAll()) {
						if (c instanceof IView && c instanceof ISource) { //ViewSource cleanup
							((ISource)c).invalidate();
							((ISource)c).clearCache();
						}
					}
				}
			}
			state.getSourcesToClean().clear();
			this.fireExecutionFinished(state);	
		}
	}	
	
	private void persistDetails() throws RuntimeException, ExecutionException {
		//handle execution details save only for master execution		
		if (!dependent) {
			getExecutable().getContext().clear(); //CSCHW: clear context and aggregate detail info transiently into state, where it is taken for persistence.
			StateManager.getInstance().persistDetails(getExecutionState().getDetails().values(),getExecutable().getContext().getParameter().getProperty("#persistDetails", "true").equalsIgnoreCase("true"));
		}
	}
	
	protected void endExecution(Date startDate, int initialWarnings, int initialErrors) {
		try {
			if (Thread.interrupted()) log.debug("Iterrupted info is cleared to allow cleanup.");
			Codes code = getExecutionState().close(isDependent(),initialWarnings,initialErrors,getExecutable().getContext());
			Date stopDate = new Date();
			long diff = stopDate.getTime() - startDate.getTime();
			log.info("Finished execution of "+(dependent ? "sub" : "") + locator.getManager().substring(0,locator.getManager().length()-1)+" "+locator.getName()+" after " +( (double)diff/1000) + "s with status: "+getExecutionState().getString(code));
			StateManager.getInstance().save(getExecutionState());
			persistDetails();
		}
		catch (Exception e) {
			try {
				log.error("Exception recovery of execution cleanup failed. State may have not been persisted. Retrying... "+e.getMessage());
				StateManager.getInstance().save(getExecutionState());
				persistDetails();
			}
			catch (Exception ex) {}
		}
		cleanUp();
	}
	
	/**
	 * runs the execution logic in the callers thread
	 * @param threadName
	 */
	public void execute(String threadName) {
		Date startDate = new Date();
		int initialWarnings=0;    
		int initialErrors=0; 
		try {
			try {
				getExecutionState().getExecutionLog().addTargetThreadName(threadName);
				if (dependent) {
					// Number of Warnings/Errors until now, only relevant for dependent executions in order to determine its correct state
					initialWarnings=getExecutionState().getWarnings(); 
					initialErrors=getExecutionState().getErrors();     
				}	
				if (dependent)
					log.info("Starting execution of sub"+locator.getManager().substring(0,locator.getManager().length()-1)+" "+locator.getName());
				else  
					log.info("Starting execution of "+locator.getManager().substring(0,locator.getManager().length()-1)+" "+locator.getName()+" in project "+locator.getRootName()+" (ID: "+getExecutionState().getId()+")");
				getExecutionState().open(getExecutable().getContext(),isDependent());
				StateManager.getInstance().save(getExecutionState());
				getExecutable().execute();
				endExecution(startDate,initialWarnings,initialErrors);
			}
			catch (Exception e) {
				try {
					endExecution(startDate,initialWarnings,initialErrors);
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
				getExecutionState().close(isDependent(),initialWarnings,initialErrors,null);
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
	 * @throws ExecutionException 
	 */
	public void interrupt() throws ExecutionException {
		try {
			//todo: separate state for interruption
			getExecutionState().setStatus(Codes.statusInterrupted);
			StateManager.getInstance().save(getExecutionState());
		} catch (RuntimeException e) {
			throw new ExecutionException(e);
		}			
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
		Codes status = getExecutionState().getStatus();
		return (status.equals(Codes.statusRunning) || (status.equals(Codes.statusStopping) || (status.equals(Codes.statusInterrupting))));
	}
	
	public Properties getContextProperties() {
		return contextProperties;
	}
	
	public ExecutionTypes getExecutionType() {
		return type;
	}
	
	public String getName() {
		return locator.getDisplayName();
	}
	
	public boolean isActive() {
		Codes status = getExecutionState().getStatus();
		return (status.equals(Codes.statusRunning) || (status.equals(Codes.statusStopping)) || (status.equals(Codes.statusInterrupting))  || (status.equals(Codes.statusQueued)));
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
