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
package com.jedox.etl.core.source;

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
 * Factory Class for generic DataSource Creation from a set of properties
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class SourceFactory {
	
	private static final SourceFactory instance = new SourceFactory();
	private static final Log log = LogFactory.getLog(SourceFactory.class);
	
	public SourceFactory() {
	}
	
	/**
	 * gets the static instance
	 * @return
	 */
	public static SourceFactory getInstance() {
		return instance;
	}
	
	/**
	 * instantiates a new source.
	 * @param descriptor the descriptor holding the static configuration from a component.xml
	 * @param parent the parent object of the project, usually a source manager of some kind
	 * @param config the dynamic configuration of the source given by the defining project xml
	 * @return the newly created source instance.
	 * @throws CreationException
	 */
	public ISource newSource(ComponentDescriptor descriptor, ILocatable parent, IContext context, Element config) throws CreationException {
		try {
			Class<?> datasourceClass = CustomClassLoader.getInstance().loadClass(descriptor.getClassName());
			ISource ds = (ISource) datasourceClass.newInstance();
			ds.getConfigurator().addParameter(descriptor.getParameters());
			ds.getConfigurator().addParameter(parent.getParameter());
			ds.getConfigurator().setLocator(parent.getLocator(),context);
			ConfigResolver resolver = new ConfigResolver();
			config = resolver.resolve(config, context);
			ConfigValidator.getInstance().validate(datasourceClass, config);
			ds.getConfigurator().setXML(config,resolver.getVariables());
			ds.initialize();
			log.debug("Created source "+ds.getName()+" in context "+context.getName());
			return ds;
		}
		catch (Exception e) {
			log.debug("Failed to create source "+config.getAttributeValue("name")+".");
			if (e instanceof ClassNotFoundException) 
				throw new CreationException("Class "+descriptor.getClassName()+" not found for source of type "+descriptor.getName()+". Please check component.xml.");
			throw new CreationException(e);
		}
	}
	
}
