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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.context;

import java.util.Properties;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;

/**
 * Interface for Contexts to implement. A Context holds all necessary data structures for an {@link com.jedox.etl.core.execution.Execution Execution} to run thread save. Thus for every such execution a seperate and unique context is created. Additionally a (non-executable) default context holds all components for validation purpose.
 * All Components located in this contexts can be accessed by their according managers. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IContext extends IComponent {
	
	/**
	 * the name constant for the default context.
	 */
	public static final String defaultName = "default";
	/**
	 * determines, if this context is the default context or a runtime context
	 * @return true, if default and false if runtime
	 */
	public boolean isDefault();
	/**
	 * sets variables for this context, which may affect component configuration and runtime behavior.
	 * @param variables a set of properties serving as variables
	 */
	public void addVariables(Properties variables);
	/**
	 * gets the variables for this context, which may affect component configuration and runtime behavior.
	 * @param variables a set of properties serving as variables
	 */
	public Properties getVariables();
	
	public Properties getExternalVariables();
	/**
	 * clears this context and all references to the datastructures created in it.
	 */
	public void clear();
	/**
	 * sets the name of the context.
	 * @param name the context name
	 */
	public void setName(String name);
	/**
	 * adds a child context to this context, which inherits all variables from this context and which is also cleared when the parent context is cleared.
	 * @param child the child context
	 */
	public void addChildContext(IContext child) throws RuntimeException;
	/**
	 * gets the base context name, which is the name of a context defined in the project, where this context is deduced of.
	 * @return the base context
	 */
	public String getBaseContextName();
	/**
	 * sets the base context name, which is the name of a context defined in the project, where this context is deduced of.
	 * @param baseContextName
	 */
	public void setBaseContextName(String baseContextName);
	
	

}
