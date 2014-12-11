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
import com.jedox.palojlib.interfaces.IElement.ElementType;

/**
 * Interface that represents a dimension in database
 * @author khaddadin
 *
 */
public interface IDimension {

	/**
	 * olap dimension types
	 * @author khaddadin
	 *
	 */
	public static enum DimensionType{
		/**
		 * normal dimension
		 */
		DIMENSION_NORMAL, 
		/**
		 * system dimension 
		 */
		DIMENSION_SYSTEM, 
		/**
		 * attribute dimension
		 */
		DIMENSION_ATTRIBUTE,
		/**
		 * userinfo dimension
		 */
		DIMENSION_USERINFO, 
		/**
		 * systemid dimension
		 */
		DIMENSION_SYSTEM_ID
	}

	/**
	 * Get the name of the dimension
	 * @return name of the dimension
	 */
	public String getName();

	/**
	 * Get the ID of the Dimension
	 * @return Dimension ID
	 */
	public int getId();

	/**
	 * Get the dimension type  {@link DimensionType}
	 * @return type
	 */
	public DimensionType getType();

	/**
	 * Get the dimension info object {@link IDimensionInfo}
	 * @return dimension info object 
	 * @throws PaloException
	 */
	public IDimensionInfo getDimensionInfo() throws PaloException;

	/**
	 * Get the list of elements in the dimension
	 * @param withAttributes true get the element attribute information as well, false this information will be ignored.
	 * @return list of elements
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement[] getElements(boolean withAttributes) throws PaloException, PaloJException;

	/**
	 * Get the list of the root elements in the dimension
	 * @param withAttributes true get the element attributes information as well, false this information will be ignored.
	 * @return list of root elements
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement[] getRootElements(boolean withAttributes) throws PaloException, PaloJException;

	/**
	 * Get the list of the bases elements in the dimension
	 * @param withAttributes true get the element attributes information as well, false this information will be ignored.
	 * @return list of root elements
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement[] getBasesElements(boolean withAttributes) throws PaloException, PaloJException;

	/**
	 * Get an element using its name
	 * @param name name of the element
	 * @param withAttributes  true get the element attributes information as well, false this information will be ignored.
	 * @return element object
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement getElementByName(String name,boolean withAttributes) throws PaloException, PaloJException;
	
	/**
	 * Get an array of elements using their names, an exception will be thrown if at least one of them does not exist
	 * @param name names array of the elements
	 * @param withAttributes  true get the element attributes information as well, false this information will be ignored.
	 * @return element array object
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement[] getElementsByName(String[] name,boolean withAttributes) throws PaloException, PaloJException;

	/**
	 * Get an attribute using its name
	 * @param name name of the attribute
	 * @return attribute object
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IAttribute getAttributeByName(String name) throws PaloException, PaloJException;

	/**
	 * Create a list of elements
	 * @param names the names of the new elements
	 * @param types the types of the new elements
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void addElements(String[] names, ElementType[] types) throws PaloException, PaloJException;
	
	/**
	 * update the types of the elements, only ElementTypes type string and numeric can be used {@link ElementType}.
	 * Consolidated elements can not be included in the given list of elements.
	 * @param elements the new elements
	 * @param type string or numeric
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void updateElementsType(IElement[] elements, ElementType type) throws PaloException, PaloJException;
	
	
	/**
	 * Create one element, this method does not invalidate the cache.
	 * @param name the name of the new element
	 * @param type the type of the new element
	 * @return the newly created Element
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public IElement addBaseElement(String name, ElementType type) throws PaloJException, PaloException;
	
	/**
	 * Create one attribute, this method does not invalidate the cache.
	 * @param name the name of the new attribute
	 * @param type the type of the new attribute
	 * @return the newly created attribute
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public IAttribute addAttribute(String name, ElementType type) throws PaloJException, PaloException;

	/**
	 * Create new attributes in this dimension
	 * @param names names of the new attributes
	 * @param types types of the new attributes
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void addAttributes(String[] names, ElementType[] types) throws PaloJException, PaloException;

	/**
	 * removes the elements
	 * @param elements names of the elements to be removed
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public void removeElements(IElement[] elements) throws PaloException, PaloJException;

	/**
	 * Removes the attributes
	 * @param attributes names of the attributes to be removed
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public void removeAttributes(IAttribute[] attributes) throws PaloException, PaloJException;

	/**
	 * Removes the attribute values for these element in this dimension
	 * @param attribute attribute
	 * @param elements list of elements
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void removeAttributeValues(IAttribute attribute, IElement [] elements) throws PaloJException, PaloException;


	/**
	 * Writes the attribute values for these elements
	 * @param attribute
	 * @param elements
	 * @param values
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void addAttributeValues(IAttribute attribute, IElement [] elements, Object[] values) throws PaloJException, PaloException;

	/**
	 * remove the consolidations for these elements i.e. their children
	 * @param elements
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public void  removeConsolidations(IElement[] elements) throws PaloException, PaloJException;

	/**
	 * update the consolidation for the elements mentioned as parents in the consolidations set.
	 * @param consolidations
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public void  updateConsolidations(IConsolidation[] consolidations) throws PaloException, PaloJException;

	/**
	 * remove the consolidations for this attribute i.e. its children
	 * @param attribute
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public void  removeAttributeConsolidations(IAttribute attribute) throws PaloException, PaloJException;

	/**
	 * add a child for this attribute
	 * @param attribute
	 * @param child
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void  addAttributeConsolidation(IAttribute attribute, IAttribute child) throws PaloJException, PaloException;

	/**
	 * create a consolidation object, no change is yet done to the dimension
	 * @param parent
	 * @param child
	 * @param weight
	 * @return the consolidation object
	 */
	public IConsolidation newConsolidation(IElement parent,IElement child, double weight);

	/**
	 * Get a list of attributes for this dimension
	 * @return attributes
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public IAttribute[] getAttributes() throws PaloException, PaloJException;
	
	/**
	 * rename a dimension
	 * @param newname new dimension name
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void rename(String newname) throws PaloException, PaloJException;
	
	/**
	 * get a map that maps each element name to its children
	 * @return map
	 */
	public Map<String,IElement[]> getChildrenMap() throws PaloException;
	
	/**
	 * get a map that maps each element name to a sub map which maps the element attribute names to their corresponding values for that element
	 * @return map
	 */
	public Map<String,HashMap<String,Object>> getAttributesMap() throws PaloException;
	
	/**
	 * get a map that maps each element name to a sub map which maps the elements' parents to their weights for that element
	 * @return map
	 */
	public Map<String, HashMap<String, Double>> getWeightsMap() throws PaloException;
	
	/**
	 * set the cache to be trusted (i.e. no need to check server tokens) for a certain amount of time. 
	 * Be careful! changes from outside the palojlib API on the OLAP server will not be considered.
	 * The value will be reseted as soon as a write API call is done with palojlib, with exception 
	 * to methods {@link IDimension#addBaseElement(String, ElementType), {@link IDimension#addAttribute(String, ElementType)  
	 * @param seconds number of seconds that the cache will be trusted
	 */
	public void setCacheTrustExpiry(int seconds);
	
	/**
	 * check if at least one C-Element exists
	 * @return true, if at least one C-element exists
	 */
	public boolean hasConsolidatedElements();
	
	/**
	 * decide whether elements should be read with permission or not
	 * @param withPermission true means element permission will be read as well, default is false.
	 */
	public void setWithElementPermission(boolean withPermission);
	
	/**
	 * clear any cached information if exists, the expiry trust time will not be affected
	 */
	public void resetCache();
	
	/**
	 * clear all consolidations that exists in the dimension
	 * @return number of C-Elements that were converted to base element
	 */
	public int removeAllConsolidations();
	
	/**
	 * Add new elements with the given type or simply change type of existing elements 
	 * @param elements element to append
	 * @throws PaloException
	 * @throws PaloJException
	 */
	public void appendElements(IElement[] elements) throws PaloException, PaloJException;
	
	/**
	 * move a list of elements to new positions
	 * @param elements elements to move
	 * @param positions new positions
	 * @throws PaloJException 
	 * @throws PaloException 
	 */
	public void moveElements(IElement[] elements, Integer[] positions) throws PaloException, PaloJException;
	
	/**
	 * Get the a single element in the dimension, the dimension elements will not be read in this case
	 * @param elementName name of the element
	 * @return element if exists,otherwise null.
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement getSingleElement(String elementName, boolean withAttributes) throws PaloException, PaloJException;

}
