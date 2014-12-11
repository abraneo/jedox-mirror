/**
 *
 */

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
 *	Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.main;

import java.util.ArrayList;
import java.util.HashMap;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.util.Helpers;

public class Element implements IElement {

	private final int id;
	private final ElementHandler elementhandler;
	private final Database database;
	private final int dimensionId;

	private String name;
	private ElementType type;
	private int[] parentsIds;
	private int[] childrenIds;
	private double[] childrenweights;
	protected double weight;
	private HashMap<String, Object> attributeValues;
	private HashMap<String, Element> attributes;
	private int dimensionToken;
	protected HashMap<Integer, Element> childrenElements;

	protected Element(String contextId, Database database, int dimensionId, int id, String name, ElementType type, int[] parentsIds, int[] childrenIds,
			double[] weights, HashMap<String, Object> attributesValues,HashMap<String, Element> attributes, int dimensionToken) throws PaloException, PaloJException {
		elementhandler = new ElementHandler(contextId);
		this.database = database;
		this.dimensionId = dimensionId;
		this.name = name;
		this.id = id;
		this.type = type;
		this.parentsIds = parentsIds;
		this.childrenIds = childrenIds;
		this.childrenweights = weights;
		this.attributeValues = attributesValues;
		this.dimensionToken = dimensionToken;
		this.attributes = attributes;
		childrenElements = null;
	}

	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}

	public ElementType getType() {
		return type;
	}
	
	protected void setType(ElementType type) {
		this.type= type;
	}

	public Object getAttributeValue(String attributeName) throws PaloJException, PaloException {

		if(!validateCache(true)){
			reinitCache(true);
		}

		Object value = attributeValues.get(attributeName);
		if( value == null)
			throw new PaloJException("No attribute with this name exists for this element.");

		//if(this.type.equals(ElementType.ELEMENT_NUMERIC) && attributes.get(attributeName).getType().equals(ElementType.ELEMENT_NUMERIC))
		//	return new Double(value.toString());
		return value.toString();
	}

	public int getChildCount() throws PaloException, PaloJException {
		if (!validateCache(false)) {
			reinitCache(false);
		}
		return this.childrenIds.length;
	}

	public int getParentCount() throws PaloException, PaloJException {
		if(!validateCache(false)){
			reinitCache(false);
		}
		return this.parentsIds.length;
	}

	public Element[] getChildren() throws PaloException, PaloJException {
		if(!validateCache(false)){
			reinitCache(false);
		}

		return getChildrenObjects();
	}

	/**
	 * @return
	 */
	protected Element[] getChildrenObjects() {
		Dimension dim = database.getDimensionById(dimensionId);
		Element[] children = new Element[getChildCount()];
		this.childrenElements = new HashMap<Integer, Element>();

		for(int i=0;i<children.length;i++){
			children[i] = dim.getElementById(childrenIds[i]);
			children[i].weight = childrenweights[i];
			this.childrenElements.put(children[i].getId(), children[i]);
		}

		return children;
	}

	public Element[] getParents() throws PaloException, PaloJException {
		if(!validateCache(false)){
			reinitCache(false);
		}
		Dimension dim = database.getDimensionById(dimensionId);
		Element[] parents = new Element[getParentCount()];
		for(int i=0;i<parents.length;i++)
			parents[i] = dim.getElementById(parentsIds[i]);

		return parents;
	}

	public HashMap<String,HashMap<String,Object>>   getSubTreeAttributes() throws PaloException, PaloJException {

		if (!validateCache(true)) {
			reinitCache(true);
		}

		Dimension dim = database.getDimensionById(dimensionId);
		HashMap<String,HashMap<String,Object>>  attributesMap = new HashMap<String,HashMap<String,Object>>();

		Element e = dim.elementsIdMap.get(id);
		int[] childrenIds = e.childrenIds;
		ArrayList<Element> children = new ArrayList<Element>();

		for(int id:childrenIds){
			Element child = dim.elementsIdMap.get(id);
			children.add(child);
			attributesMap.putAll(getElementSubTreeAttributesMain(child));
		}

		attributesMap.put(e.getName(), e.attributeValues);

		return attributesMap;
	}

	private HashMap<String,HashMap<String,Object>>  getElementSubTreeAttributesMain(Element e) throws PaloException, PaloJException{

		Dimension dim = database.getDimensionById(dimensionId);
		HashMap<String,HashMap<String,Object>>  attributesMap = new HashMap<String,HashMap<String,Object>>();

		int[] childrenIds = e.childrenIds;
		ArrayList<Element> children = new ArrayList<Element>();
		for(int id:childrenIds){
			Element child = dim.elementsIdMap.get(id);
			children.add(child);
			attributesMap.putAll(getElementSubTreeAttributesMain(child));
		}

		attributesMap.put(e.getName(), e.attributeValues);

		return attributesMap;

	}

	public HashMap<String,IElement[]>  getSubTree() throws PaloException, PaloJException {

		if(!validateCache(false)){
			reinitCache(false);
		}

		HashMap<String, IElement[]> elementsMap = new HashMap<String, IElement[]>();
		Element e = database.getDimensionById(dimensionId).elementsIdMap.get(getId());
		int[] childrenIds = e.childrenIds;
		ArrayList<Element> children = new ArrayList<Element>();
		for(int id:childrenIds){
			Element child = database.getDimensionById(dimensionId).elementsIdMap.get(id);
			//child.getWeight(e);
			children.add(child);
			elementsMap.putAll(getElementSubTreeMain(child));
		}

		elementsMap.put(e.getName(), children.toArray(new Element[children.size()]));

		return elementsMap;
	}

	private HashMap<String,Element[]> getElementSubTreeMain(Element e) throws PaloException, PaloJException{

		HashMap<String, Element[]> elementsMap = new HashMap<String, Element[]>();

		int[] childrenIds = e.childrenIds;
		ArrayList<Element> children = new ArrayList<Element>();
		for (int id : childrenIds) {
			Element child = database.getDimensionById(dimensionId).elementsIdMap.get(id);
			// child.getWeight(e);
			children.add(child);
			elementsMap.putAll(getElementSubTreeMain(child));
		}

		elementsMap.put(e.getName(), children.toArray(new Element[children.size()]));

		return elementsMap;

	}

	public double getWeight(IElement parent) throws PaloException, PaloJException {
		if(!validateCache(false)){
			reinitCache(false);
		}

		
		if (this.parentsIds.length == 0 && parent == null)
			return 1;
		else if(this.parentsIds.length == 0 && parent != null){
			throw new PaloJException("Element " + parent.getName() + " is not a parent of element " + name);
		}
		else{
			Element[] parents = this.getParents();
			for(Element p:parents){
				if(p.getName().equals(parent.getName())){

					Element parentElement = (Element)parent;
					if(parentElement.childrenElements== null) parentElement.getChildren();
					Element child = parentElement.childrenElements.get(id);
					if (parentElement.childrenElements.get(id) != null) {
						this.weight = child.weight;
					}else{
						throw new PaloJException("Element " + parent.getName() + " is not a parent of element " + name);
					}
				}
			}
		}

		return weight;
	}

	public void rename(String name) throws PaloException, PaloJException {
		elementhandler.rename(database.getId(), dimensionId, id, Helpers.urlEncode(Helpers.escapeDoubleQuotes(name)));
		this.name = name;
	}

	/*********************************************************************************/

	protected int getId() {
		return id;
	}

	/*
	 * protected String getInfo() { return
	 * elementhandler.getInfo(database.getId(), dimensionId, id); }
	 */

	protected int[] getParentsIds() {
		return parentsIds;
	}

	protected int[] getChildrenIds() {
		return childrenIds;
	}
	
	protected HashMap<String, Object> getAttributeValues(){
		return attributeValues;
	}

	private void reinitCache(boolean withAttributes) throws PaloException, PaloJException {

		Dimension dim = database.getDimensionById(dimensionId);

		if (!dim.validateCache(withAttributes)) {
			dim.getElements(withAttributes);
			//dim.refreshDimensionInfo();
		}

		/* update children parent */
		Element copy = dim.getElementById(this.id);
		if (copy == null)
			throw new PaloJException("Element " + name + " with id " + id
					+ " is already deleted.");

		/* recalculate the cache */
		this.type = copy.type;
		this.parentsIds = copy.parentsIds;
		this.childrenIds = copy.childrenIds;
		this.childrenweights = copy.childrenweights;
		this.weight = copy.weight;
		this.attributeValues = copy.attributeValues;
		this.attributes = copy.attributes;
		this.childrenElements = null;
	}

	private boolean validateCache(boolean withAttributeValues) throws PaloException, PaloJException {
		
		Dimension dim = database.getDimensionById(dimensionId);
		
		if( dim.hasCache(withAttributeValues) && dim.isValidCache()){
			return true;
		}

		if (!withAttributeValues) {

			int currentToken = dim.getServerDimensionInfo().getToken();
			if (dimensionToken == currentToken)
				return true;
			else {
				dimensionToken = currentToken;
				return false;
			}
		}
		// validate attributes values cache
		else {
			Cube attCube = this.database.getCubeById(dim.getDimensionInfo().getAttributeCubeId());

			// TODO this is not working correctly since there is no Palo token
			// that tells if cells values were changed
			// in the best case, Palo_CC token should be used when the cube
			// configuration enables cache.
			long currentToken = attCube.getCurrentCBToken();
			long serverToken = attCube.getCBToken();

			if (currentToken == serverToken)
				return true;
			else {
				return false;
			}
		}

	}

}
