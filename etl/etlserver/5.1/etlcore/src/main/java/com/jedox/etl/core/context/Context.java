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
package com.jedox.etl.core.context;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.execution.ExecutionDetail;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.extract.ExtractManager;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.job.JobManager;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.LoadManager;
import com.jedox.etl.core.source.CompositeSourceManager;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.transform.TransformManager;
import com.jedox.etl.core.util.NamingUtil;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.Map;

/**
 * Standard implementation class for {@link IContext}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Context extends Component implements IContext {

	private static final Log log = LogFactory.getLog(Context.class);
	private String baseContextName;
	private Properties variables = new Properties();
	private Properties externalVariables = new Properties();
	private Properties componentExecutionParameter = new Properties();
	private ConfigManager configManager;
	private ExecutionState state;
	private ProcessorRegistry processorRegistry = new ProcessorRegistry();

	public Context() {
		setConfigurator(new BasicConfigurator());
	}
	
	public BasicConfigurator getConfigurator() {
		return (BasicConfigurator)super.getConfigurator();
	}

	protected void addManager(IManager manager, IManager.LookupModes mode) {
		super.addManager(manager, mode);
		//pretend for all managers to be on root level
		manager.setLocator(getLocator().getRootLocator().add(manager.getName()),this);
	}

	public boolean isDefault() {
		return getName().equals(defaultName);
	}

	public String getBaseContextName() {
		return (baseContextName == null) ? getName() : baseContextName;
	}

	public void setBaseContextName(String baseContextName) {
		this.baseContextName = baseContextName;
	}
	
	private boolean isComponentExecutionParameter(String parameter) {
		int parameterStart = parameter.lastIndexOf(".");
		if (parameterStart > -1) {
			String locatorString = parameter.substring(1,parameterStart);
			try {
				if (getConfigManager().findElement(Locator.parse(locatorString)) != null) {
					return true;
				}
			}
			catch (ConfigurationException e) {
				return false;
			}
		}
		return false;
	}
	
	public void addVariables(Properties variables) {
		Properties acceptedVariables = new Properties();
		//variables have to be declared
		for (Object key : variables.keySet()) {
			if (!this.variables.containsKey(key))
				//check, if it is an execution parameter, which is allowed
				if (Settings.getInstance().getContext(Settings.ExecutionsCtx).containsKey(key)) {
					getParameter().setProperty(key.toString(), variables.getProperty(key.toString()));
					acceptedVariables.put(key, variables.get(key));
				}
				else if (key.toString().startsWith(NamingUtil.internalPrefix())) {
					if (isComponentExecutionParameter(key.toString())) {
						componentExecutionParameter.put(key, variables.get(key));
					} else {
						log.warn("Execution parameter "+key.toString()+" is not existing. Ignoring it.");
					}
				}
				else if (key.toString().startsWith(NamingUtil.hiddenInternalPrefix())) {
					acceptedVariables.put(key, variables.get(key));
				}
				else {
					log.warn("Variable "+key.toString()+" is not declared in variables section. Ignoring it.");
				}
			else
			{
				acceptedVariables.put(key, variables.get(key));
			}
		}
		this.variables.putAll(acceptedVariables);
		externalVariables.putAll(acceptedVariables);
	}
	
	public Properties getVariables() {
		return variables;
	}
	
	public Properties getExternalVariables() {
		return externalVariables;
	}

	public void addChildContext(IContext child) throws RuntimeException {
		getManager(ITypes.Contexts).add(child);
	}
	
	public void registerProcessor(IProcessor processor) {
		int logBlockSize = 0;
		try {
			String logBlockSizeString = processor.getOwner().getParameter().getProperty("logBlockSize", "0");
			logBlockSizeString = componentExecutionParameter.getProperty(NamingUtil.internalPrefix()+processor.getOwner().getLocator().clone().add("logBlockSize").toString(), logBlockSizeString);
			logBlockSize = Integer.parseInt(logBlockSizeString);
		} catch (NumberFormatException e) {
			log.warn("Value of parameter logBlockSize has to be numeric.");
		}
		processor.setLogBlockSize(logBlockSize);
		processorRegistry.addProcessor(processor);
	}
	
	public void registerLoad(ILoad load) {
		processorRegistry.addLoad(load);
	}
	
	public Map<String,ExecutionDetail> getExecutionDetails() {
		return processorRegistry.getExecutionDetails();
	}
	
	public List<IProcessor> getProcessorsByOwner(IComponent component) {
		return processorRegistry.getProcessorsByOwner(component);
	}

	public void clear() {
		if (!isDefault() && getManagers().length > 0) {
			if (state != null) {
				state.addDetails(getExecutionDetails());
				if (isOptimizePersistence()) {
					List<ISource> sourcesToCleanGlobally = new ArrayList<ISource>();
					for (IComponent c : getManager(ITypes.Sources).getAll()) {
						sourcesToCleanGlobally.add((ISource)c);
					}
					state.addSourcesToClean(sourcesToCleanGlobally);
				}
			} else {
				for (IComponent c : getManager(ITypes.Sources).getAll()) {
					((ISource)c).clearCache();
				}
			}
			processorRegistry.clear();
			ContextManager ccm = (ContextManager) getManager(ITypes.Contexts);
			for (IContext c : ccm.getAll()) {
				c.clear();
			}
			for (IComponent c : getManager(ITypes.Sources).getAll()) {
				((ISource)c).invalidate();
				if (!isOptimizePersistence()) ((ISource)c).clearCache();
			}
			for (IComponent c : getManager(ITypes.Connections).getAll()) {
				IConnection connection = (IConnection)c;
				connection.keep(false);
				connection.close();
			}
			if (configManager != null) { //may be already cleaned if it is a nested child context
				configManager.getProjectManager().clear();
				configManager = null;
			}
			variables.clear();
			externalVariables.clear();
			componentExecutionParameter.clear();
			variables = null;
			externalVariables = null;
			componentExecutionParameter = null;
			ContextManager.getInstance().remove(getName());
			clearManagers();
			state = null;
		} 
	}
	
	public void setConfigManager(ConfigManager configManager) {
		this.configManager = configManager;
	}
	
	public ConfigManager getConfigManager() {
		return configManager;
	}
	
	@Override
	public void setState(ExecutionState state) {
		this.state = state;
	}

	@Override
	public ExecutionState getState() {
		return state;
	}

	@Override
	public boolean isExecutable() {
		return (state == null) || state.isExecutable();
	}
	
	
	public IComponent getComponent(Locator locator) throws ConfigurationException {
		return configManager.getComponent(locator, this.getName());
	}

	public void init() throws InitializationException {
		super.init();
		variables = getConfigurator().getVariables();
		addManager(new ContextManager()); //for derived contexts
		addManager(new FunctionManager(), IManager.LookupModes.Locator);
		addManager(new JobManager(), IManager.LookupModes.Locator);
		addManager(new LoadManager(), IManager.LookupModes.Locator);
		addManager(new ConnectionManager(), IManager.LookupModes.Locator);
		//combine transforms and extracts into a composite source manager, which manages both. Both are directly accessible also.
		TransformManager transformManager = new TransformManager();
		ExtractManager extractManager = new ExtractManager();
		addManager(transformManager, IManager.LookupModes.Locator);
		addManager(extractManager, IManager.LookupModes.Locator);
		CompositeSourceManager compositeManager = new CompositeSourceManager();
		compositeManager.addManager(extractManager);
		compositeManager.addManager(transformManager);
		addManager(compositeManager, IManager.LookupModes.Locator);
	}
}
