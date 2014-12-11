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
package com.jedox.etl.core.component;

import com.jedox.etl.core.config.IConfigurator;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;

import java.util.ArrayList;

import org.jdom.Element;

/**
 * ETL-Server follows an extensible component architecture.
 * This interface defines all abstract functionality common to all components.  
 * <p>
 * A component has to registered in a component registry held by the {@link ComponentFactory}, which is responsible for instantiating all components.
 * Components may be created for all types defined in {@link ITypes.Components}. Each component type correspond to an interface this component has to mandatory implement when registered in this scope.
 * Generally components may decide to implement more than one such interface. In this case they may be registered in multiple scopes. 
 * </p>
 * <p>
 * The counterpart to the component concept is the {@link IManager manager} concept, which may hold and manage multiple components of the same type.
 * Components in turn may register multiple managers, each one for a particular type of sub-components.
 * The registration of external dependent sub-components in managers is required to support the correct calculation of dependencies necessary for component {@link com.jedox.etl.core.execution.IExecutor execution}. 
 * Only sub-components, which are strictly local to this component and thus only are used by this component may omitted from manager registration. 
 * </p>
 * <p>
 * All components configure themselves in the {@link #init() initialization} phase with the help of {@link IConfigurator configurators}, which translate the component mark-up (XML) to suitable internal data-structures.  
 * </p>
 * <p>
 * All components are {@link ILocatable} locatable via their {@link Locator locators}. Requesting a component from the {@link com.jedox.etl.core.config.ConfigManager} is the recommended way of localizing components, since the ConfigManager always fetches a fresh non-dirty instance. 
 * A component is considered dirty, when the configuration of the component itself or the configuration of any registered sub-component has changed. Additionally components may be set dirty by application logic to enforce a new generation after some special operations.    
 * </p>    
 * <p>
 * The component life-cycle includes four distinct phases:
 * <ul>
 * 	<li>Creation phase: the implementing class is determined and instantiated. The configurator class is initialized</li>
 *  <li>Configuration phase: the configurator class tries to configure itself accordingly to the xml description</li>
 *  <li>Initialization phase: the component tries to initialize itself with the data structures produced by the configurator</li>
 *  <li>Runtime phase: the component gets in action and extracts / transforms / loads data</li>
 * </ul>
 * </p>
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IComponent extends ILocatable, Comparable<IComponent> {
	
	/**
	 * Common method to put all component specific initialization code into. Typically the component uses this method to configure itself with the help of the provided configurator class.
	 * @throws InitializationException if the component cannot be properly initialized.
	 */
	public void initialize() throws InitializationException;
	/**
	 * Provides access to the component specific configurator class, which is responsible for providing methods to access the components configuration defined in XML.  
	 * @return the configurator
	 */
	public IConfigurator getConfigurator();
	/**
	 * Provides access to a manager managing sub components of a given {@link ITypes.Components type}.
	 * @param type the type of manager to return. Usually the plural of the component types managed.
	 * @return the manager of the given type. NULL if no manager of this type is available.
	 */
	public IManager getManager(String type); 
	/**
	 * Gets all managers of this component.
	 * @return the defined managers.
	 */
	public IManager[] getManagers();
	/**
	 * Recursively gets all components, which this component is dependent on. Dependencies are all components of all managers of this components including their respective dependencies.   
	 * @return a list of dependent components.
	 */
	public ArrayList<IComponent> getDependencies();
	/**
	 * Recursively gets all components, which this component is dependent on, but does not further descend into components in the given list. Used internally to avoid cyclic dependences.
	 * @param list a list of already resolved dependencies.
	 * @param recursive if true also consider transitive dependencies. if false only consider direct dependencies.
	 * @return a list of dependent components.
	 */
	public ArrayList<IComponent> getDependencies(ArrayList<IComponent> list, boolean recursive);
	
	/**
	 * Marks this component as dirty. When a component is dirty, it is rebuilt from configuration the next time it is requested from the {@link com.jedox.etl.core.config.ConfigManager}. Existing references from local managers stay intact until then, so a running {@link com.jedox.etl.core.execution.Execution Execution} is not affected.  
	 */
	public void setDirty();
	/**
	 * Checks if this component was set dirty and should be possibly renewed from a fresh config.
	 * @param checkDependencies if set the component is considered dirty if any dependent component is.
	 * @return true if dirty, false otherwise.
	 */
	public boolean isDirty(boolean checkDependencies);
	
	/**
	 * Test the component in runtime
	 * @throws RuntimeException whenever the test fails
	 */
	public void test() throws RuntimeException;
	
	
	/**
	 * Gets the component output specification as Row
	 * @return the Row holding the output columns
	 */
	public Row getOutputDescription() throws RuntimeException;	
	
	public Element getComponentDescription() throws ConfigurationException;
	
	public void setConfigurator(IConfigurator configurator);
	
	public IProcessor initProcessor(IProcessor processor, Facets facet) throws RuntimeException;
	
	public ITreeProcessor initTreeProcessor(ITreeProcessor processor, IProcessor.Facets facet) throws RuntimeException;

}
