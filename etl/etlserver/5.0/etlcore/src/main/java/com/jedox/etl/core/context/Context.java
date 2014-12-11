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
package com.jedox.etl.core.context;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.extract.ExtractManager;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.job.JobManager;
import com.jedox.etl.core.load.LoadManager;
import com.jedox.etl.core.source.CompositeSourceManager;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.transform.TransformManager;
import com.jedox.etl.core.util.NamingUtil;

import java.util.Properties;

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
					log.warn("Execution parameter "+key.toString()+" is not existing. Ignoring it.");
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

	public void clear() {
		if (!isDefault()) {
			ContextManager ccm = (ContextManager) getManager(ITypes.Contexts);
			for (IContext c : ccm.getAll()) {
				c.clear();
			}
			//ccm.clear();
			for (IComponent c : getManager(ITypes.Sources).getAll()) {
				((ISource)c).invalidate();
			}
			getManager(ITypes.Sources).clear();
			getManager(ITypes.Functions).clear();
			getManager(ITypes.Jobs).clear();
			getManager(ITypes.Loads).clear();
			for (IComponent c : getManager(ITypes.Connections).getAll()) {
				IConnection connection = (IConnection)c;
				connection.keep(false);
				connection.close();
			}
			getManager(ITypes.Connections).clear();
		}
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
