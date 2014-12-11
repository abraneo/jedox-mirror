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
import java.util.Properties;

import com.jedox.etl.core.context.IContext;

/**
 * Interface for the basic common localization functionality of {@link IComponent components} and {@link IManager managers}.
 * Each Locatable thus has a unique address in project space reflecting the component-manager hierarchy.  
 * These address is called a {@link Locator} and serves as unique ID suitable for searching for and identifying Locatables.  
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface ILocatable {
	/**
	 * Gets the name of the Locatable, which is the last part of its Locator.
	 * @return the name
	 */
	public String getName();
	
	/**
	 * Gets all parameters / properties set for this Locatable
	 * @return a Properties object holding the parameters.
	 */
	public Properties getParameter();
	/**
	 * Gets the {@link com.jedox.etl.core.context.IContext Context} object, describing the context this Locatable was created in. 
	 * @return the Locatable representing the context.
	 */
	public IContext getContext();
	/**
	 * Gets the name of the {@link com.jedox.etl.core.context.IContext Context} this Locatable. Serves as convenience method. 
	 * @return The name of the context. When no context is set "default" is returned. 
	 */
	public String getContextName();
	/**
	 * Sets the Locator and Context of this locatable.
	 * @param locator the locator object
	 * @param locatable the context object
	 */
	public void setLocator(Locator locator, IContext locatable);
	/**
	 * Gets the Locator of the Locatable describing its absolute address in the component project space. e.g "myProject.jobs.myJob" for a job with name "myJob" in the project "myProject"
	 * @return the Locator
	 */
	public Locator getLocator();
}
