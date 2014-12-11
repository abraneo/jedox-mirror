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
package com.jedox.etl.core.aliases;

import java.util.ArrayList;

import org.jdom.Element;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.ConfigValidator;
import com.jedox.etl.core.context.IContext;

public class AliasMapFactory {
	
	private static final AliasMapFactory instance = new AliasMapFactory();
	private static final Log log = LogFactory.getLog(AliasMapFactory.class);
	
	public AliasMapFactory() {
	}
	
	/**
	 * gets the static instance
	 * @return the alias map factory
	 */
	public static final AliasMapFactory getInstance() {
		return instance;
	}

	
	/**
	 * creates a new object implementing the {@link IAliasMap} interface.
	 * @param className the name of the implementing class.
	 * @param parent the parent of this alias map. Usually an {@link AliasMapManager} or a {@link com.jedox.etl.core.source.ISource Source}
	 * @param context the {@link com.jedox.etl.core.context.IContext context} to create this alias map in.
	 * @param config the xml config for this alias map 
	 * @return the new alias map.
	 * @throws CreationException
	 */
	public IAliasMap newAliasMap(String className, ILocatable parent, IContext context, Element config) throws CreationException {
		try {
			Class<?> mapClass = Class.forName(className);
			ConfigValidator.getInstance().validate(mapClass, config);
			IAliasMap m = (IAliasMap) mapClass.newInstance();
			if (context == null) //use default context
				context = ConfigManager.getInstance().getContext(parent.getLocator().getRootName(), IContext.defaultName);
			m.getConfigurator().addParameter(parent.getParameter());
			m.getConfigurator().setLocator(parent.getLocator(),context);
			m.getConfigurator().setXML(config, new ArrayList<String>());
			m.initialize();
			//log.info("Created AliasMap "+m.getName());
			return m;
		}
		catch (Exception e) {
			log.debug("Failed to create AliasMap.");
			throw new CreationException(e);
		}
	}

}
