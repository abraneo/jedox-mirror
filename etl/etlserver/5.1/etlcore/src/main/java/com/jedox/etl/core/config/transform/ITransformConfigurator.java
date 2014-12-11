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

import java.util.List;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;

/**
 * Common interface for all Pipeline Configurators.
 * Bundles all functionality common to all Pipeline types.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface ITransformConfigurator {
	/**
	 * gets the ColumnManager, which holds the output column definition of the pipeline
	 * @return the ColumnManager
	 */
	public Row getRow() throws RuntimeException;
	/**
	 * Gets all Sources, which are defined as data sources for the pipeline
	 * @return A List of Sources.
	 * @throws ConfigurationException if the sources are faulty or non existent.
	 */
	public List<IComponent> getSources() throws ConfigurationException;
	/**
	 * Gets all Transformers, which are defined in the pipeline
	 * @return A List of Transformations
	 * @throws ConfigurationException if the transformers are faulty or non existent
	 */
	public List<IComponent> getFunctions() throws ConfigurationException;

}
