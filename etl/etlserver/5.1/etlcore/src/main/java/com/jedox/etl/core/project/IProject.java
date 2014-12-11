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

import java.util.List;
import java.util.Map;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.context.ContextManager;

/**
 * Interface for a Project to implement. A project is a high level container for ETL-processes and may contain {@link com.jedox.etl.core.context.IContext Variables}, {@link com.jedox.etl.core.connection.IConnection Connections}, {@link com.jedox.etl.core.source.ISource Sources}, {@link com.jedox.etl.core.transform.ITransform Pipelines}, {@link com.jedox.etl.core.load.ILoad Exports} and {@link com.jedox.etl.core.job.IJob Jobs}.
 * A Project is self contained in the sense, that structurally it does not depend on Components defined in other projects. However a Project may be dependent on other Projects in the sense that data-structures built by these other Projects in external back-ends may be required.   
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IProject extends IComponent {
	
	/**
	 * Defines the state of the XML-Project declaration
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 * @author Andreas Fröhlich
	 *
	 */
	public static enum Declaration  {
		/**
		 * the XML declaration is complete and in a format ready for validation. No conversion needs to be done.
		 */
		strict, 
		/**
		 * the XML declaration my be incomplete. Before validation a conversion to STRICT format is done. 
		 */
		lazy
	}
		
	/**
	 * Gets the ContextManager holding the contexts of this project. Each Context holds a set of variables and all Runtime Components initialized in this context. A Context called "default" does always exist and holds the default settings for all variables specified. Additional context are created when needed on runtime. 
	 * @return the ContextManager holding the currently available contexts.
	 */
	public ContextManager getContextManager();
	
	/**
	 * gets all model components (connections, extracts, transforms, loads, jobs)
	 * @return a list of all model components
	 */
	public List<IComponent> getModelComponents(); 
	
	/**
	 * gets the full component dependency map
	 * @param mode the dependency mode 
	 * @param includeVariables 
	 * @return the map mapping a list of direct dependencies for each component.
	 */
	public Map<String,List<String>> getDependencyMap(boolean includeVariables);
	
	/**
	 * gets the dependencies of a component 
	 * @param qname the qualified name of the component
	 * @param includeVariables 
	 * @param mode the dependency mode  
	 * @return the map mapping a list of direct dependencies for each component involved.
	 */
	
	public List<String> getDirectDependencies(String qname, boolean includeVariables);

	public Map<String,List<String>> getAllDependencies (String qname, boolean includeVariables);
	
	/**
	 * gets the dependent components of a component 
	 * @param qname the qualified name of the component
	 * @param mode the dependency mode
	 * @return the map mapping a list of direct dependent components for each component involved.
	 */
	public List<String> getDirectDependents(String qname, boolean includeVariables);

	public Map<String,List<String>> getAllDependents(String qname, boolean includeVariables);
	
	/**
	 * Deletes all internal caches, so that all data is rebuilt on the next request.
	 * @throws Exception 
	 */
	public void invalidate() throws Exception;
		
}
