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
package com.jedox.etl.core.config.transform;

import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.ViewSource;

/**
 * Helper class for the configuration of transforms.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TransformConfigUtil {

	private Element xml;
	private Locator locator;
	private IContext context;
	private HashMap<String,IComponent> sources;
	private ArrayList<IComponent> functions;
	private ArrayList<IComponent> orderedSources;

	public TransformConfigUtil(Element xml, Locator locator, IContext context) {
		this.xml = xml;
		this.locator = locator.clone().add(xml.getAttributeValue("name"));
		this.context = context;
	}

	protected Locator getLocator() {
		return locator.clone();
	}

	protected Element getXML() {
		return xml;
	}

	/**
	 * gets the sources specified as input for this transform. Order is as specified in the XML
	 * @return the sources
	 * @throws ConfigurationException
	 */
	public List<IComponent> getSources() throws ConfigurationException {
		getSourcesLookup();
		return orderedSources;
	}

	/**
	 * gets the sources used as input mapped by their original name. This is useful, when views use other names than the underlying sources. At the moment this is not the case
	 * @return the sources mapped by their original name
	 * @throws ConfigurationException
	 */
	public HashMap<String,IComponent> getSourcesLookup() throws ConfigurationException {
		if (sources == null) {
			orderedSources = new ArrayList<IComponent>();
			sources = new HashMap<String,IComponent>();
			Element datasources = getXML().getChild("sources");
			List<?> ds = datasources.getChildren("source");
			for (int j=0; j<ds.size(); j++) {
				Element e = (Element) ds.get(j);
				String sourceName = e.getAttributeValue("nameref");
				if (locator.getName().equals(sourceName))
					throw new ConfigurationException("Circular reference in transform "+locator.getName());
				try {
					ISource viewSource = SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), context, context, e);
					sources.put(sourceName, viewSource);
					orderedSources.add(viewSource);
				}
				catch (CreationException ex) {
					throw new ConfigurationException(ex.getMessage());
				}
			}
		}
		return sources;
	}

	/**
	 * gets the functions defined for this transform
	 * @return the functions
	 * @throws ConfigurationException
	 */
	public List<IComponent> getFunctions() throws ConfigurationException {
		if (functions == null) {
			functions =  new ArrayList<IComponent>();
			Element transforms = getXML().getChild("functions");
			if (transforms != null) {
				List<?> ds = transforms.getChildren("function");
				for (int j=0; j<ds.size(); j++) {
					Element transformer = (Element) ds.get(j);
					String name = transformer.getAttributeValue("name");
					IComponent function = context.getComponent(getLocator().add(ITypes.Functions).add(name));
					if(functions.contains(function))
						throw new ConfigurationException("Function " + name + " is defined more than once.");
					functions.add(function);
				}
			}
		}
		return functions;
	}

	/*
	public void checkSource(String name) throws ConfigurationException {
		for (IComponent s : getSources())
			if (s.getName().equals(name)) return;
		throw new ConfigurationException("Error in "+locator.getType()+" "+locator.getName()+": Referenced source "+name+" not found.");
	}
	*/
	/*
	public void checkInput(String name) throws ConfigurationException, RuntimeException {
		ColumnManager c = getColumns();
		if (!c.getRow().containsColumn(name))
			throw new ConfigurationException("Error in "+locator.getType()+" "+locator.getName()+": Referenced input "+name+" not found.");
	}
	*/

}
