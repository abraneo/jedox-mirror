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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Interface for the unification of table based and tree based sources. Tree based sources may be rendered in multiple table-equivalent classes. This is, what a view is responsible for.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IView {
	
	/**
	 * The formats available to be rendered by a view
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum Views {
		/**
		 * Full Hierarchy 
		 */
		FH, 
		/**
		 * Full Hierarchy weighted
		 */
		FHW, 
		/**
		 * Palo NC Format
		 */
		NCW, 
		/**
		 * Palo NC Format with Attributes
		 */
		NCWA, 
		/**
		 * Parent Child format 
		 */
		PC, 
		/**
		 * Parent Child Weighted
		 */
		PCW,
		/**
		 * Parent Child Weighted with Attributes
		 */
		PCWA, 
		/**
		 * Parent Child Weighted with Attributes plus column Type
		 */
		PCWAT,
		/**
		 * Element Attribute format
		 */
		EA, 
		/**
		 * Level Element Weight Type format
		 */
		LEWT, 
		/**
		 * Level Element Weight Type Attribute format
		 */
		LEWTA, 		
		/**
		 * Format not specified
		 */
		NONE;
	}
	
	/**
	 * gets the default processor for this view. Format is specified by configuration or native format
	 * @return a processor on this view
	 * @throws RuntimeException
	 */
	public IProcessor getProcessor() throws RuntimeException;
	/**
	 * gets a processor for this view complying to the specified rendering format
	 * @param view the view rendering format
	 * @return a processor on this view
	 * @throws RuntimeException
	 */
	public IProcessor getProcessor(Views view) throws RuntimeException;
	/**
	 * Determines if the underlying datasource is tree based. View formats may only be used, if this is true.
	 * @return true, if so. False if backed by a table based source.
	 */
	public boolean isTreeBased();
	/**
	 * gets the underlying base source of this view
	 * @return the underlying base source.
	 */
	public ISource getBaseSource();
	/**
	 * gets the tree representation of the source. Implicitly converts table sources to tree sources with the given format, if needed. 
	 * @param format the conversion format. Use NONE for no implicit conversion.
	 * @return the TreeManager holding the tree
	 * @throws RuntimeException
	 */
	public ITreeManager renderTree(IView.Views format) throws RuntimeException;

}
