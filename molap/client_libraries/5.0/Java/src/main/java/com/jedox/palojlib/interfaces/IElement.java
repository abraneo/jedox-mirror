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
 * Interface that represents an Element in dimension
 * @author khaddadin
 *
 */
public interface IElement {

	public static enum ElementType{
		ELEMENT_STRING, ELEMENT_NUMERIC, ELEMENT_CONSOLIDATED
	}

	/**
	 * Get the name of the element
	 * @return name
	 */
	public String getName();

	/**
	 * Get the type of the element, numeric, string, consolidated
	 * @return type
	 */
	public ElementType getType();

	/**
	 * Get the value of the attribute for this element
	 * @param attributeName
	 * @return the value
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
	 * Get a hashmap with the element and its children, and  all the elements in its subtree with their corresponding children.
	 * @return hashmap that maps an element name to a list of children element
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public Map<String,IElement[]>  getSubTree() throws PaloException, PaloJException;

	/**
	 * Get a hashmap with the element and its attributes-values map, and  all the elements in its subtree with their corresponding attributes-values maps..
	 * @return hashmap that maps an element name to a map that contains
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public Map<String,HashMap<String,Object>>   getSubTreeAttributes() throws PaloException, PaloJException;
	
	/**
	 * rename an element
	 * @param newname new element name
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void rename(String newname) throws PaloException, PaloJException;

}
