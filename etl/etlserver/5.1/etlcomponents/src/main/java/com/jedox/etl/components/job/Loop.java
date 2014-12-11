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
import java.util.List;
import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.job.LoopJobConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.execution.ParallelExecutor;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.NamingUtil;

/**
 * 
 * @author Kais Haddadin. 
 *
 */
public class Loop extends Default implements IJob {

	private static final Log log = LogFactory.getLog(Loop.class);
	private ISource loopsource;
	private int bulkSize = 0;

	public Loop() {
		setConfigurator(new LoopJobConfigurator());
	}

	public LoopJobConfigurator getConfigurator() {
		return (LoopJobConfigurator)super.getConfigurator();
	}

	public void execute() {
		if (isExecutable()) {
			log.debug("Starting job: "+getName());
			try {
				IProcessor loopprocessor  = loopsource.getProcessor();
				Row row = loopprocessor.next();
				if(row == null ){
					throw new RuntimeException("The source " + loopsource.getName() + " is empty.");
				}
				for(int i=0;i<row.getColumns().size();i++){
					IColumn col = row.getColumn(i);
					String var = col.getName();
					if(!getContext().getVariables().containsKey(var)){
						throw new RuntimeException("Source contains column " + var + "  that does not have a corresponding variable.");
					}
					if(getContext().getExternalVariables().containsKey(var)){
						log.warn("Context variable " + var + " with value " + getContext().getExternalVariables().getProperty(var)+ " will be ignored. It can not be used in the job loop because it is given by the loop source.");
					}
				}

				Properties jobEnv = new Properties();
				jobEnv.putAll(getContext().getVariables());
				ParallelExecutor executor = new ParallelExecutor();
				List<Execution> parallelSet = new ArrayList<Execution>();

				while(row!=null){
					int colNum = row.getColumnNames().size();
					for(int i=0;i<colNum;i++){
						IColumn col = row.getColumn(i);
						String var = col.getName();
						String varValue = col.getValueAsString();
						jobEnv.put(var, varValue);
					}			

					//first run all executables
					IExecutable e = executables.get(0);
					e.getContext().addVariables(jobEnv);
					
					if (getConfigurator().isParallel(e) || e instanceof IJob) {
						if (isExecutable()) {
							//push Variables top down, except for default context variable values, which may be overwritten anywhere at deeper level.
							Properties subJobEnv = new Properties();
							subJobEnv.putAll(e.getContext().getExternalVariables());
							Properties ownVariables = e.getConfigurator().getOwnVariables();
							for (Object key : ownVariables.keySet()) {
								if (!subJobEnv.containsKey(key)) subJobEnv.put(key, ownVariables.get(key));
							}
							if(!getConfigurator().isParallel(e)){
								subJobEnv.setProperty(NamingUtil.internal("syncMode"), SyncModes.SINGLE.toString());
								Execution execution = Executor.getInstance().createExecution(e.getLocator(), subJobEnv,getContext().getState());
								execution.execute(Thread.currentThread().getName());
								if (execution.isFailOnError() && (Codes.statusInvalid.equals(execution.getExecutionState().getStatus()) || execution.getExecutionState().getErrors() > 0)) 
									break;
							}else{					
								subJobEnv.setProperty(NamingUtil.internal("syncMode"), SyncModes.PARALLEL.toString());
								Execution execution = Executor.getInstance().createExecution(e.getLocator(), subJobEnv);
								parallelSet.add(execution);
								row = loopprocessor.next();
								if(row==null){
									executor.execute(getContext(), parallelSet);
									break;
								}
								else if(parallelSet.size()== bulkSize){
									executor.execute(getContext(), parallelSet);
									parallelSet.clear();
									continue;
								}else{
									continue;
								}
							}
						}
					}
					else { //execute loads directly in own environment				
						IContext context = ContextManager.getInstance().provide(e.getContext());
						e = (IExecutable) context.getComponent(e.getLocator());
						if (isExecutable()) e.execute();
						context.clear();					
					}
					
					row = loopprocessor.next();
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
		loopsource.test();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			if(executables.size()!= 1){
				throw new InitializationException("Only one Job or one Load can be used in Loop Job.");
			}			
			addManager(new SourceManager());
			loopsource = getConfigurator().getLoopSource();
			getManager(ITypes.Sources).add(loopsource);
			bulkSize = getConfigurator().getBulkSize();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}	
	
}
