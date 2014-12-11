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
 *   You may obtain a copy of the License at
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
	private int position;
	private int[] parentsIds;
	private int[] childrenIds;
	private double[] childrenweights;
	private boolean hiddenParentsFiltered = false;
	private boolean hiddenChildrenFiltered = false;
	private boolean hiddenChildrenWeightsFiltered = false;
	//protected double weight;
	protected HashMap<Integer, Double> parentWeights;
	protected HashMap<String, Object> attributeValues;
	private HashMap<String, Element> attributes;
	private int dimensionToken;
	protected HashMap<Integer, Element> childrenElements;
	private ElementPermission permission = null;

	protected Element(String contextId, Database database, int dimensionId, int id, String name, ElementType type,int position, int[] parentsIds, int[] childrenIds,
			double[] weights, HashMap<String, Object> attributesValues,HashMap<String, Element> attributes, int dimensionToken,String permission,boolean hiddenFiltered) throws PaloException, PaloJException {
		elementhandler = new ElementHandler(contextId);
		this.database = database;
		this.dimensionId = dimensionId;
		this.name = name;
		this.id = id;
		this.type = type;
		this.position = position;
		this.parentsIds = parentsIds;
		this.childrenIds = childrenIds;
		this.childrenweights = weights;
		this.attributeValues = attributesValues;
		this.dimensionToken = dimensionToken;
		this.attributes = attributes;
		childrenElements = null;
		//int initialCapacity = Math.max(16,((this.parentsIds!=null?((int)(this.parentsIds.length/0.75)+1):16)));
		this.parentWeights = new HashMap<Integer, Double>();
		if(permission!=null && !permission.isEmpty())
			this.permission = ElementPermission.valueOf(permission);
		
		hiddenParentsFiltered = hiddenFiltered;
		hiddenChildrenFiltered = hiddenFiltered;
		hiddenChildrenWeightsFiltered = hiddenFiltered;
		
	}

	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}
	
	public int getPosition() {
		return position;
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
		return getChildrenIds().length;
	}

	public int getParentCount() throws PaloException, PaloJException {
		if(!validateCache(false)){
			reinitCache(false);
		}
		return getParentsIds().length;
	}

	public Element[] getChildren() throws PaloException, PaloJException {
		if(!validateCache(false)){
			reinitCache(false);
		}

		return getChildrenObjects();
	}

	protected Element[] getChildrenObjects() {
		Dimension dim = database.getDimensionById(dimensionId);
		Element[] children = new Element[getChildCount()];
		int initialCapacity = Math.max(16,((int)(children.length/0.75)+1));
		this.childrenElements = new HashMap<Integer, Element>(initialCapacity);

		for(int i=0;i<children.length;i++){
			children[i] = dim.getElementById(getChildrenIds()[i]);
			children[i].parentWeights.put(id, getChildrenWeights()[i]);
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
			parents[i] = dim.getElementById(getParentsIds()[i]);

		return parents;
	}

	public HashMap<String,HashMap<String,Object>>   getSubTreeAttributes() throws PaloException, PaloJException {

		if (!validateCache(true)) {
			reinitCache(true);
		}

		Dimension dim = database.getDimensionById(dimensionId);
		HashMap<String,HashMap<String,Object>>  attributesMap = new HashMap<String,HashMap<String,Object>>();

		Element e = dim.elementsIdMap.get(id);
		int[] childrenIds = e.getChildrenIds();
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

		int[] childrenIds = e.getChildrenIds();
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
		int[] childrenIds = e.getChildrenIds();
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

		int[] childrenIds = e.getChildrenIds();
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

		int parentsNum = getParentsIds().length;
		if (parentsNum == 0 && parent == null)
			return 1;
		else if(parentsNum == 0 && parent != null){
			throw new PaloJException("Element " + parent.getName() + " is not a parent of element " + name);
		}
		else{
			Element[] parents = this.getParents();
			for(Element p:parents){
				if(p.getName().equals(parent.getName())){

					Element parentElement = (Element)parent;
					if(parentElement.childrenElements== null) parentElement.getChildren();
					Element child = parentElement.childrenElements.get(id);
					return child.parentWeights.get(parentElement.getId());
				}
			}
			throw new PaloJException("Element " + parent.getName() + " is not a parent of element " + name);
		}
	}

	public void rename(String name) throws PaloException, PaloJException {
		elementhandler.rename(database.getId(), dimensionId, id, Helpers.urlEncode(name));
		this.name = name;
	}
	
	public void move(int position) throws PaloException, PaloJException {
		if(position<0)
			throw new PaloJException("Element position should be positive or zero");
		elementhandler.move(database.getId(), dimensionId, id, position);
		this.position = position;
	}

	/*********************************************************************************/

	public int getId() {
		return id;
	}

	/*
	 * protected String getInfo() { return
	 * elementhandler.getInfo(database.getId(), dimensionId, id); }
	 */

	protected int[] getParentsIds() {
		if(!hiddenParentsFiltered){
			ArrayList<Integer> ids = new ArrayList<Integer>();
			for(int id:parentsIds){
				if(database.getDimensionById(dimensionId).getElementById(id)!=null)
					ids.add(id);
			}
			parentsIds = new int[ids.size()];
			for(int i=0;i<parentsIds.length;i++)
				parentsIds[i]=ids.get(i);
			
			hiddenParentsFiltered = true;
		}
		return parentsIds;
	}

	protected int[] getChildrenIds() {
		if(!hiddenChildrenFiltered){
			ArrayList<Integer> ids = new ArrayList<Integer>();
			for(int id:childrenIds){
				if(database.getDimensionById(dimensionId).getElementById(id)!=null)
					ids.add(id);
			}
			
			childrenIds = new int[ids.size()];
			for(int i=0;i<childrenIds.length;i++)
				childrenIds[i]=ids.get(i);
			
			hiddenChildrenFiltered= true;
		}
		return childrenIds;
	}
	
	protected double[] getChildrenWeights() {
		if(!hiddenChildrenWeightsFiltered){
			ArrayList<Double> weight = new ArrayList<Double>();
			for(int i=0;i<childrenIds.length;i++){
				if(database.getDimensionById(dimensionId).getElementById(childrenIds[i])!=null)
					weight.add(childrenweights[i]);
			}
			
			childrenweights = new double[weight.size()];
			for(int i=0;i<childrenweights.length;i++)
				childrenweights[i]=weight.get(i);
			
			hiddenChildrenWeightsFiltered = true;
			
		}
		
		return childrenweights;
	}
	
	protected HashMap<String, Object> getAttributeValues(){
		return attributeValues;
	}

	private void reinitCache(boolean withAttributes) throws PaloException, PaloJException {

		Dimension dim = database.getDimensionById(dimensionId);

		if (!dim.checkCacheStatus(withAttributes)) {
			dim.buildCache(withAttributes);
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
		this.parentWeights = copy.parentWeights;
		this.attributeValues = copy.attributeValues;
		this.attributes = copy.attributes;
		this.childrenElements = null;
	}

	private boolean validateCache(boolean withAttributeValues) throws PaloException, PaloJException {
		
		Dimension dim = database.getDimensionById(dimensionId);
		
		if( dim.cacheExists(withAttributeValues) && dim.inTrustTime()){
			return true;
		}

		int currentToken = dim.info.getToken();
		if (dimensionToken != currentToken)
			return false;
			
		/**
		 * IMPORTANT: this will track only structure changes not attribute values changes
		 */
		int currentServerToken = dim.getServerDimensionInfo().getToken();
		if (dimensionToken == currentServerToken)
			return true;
		else {
			dimensionToken = currentServerToken;
			return false;
		}

	}

	@Override
	public ElementPermission getPermission() {
		return permission;
	}

}
