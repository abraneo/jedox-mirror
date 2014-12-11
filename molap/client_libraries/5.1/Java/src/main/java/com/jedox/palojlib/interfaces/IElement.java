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
 *   You may obtain a copy of the License at
*
 *   If you are developing and distributing open source applications under the
 *   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 *   ISVs, and VARs who distribute Palo with their products, and do not license
 *   and distribute their source code under the GPL, Jedox provides a flexible
 *   OEM Commercial License.
 *
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.interfaces;

import java.util.HashMap;
import java.util.Map;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;

/**
 * represent an Element in an olap dimension
 * @author khaddadin
 *
 */
public interface IElement {

	/**
	 * element types 
	 * @author khaddadin
	 *
	 */
	public static enum ElementType{
		/**
		 * string base elements
		 */
		ELEMENT_STRING, 
		/**
		 * numeric base element
		 */
		ELEMENT_NUMERIC, 
		/**
		 * consolidated element
		 */
		ELEMENT_CONSOLIDATED
	}
	
	/**
	 * element permission in olap. 
	 * @author khaddadin
	 *
	 */
	/* The sequence here is "very" important, please don't change */
	public static enum ElementPermission{
		/**
		 * none
		 */
		N,
		/** 
		 * read
		 */
		R,
		/**
		 * read+write
		 */
		W,
		/**
		 * read+write+execute
		 */
		D,
		/**
		 * splash
		 */
		S;
		
		/**
		 * check wither a permission is higher or equal to another permission
		 * @param permission
		 * @return true if higher or equal e.g D>W, false otherwise
		 */
		public boolean higherOrEqual(ElementPermission permission){
			
			if(permission.ordinal()<=this.ordinal())
				return true;
			
			return false;
		}
	}

	/**
	 * Get the name of the element
	 * @return name
	 */
	public String getName();

	/**
	 * Get the type of the element {@link ElementType}
	 * @return type
	 */
	public ElementType getType();

	/**
	 * Get the value of the attribute for this element
	 * @param attributeName
	 * @return the attribute value
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public Object getAttributeValue(String attributeName) throws PaloJException, PaloException;

	/**
	 * Get the children of the element
	 * @return children list
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public IElement[] getChildren() throws PaloException, PaloJException;

	/**
	 * Get the parents of the element
	 * @return parents list
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public IElement[]  getParents() throws PaloException, PaloJException;

	/**
	 * Get the number of children for this element
	 * @return number of children
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public int getChildCount() throws PaloException, PaloJException;

	/**
	 * Get the number of parents for this element
	 * @return number of parents
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public int getParentCount() throws PaloException, PaloJException;

	/**
	 * Get the weight of this element under this parent element
	 * @param parent
	 * @return double value
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public double getWeight(IElement parent) throws PaloException, PaloJException;

	/**
	 * Get a map that maps an element to its children, only the elements and its descendants will be included in the map.
	 * @return map that maps an element name to a list of children element
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public Map<String,IElement[]> getSubTree() throws PaloException, PaloJException;

	/**
	 * Get a map that maps an element to its attributes-values map, only the elements and its descendants will be included in the map.
	 * @return map that maps an element to its attributes-values map
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public Map<String,HashMap<String,Object>> getSubTreeAttributes() throws PaloException, PaloJException;
	
	/**
	 * rename an element
	 * @param newname new element name
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void rename(String newname) throws PaloException, PaloJException;
	
	/**
	 * move an element to a new position
	 * @param position new position
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void move(int position) throws PaloException, PaloJException;
	
	/**
	 * get the current position
	 * @return positive integer value
	 */
	public int getPosition();
	
	/**
	 * get the permission for this element {@link ElementPermission}.
	 * {@link IDimension#setWithElementPermission(boolean)} should be set to true before reading the elements
	 * @return permission for this element
	 */
	public ElementPermission getPermission();
	
	

}
