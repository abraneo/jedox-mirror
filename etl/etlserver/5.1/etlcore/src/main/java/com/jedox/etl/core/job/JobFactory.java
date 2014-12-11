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
package com.jedox.etl.core.job;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.config.ConfigValidator;
import com.jedox.etl.core.config.ConfigResolver;
import com.jedox.etl.core.context.IContext;


/**
 * Factory class for the creation of {@link IJob Jobs}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class JobFactory {
	
	private static final JobFactory instance = new JobFactory();
	private static Log log = LogFactory.getLog(JobFactory.class);

	public JobFactory() {
	}
	
	/**
	 * gets the static singleton instance
	 * @return the JobFactory instance
	 */
	public static final JobFactory getInstance() {
		return instance;
	}
	
	/**
	 * creates a new {@link IJob Job} from a config
	 * @param descriptor the descriptor holding static component information from a component.xml 
	 * @param parent the parent object of this connection, usually a {@link JobManager}
	 * @param context the context to use for job creation
	 * @param config the XML configuration of the job
	 * @return a newly created job
	 * @throws CreationException, if anything goes wrong in this process
	 */
	public IJob newJob(ComponentDescriptor descriptor, ILocatable parent, IContext context, Element config) throws CreationException{
		try {
			Class<?> jobClass = Class.forName(descriptor.getClassName());
			IJob j = (IJob) jobClass.newInstance();
			j.getConfigurator().addParameter(descriptor.getParameters());
			j.getConfigurator().addParameter(parent.getParameter());
			j.getConfigurator().setLocator(parent.getLocator(),context);
			ConfigResolver resolver = new ConfigResolver();
			config = resolver.resolve(config, context);
			ConfigValidator.getInstance().validate(jobClass, config);
			j.getConfigurator().setXML(config,resolver.getVariables());
			j.initialize();
			log.debug("Created job "+j.getName());
			return j;
		}
		catch (Exception e) {
			log.debug("Failed to create job "+config.getAttributeValue("name", "default")+".");
			throw new CreationException(e);
		}
	}
}
