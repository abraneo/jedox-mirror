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
package com.jedox.etl.core.project;

import org.jdom.Element;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ILocatable;
import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.config.ConfigValidator;

public class ProjectFactory {

	private static ProjectFactory instance = new ProjectFactory();
	private static final Log log = LogFactory.getLog(ProjectFactory.class);
	
	public ProjectFactory() {
	}
	
	/**
	 * gets the static instance
	 * @return the ProjectFactoryInstance
	 */
	public static ProjectFactory getInstance() {
		return instance;
	}
	
	/**
	 * instantiates a new project.
	 * @param descriptor the descriptor holding the static configuration from a component.xml
	 * @param parent the parent object of the project, usually the project manager
	 * @param config the dynamic configuration of the project given by the defining project xml
	 * @return the newly created project instance.
	 * @throws CreationException
	 */
	public IProject newProject(ComponentDescriptor descriptor, ILocatable parent, Element config) throws CreationException {
		try {
			Class<?> projectClass = Class.forName(descriptor.getClassName());
			ConfigValidator.getInstance().validate(projectClass, config);
			IProject p = (IProject) projectClass.newInstance();
			//p.getConfigurator().setLocator(parent.getLocator());
			p.getConfigurator().addParameter(descriptor.getParameters());
			p.getConfigurator().setXML(config);
			p.init();
			return p;
		}
		catch (Exception e) {
			log.debug("Failed to create project "+config.getAttributeValue("name")+".");
			throw new CreationException(e);
		}
	}
	
}
