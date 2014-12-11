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

import java.util.Properties;

import com.jedox.etl.components.config.job.ExternalJobConfigurator;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.OLAPAuthenticator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.job.IJob;
import com.jedox.etl.core.util.NamingUtil;

public class ExternalJob extends Component implements IJob {
	
	private IJob externalJob;
	
	public ExternalJob() {
		setConfigurator(new ExternalJobConfigurator());
	}

	public ExternalJobConfigurator getConfigurator() {
		return (ExternalJobConfigurator)super.getConfigurator();
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			Locator externalLocator = getConfigurator().getLocators().get(0);
			//check permission
			Properties properties = new Properties();
			String session = getContext().getVariables().getProperty(NamingUtil.hiddenInternalPrefix()+NamingUtil.etlsession);
			if(session!=null)
				properties.setProperty(NamingUtil.etlsession, session);
			OLAPAuthenticator.getInstance().authenticateComponent(externalLocator.getRootLocator(), properties,OLAPAuthenticator.roPermSet2);
			//check project
			ConfigManager.getInstance().checkComponent(externalLocator);
			//check external context default variables
			IContext externalContext = ConfigManager.getInstance().getContext(externalLocator.getRootName(), IContext.defaultName);
			for (Object key : externalContext.getVariables().keySet()) {
				if (!getContext().getVariables().containsKey(key)) {
					getContext().getVariables().put(key, externalContext.getVariables().get(key));
				}
			}
			//add external project definition to context cache
			getContext().getConfigManager().add(externalLocator.getRootLocator(), ConfigManager.getInstance().findElement(externalLocator.getRootLocator()));
		    //init job
			externalJob = (IJob) getContext().getComponent(externalLocator);
		}
		catch (ConfigurationException e) {
			throw new InitializationException(e);
		}
	}

	@Override
	public void execute() {
		externalJob.execute();
	}

	@Override
	public boolean isExecutable() {
		return externalJob.isExecutable();
	}

	@Override
	public boolean isParallel() {
		return false;
	}

}
