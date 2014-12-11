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
package com.jedox.etl.core.source;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Interface for all {@link com.jedox.etl.core.source.ISource Sources}, which are based an a tree representation.
 * Implementing sources may be rendered an multiple View formats to conform to the Row based source standard.
 * Additionally operations on the entire tree are available. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface ITreeSource extends ISource {
		
	/**
	 * Generates the internal Tree representation.
	 * @return the TreeManager given access to the Tree representation.
	 */
	public ITreeManager generate() throws RuntimeException;
	
	/**
	 * Returns a previously generated output (without re-generating it)
	 * @return The TreeManager holding the Tree as generated in the last preceeding {@link #generate()} call.
	 */
	public ITreeManager getTreeManager();
	
	/**
	 * Gets a Processor for the rendering of the tree in a table based format.
	 * @param view the format the tree should be rendered.
	 * @param includeRoot determines if the root node of the tree should be included in the rendering. 
	 * @return the processor for the rendered tree.
	 */
	public IProcessor getProcessor(Views view) throws RuntimeException;
	
	/**
	 * gets the description of all attributes defined. 
	 * @return Row holding a Column for each attribute
	 */
	public Row getAttributes() throws RuntimeException;
	
	
}
