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
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.DimensionInfo;

/**
 * Interface that represents a dimension in database
 * @author khaddadin
 *
 */
public interface IDimension {

	public enum DimensionType{
		DIMENSION_NORMAL, DIMENSION_SYSTEM, DIMENSION_ATTRIBUTE,DIMENSION_USERINFO
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
	 * Get the dimension type. Normal, attribute, system or  userinfo dimension.
	 * @return type {@link DimensionType}
	 */
	public DimensionType getType();

	/**
	 * Get the dimension info object. It includes information like 	maximumLevel, maximumIndent and maximumDepth;
	 * @return dimension info object {@link DimensionInfo}
	 * @throws PaloException
	 */
	public IDimensionInfo getDimensionInfo() throws PaloException;

	/**
	 * Get the list of elements in the dimension
	 * @param withAttributes true get the element attribute information as well, false this information will be ignored.
	 * @return list of elements {@link IElement}
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
	 * Get an element using its name
	 * @param name name of the element
	 * @param withAttributes  true get the element attributes information as well, false this information will be ignored.
	 * @return element object
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement getElementByName(String name,boolean withAttributes) throws PaloException, PaloJException;

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
	 * Create a list of elements
	 * @param name the name of the new element
	 * @param type the type of the new element
	 * @return the newly created Element
	 * @throws PaloException 
	 * @throws PaloJException 
	 */
	public IElement addBaseElement(String name, ElementType type) throws PaloJException, PaloException;

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
	 * Writes the attribute values for these element
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
	 * get a hashmap with each element name and the list of children
	 * @return map
	 */
	public Map<String,IElement[]> getChildrenMap() throws PaloException;
	
	/**
	 * get a hashmap with each element name and a map that map its' attributes names to their corresponding values for this element
	 * @return map
	 */
	public Map<String,HashMap<String,Object>> getAttributesMap() throws PaloException;
	
	/**
	 * get a hashmap with each element name and a map that map its' parent names to it's weight
	 * @return map
	 */
	public Map<String, HashMap<String, Double>> getWeightsMap() throws PaloException;
	
	/**
	 * set the cache to be trusted (i.e. no need to check server tokens) for a certain amount of time. 
	 * Be careful! changes from outside the palojlib API on the OLAP server will not be considered.
	 * The value will be reseted as soon as a write API call is done with palojlib.
	 * @param seconds number of seconds that the cache will be trusted
	 */
	public void setCacheTrustExpiry(int seconds);
	
	/**
	 * Get the list of the bases elements in the dimension
	 * @param withAttributes true get the element attributes information as well, false this information will be ignored.
	 * @return list of root elements
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IElement[] getBasesElements(boolean withAttributes) throws PaloException, PaloJException;


}
