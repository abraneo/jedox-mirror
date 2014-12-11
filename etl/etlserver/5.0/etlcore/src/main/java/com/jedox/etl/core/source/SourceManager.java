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

import java.util.LinkedList;
import java.util.List;
import org.jdom.Element;

import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Manager Class for DataSources
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class SourceManager extends Manager {
	private LinkedList<ISource> sources = new LinkedList<ISource>();
	private boolean allowDuplicates = false;
	
	public void setAllowDuplicates(boolean allowDuplicates) {
		this.allowDuplicates = allowDuplicates;
	}
	
	/**
	 * adds a dataSource
	 * @param datasource object
	 */
	public ISource add(IComponent datasource) throws RuntimeException {
		if (datasource instanceof ISource) {
			ISource old = (ISource) super.add(datasource);
			if (!allowDuplicates) sources.remove(old);
			sources.add((ISource)datasource);
			return old;
		}
		throw new RuntimeException("Failed to add non Source object");
	}
	
	/**
	 * gets the datasource by name
	 * @param name the source name
	 * @return the source object (if registered)
	 */
	public ISource get(String name) {
		return (ISource) super.get(name);
	}
	
	public ISource remove(String name) {
		ISource source = (ISource) super.remove(name);
		if (source != null) {
			sources.remove(source);
		}
		return source;
	}
	
	/**
	 * gets all managed datasources 
	 */
	public ISource[] getAll() {
		return sources.toArray(new ISource[sources.size()]);
	}
	
	/**
	 * gets processors for all registered sources. (convenience method)
	 * @return a list of processors
	 * @throws RuntimeException
	 */
	public List<IProcessor> getProcessors() throws RuntimeException {
		LinkedList<IProcessor> processors = new LinkedList<IProcessor>();
		for (ISource s : sources) {
			processors.add(s.getProcessor());
		}
		return processors;
	}
	
	/**
	 * gets all registered sources in a list. (convenience method)
	 * @return a list of all rgistered sources
	 */
	public List<ISource> getSources() {
		LinkedList<ISource> tables = new LinkedList<ISource>();
		for (ISource s : sources) {
			tables.add(s);
		}
		return tables;
	}
	
	/**
	 * gets a list of all registered tree based sources.
	 * @return a list of tree based sources
	 */
	public List<ITreeSource> getTreeSources() {
		LinkedList<ITreeSource> trees = new LinkedList<ITreeSource>();
		for (ISource s : sources) {
			if ((s instanceof ITreeSource))
				trees.add((ITreeSource)s);
		}
		return trees;
	}
	
	/**
	 * gets a list of all registered views
	 * @return a list of views
	 */
	public List<IView> getViews() {
		LinkedList<IView> views = new LinkedList<IView>();
		for (ISource s : sources) {
			if ((s instanceof IView))
				views.add((IView)s);
		}
		return views;
	}
	
	/**
	 * gets the position of a source in this manager.
	 * @param name the name of the source
	 * @return the source position in the internal list.
	 */
	public int indexOf(String name) {
		return sources.indexOf(get(name));
	}
	
	public ISource add(Element datasource) throws CreationException, RuntimeException {
		String type = datasource.getAttributeValue("type");
		ISource ds = ComponentFactory.getInstance().createExtract(type, this, getContext(), datasource);
		add(ds);
		return ds;
	}

	public String getName() {
		return ITypes.Sources;
	}
	
	/**
	 * invalidates all sources registered in this manager.
	 */
	public void invalidate() {
		for (ISource s : sources) {
			s.invalidate();
		}
	}
	
	public void clear() {
		super.clear();
		sources.clear();
	}

}
