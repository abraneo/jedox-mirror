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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.prototype;

import java.io.StringWriter;
import java.util.List;
import java.util.Properties;

import org.jdom.Document;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.util.CustomClassLoader;

public class PrototypeGenerator {
		
	private static PrototypeGenerator instance = new PrototypeGenerator();
		
	public static PrototypeGenerator getInstance() {
		return instance;
	}
	
	public IPrototype getPrototypeGenerator(String prototypeName) throws ConfigurationException {
		List<ComponentDescriptor> descriptors = ComponentFactory.getInstance().getComponentDescriptors("prototypes");
		if (descriptors.isEmpty()) 
		   throw new ConfigurationException("No Prototype generators found");
		for (ComponentDescriptor descriptor : descriptors) {
			if (descriptor.getName().equalsIgnoreCase(prototypeName)) {
				String classname=descriptor.getClassName();
				try {
					Class<?> prototypeClass = CustomClassLoader.getInstance().loadClass(classname);
					return (IPrototype) prototypeClass.newInstance();
				}	
				catch (Exception e) {
					throw new ConfigurationException("Error initializing prototype generator "+classname+" "+e.getMessage());
				}
				
			}
		}
		throw new ConfigurationException("No prototype generator found for scenario "+prototypeName);
		
	}

	private String getProjectString(Document project) throws ConfigurationException {
		StringWriter writer = new StringWriter();
		try {
			XMLOutputter outputter = new XMLOutputter(Format.getPrettyFormat());
			outputter.output(project, writer);
		} catch (Exception e) {
			throw new ConfigurationException(e);
		}
		
		return writer.toString();
	}
		
	
	public String generate(String prototypeName, String projectName, Properties parameters) throws ConfigurationException {
		IPrototype prototype = getPrototypeGenerator(prototypeName);
		return getProjectString(prototype.generate(projectName,parameters));
	}
	
}
