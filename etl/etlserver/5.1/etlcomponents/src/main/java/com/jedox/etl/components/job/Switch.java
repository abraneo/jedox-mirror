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

import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.job.SwitchJobConfigurator;
import com.jedox.etl.components.config.job.SwitchJobConfigurator.SwitchClause;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.ExecutionException;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.execution.ResultCodes;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.job.JobManager;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.LoadManager;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.filter.IEvaluator;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.NamingUtil;

/**
 * 
 * @author kais.haddadin@jedox.com
 *
 */
public class Switch extends Component implements IJob {
	
	private static final Log log = LogFactory.getLog(Switch.class);
		
	public Switch() {
		setConfigurator(new SwitchJobConfigurator());
	}
	
	public SwitchJobConfigurator getConfigurator() {
		return (SwitchJobConfigurator)super.getConfigurator();
	}
	
	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
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
				String value = null;
				if(getConfigurator().isConditionSource()){
					ISource source = getConfigurator().getConditionSource();
					int rowNumber = getConfigurator().getSourceRow();
					String input = getConfigurator().getSourceInput();
					IProcessor processor = source.getProcessor(rowNumber);
					int count = 0;
					Row row = null;
					while(count<rowNumber){
						row=processor.next();
						if(row!=null){
							count++;
							if(count==getConfigurator().getSourceRow()){
								value = row.getColumn(input).getValueAsString();
							}
						}else{
							throw new RuntimeException("Row number " + rowNumber + " does not exist in source \"" + source.getName() + "\"");
						}
					}
				}else{
					IExecutable conditionExecutable = getConfigurator().getConditionExecutable();
					Execution execution = runExecutable(conditionExecutable);
					ResultCodes resultCodes = new ResultCodes();
					if(execution.getExecutionState().getErrors()>0)
						value = resultCodes.getNumeric(Codes.statusFailed);
					else if(execution.getExecutionState().getWarnings()>0)
						value = resultCodes.getNumeric(Codes.statusWarnings);
					else
						value = resultCodes.getNumeric(Codes.statusOK);
				}
				log.info("The condition value is \"" + value +"\"");
				for(SwitchClause switchClause: getConfigurator().getSwitches()){
					IEvaluator evaluator = switchClause.getEvaluator();
					boolean match = evaluator.evaluate(value);
					if(match){
						IExecutable switchExecutable = switchClause.getResultExecutable();
						runExecutable(switchExecutable);
						return;	
					}
				}
				
				IExecutable defaultExecutable = getConfigurator().getDefaultExecutable();
				if(defaultExecutable!=null){
					runExecutable(defaultExecutable);
				}	
			} 
			catch (Exception e) {
				log.error("Error executing job \""  +  getName() + "\": " + e.getMessage());
			}
			finally {
				log.debug("Finishing job "+getName()+".");
			}
		}
	}

	/**
	 * @param defaultExecutable
	 * @return 
	 * @throws ExecutionException
	 */
	private Execution runExecutable(IExecutable defaultExecutable)
			throws ExecutionException {
		//push Variables top down, except for default context variable values, which may be overwritten anywhere at deeper level.
		Properties subJobEnv = new Properties();
		subJobEnv.putAll(defaultExecutable.getContext().getExternalVariables());
		subJobEnv.setProperty(NamingUtil.internal("syncMode"), SyncModes.SINGLE.toString());
		subJobEnv.setProperty(NamingUtil.internal("failOnError"), "false");
		Execution execution = Executor.getInstance().createExecution(defaultExecutable.getLocator(), subJobEnv,getContext().getState());
		execution.execute(Thread.currentThread().getName());
		return execution;
	}

	public void test() throws RuntimeException {
		super.test();
		for (ISource source : getSourceManager().getAll()) {
			source.test();
		}
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
			
			addManager(new SourceManager());
			addManager(new LoadManager());
			addManager(new JobManager());
			
			if(getConfigurator().isConditionSource())
				getSourceManager().add(getConfigurator().getConditionSource());
			else
				addToManager(getConfigurator().getConditionExecutable());
			
			for(SwitchClause sw: getConfigurator().getSwitches())
				addToManager(sw.getResultExecutable());
			
			IExecutable defaultExec = getConfigurator().getDefaultExecutable();
			if(defaultExec!=null)
				addToManager(defaultExec);
			
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
			
	}

	/**
	 * @param e
	 * @throws RuntimeException
	 */
	private void addToManager(IExecutable e) throws RuntimeException {
		if (e instanceof IJob) {
			getJobManager().add(e);					
		} 
		else if (e instanceof ILoad) {
			getLoadManager().add(e);
		}
	}

	@Override
	public boolean isExecutable() {
		return getContext().isExecutable();
	}

	@Override
	public boolean isParallel() {
		return false;
	}
	
}
