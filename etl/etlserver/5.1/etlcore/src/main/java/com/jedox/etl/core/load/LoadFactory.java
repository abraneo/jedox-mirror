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
package com.jedox.etl.core.load;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.config.ConfigResolver;
import com.jedox.etl.core.config.ConfigValidator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.util.CustomClassLoader;


/**
 * Factory class for the creation of {@link ILoad Loads}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class LoadFactory {
	
	private static final LoadFactory instance = new LoadFactory();
	private static final Log log = LogFactory.getLog(LoadFactory.class);
	
	public LoadFactory() {
	}
	
	/**
	 * gets the singleton factory instance
	 * @return the LoadFactory instance
	 */
	public static final LoadFactory getInstance() {
		return instance;
	}
	
	/**
	 * creates a new {@link ILoad Load} from a config
	 * @param descriptor the descriptor holding static component information from a component.xml 
	 * @param parent the parent object of this connection, usually a {@link LoadManager}
	 * @param context the context to use for load creation
	 * @param config the XML configuration of the load
	 * @return a newly created load
	 * @throws CreationException, if anything goes wrong in this process
	 */
	public ILoad newLoad(ComponentDescriptor descriptor, ILocatable parent, IContext context, Element config) throws CreationException {
		try {
			Class<?> exporterClass = CustomClassLoader.getInstance().loadClass(descriptor.getClassName());
			ILoad e = (ILoad) exporterClass.newInstance();
			e.getConfigurator().addParameter(descriptor.getParameters());
			e.getConfigurator().addParameter(parent.getParameter());
			e.getConfigurator().setLocator(parent.getLocator(),context);
			ConfigResolver resolver = new ConfigResolver();
			config = resolver.resolve(config, context);
			ConfigValidator.getInstance().validate(exporterClass, config);
			e.getConfigurator().setXML(config,resolver.getVariables());
			e.initialize();
			log.debug("Created Load "+e.getName());
			return e;
		}
		catch (Exception e) {
			log.debug("Failed to create load "+config.getAttributeValue("name")+".");
			if (e instanceof ClassNotFoundException) 
				throw new CreationException("Class "+descriptor.getClassName()+" not found for load of type "+descriptor.getName()+". Please check component.xml.");
			throw new CreationException(e);
		}
	}

}
