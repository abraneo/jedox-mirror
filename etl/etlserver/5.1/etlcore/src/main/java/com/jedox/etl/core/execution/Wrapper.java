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
package com.jedox.etl.core.execution;

import java.util.ArrayList;
import java.util.Properties;

import org.jdom.Element;

import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.context.IContext;

/**
 * Abstract wrapper class for providing the {@link IExecutable} Interface for components initially not implementing it and / or enabling them to perform some runtime action in a save {@link Execution} environment. This runtime action is arbitrary and has to be defined by implementing classes in the {@link #execute()} method.  
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Wrapper implements IExecutable {
	
	private IComponent component;	
//	private ExecutionState state;   // Execution state is available via the Context
		
	public Wrapper(IComponent component) {
		this.component = component;
	}
	
	protected IComponent getComponent() {
		return component;
	}

	/**
	 * here the desired runtime functionality should be implemented, typically by delegation to another method. 
	 */
	public abstract void execute(); 

	public ExecutionState getState() {
		return getContext().getState();
	}

	public boolean isExecutable() {
		return getState().isExecutable();
	}

/*
	public void setState(ExecutionState state) {
		this.state = state;
	}
*/	

	public IConfigurator getConfigurator() {
		return component.getConfigurator();
	}

	public ArrayList<IComponent> getDependencies() {
		ArrayList<IComponent> result = component.getDependencies();
		result.add(0, component);
		return result;
	}

	public ArrayList<IComponent> getDependencies(ArrayList<IComponent> list, boolean recursive) {
		return component.getDependencies(list,recursive);
	}

	public IManager getManager(String type) {
		return component.getManager(type);
	}

	public IManager[] getManagers() {
		return component.getManagers();
	}

	public Row getOutputDescription()
			throws RuntimeException {
		return component.getOutputDescription();
	}

	public void initialize() throws InitializationException {
		component.initialize();
	}

	public boolean isDirty(boolean checkDependencies) {
		return component.isDirty(checkDependencies);
	}

	public void setDirty() {
		component.setDirty();
	}

	public void test() throws RuntimeException {
		component.test();
	}

	public IContext getContext() {
		return component.getContext();
	}

	public String getContextName() {
		return component.getContextName();
	}

	public Locator getLocator() {
		return component.getLocator();
	}

	public String getName() {
		return component.getName();
	}

	public Properties getParameter() {
		return component.getParameter();
	}

	public void setLocator(Locator locator, IContext locatable) {
		component.setLocator(locator, locatable);
	}
	
	public Element getComponentDescription() throws ConfigurationException {
		return component.getComponentDescription();
	}
	
	public void setConfigurator(IConfigurator configurator) {
		component.setConfigurator(configurator);
	}
	
	public int compareTo(IComponent argument) {		   
	   return component.compareTo(argument);
	}
	
	public IProcessor initProcessor(IProcessor processor, Facets facet) throws RuntimeException {
		return component.initProcessor(processor, facet);
	}
	
	public ITreeProcessor initTreeProcessor(ITreeProcessor processor, Facets facet) throws RuntimeException {
		return component.initTreeProcessor(processor, facet);
	}
}
