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

package com.jedox.palojlib.main;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map.Entry;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.util.Helpers;

public class Dimension extends CachedComponent implements IDimension{
	
	public static final int defaultExpiryDuration = 60;

	/* final variables are not part of the cache*/
	private final DimensionHandler dimensionhandler;
	private final int id;
	private final Database database;
	private final DimensionType type;
	private final String contextId;

	/*variables needed for the cache*/
	protected DimensionInfo info;
	private String name;
	protected boolean withAttributeCacheExists;
	protected boolean withoutAttributeCacheExists;
	protected LinkedHashMap<Integer,Element> attributesIdMap  = new LinkedHashMap<Integer,Element>();
	protected LinkedHashMap<Integer,Element> elementsIdMap = new LinkedHashMap<Integer,Element>();
	protected HashMap<String,Element> elementsNameMap = new HashMap<String,Element>();
	protected HashMap<String,Element> attributesNameMap = new HashMap<String,Element>();
	protected boolean hasConsolidatedElements = false;


	protected Dimension(String contextId, int id, String name,Database database, DimensionType type, String attributeDimensionId,String attributeCubeId,String maximumLevel,String maximumIndent,String maximumDepth,String token, String numberOfElements) throws PaloException, PaloJException{
		dimensionhandler = new DimensionHandler(contextId);
		this.contextId = contextId;
		this.id = id;
		this.name = name;
		this.database = database;
		this.type = type;
		this.withAttributeCacheExists = false;
		this.withoutAttributeCacheExists = false;
		this.info = new DimensionInfo(attributeDimensionId,attributeCubeId,maximumLevel, maximumIndent, maximumDepth,token,numberOfElements);
		cacheTrustExpiry = defaultExpiryDuration;
		//this.info.token = -1;
	}


	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}

	public DimensionType getType() {
		return type;
	}

	public DimensionInfo getDimensionInfo(){
		return dimensionhandler.getInfo(this.database.getId(), this.id);
	}

	public IElement[] getElements(boolean withAttributes) throws PaloException, PaloJException{

		if(!checkCacheStatus(withAttributes)){
			buildCache(withAttributes);
		}
		return elementsIdMap.values().toArray(new Element[elementsIdMap.size()]);

	}

	public IElement[] getRootElements(boolean withAttributes) throws PaloException, PaloJException{

		if(!checkCacheStatus(withAttributes)){
			buildCache(withAttributes);
		}
			ArrayList<Element> roots = new ArrayList<Element>();
			for(Element e: elementsIdMap.values()){
				if(e.getParentsIds().length == 0){
					roots.add(e);
				}
			}
			return roots.toArray(new Element[roots.size()]);
	}
	
	public IElement[] getBasesElements(boolean withAttributes) throws PaloException, PaloJException{

		if(!checkCacheStatus(withAttributes)){
			buildCache(withAttributes);
		}
			ArrayList<Element> bases = new ArrayList<Element>();
			for(Element e: elementsIdMap.values()){
				if(e.getChildCount() == 0){
					bases.add(e);
				}
			}
			return bases.toArray(new Element[bases.size()]);
	}
	
	public HashMap<String, IElement[]> getChildrenMap() {
		
		if(!checkCacheStatus(false)){
			buildCache(false);
		}
		
		HashMap<String, IElement[]> map = new HashMap<String, IElement[]>();	
		Collection<Element> elements = elementsNameMap.values();
		
		for(Element e:elements){
			
			int[] ids = e.getChildrenIds();
			IElement[] children = new IElement[ids.length];
			for(int i=0;i<children.length;i++){
				children[i] = getElementById(ids[i]);
			}
			map.put(e.getName(), children);
		}
		
		return map;
	}
	
	public HashMap<String,HashMap<String,Double>> getWeightsMap() {
		
		if(!checkCacheStatus(false)){
			buildCache(false);
		}
		
		HashMap<String,HashMap<String,Double>> map = new HashMap<String,HashMap<String,Double>>();	
		Collection<Element> elements = elementsNameMap.values();
		
		for(Element e:elements){
			
			int[] ids = e.getParentsIds();
			HashMap<String, Double> parentsWeightMap = new HashMap<String, Double>();
			
			for(int i=0;i<ids.length;i++){
				Element parent = getElementById(ids[i]);
				if(parent.childrenElements== null) parent.getChildrenObjects();
				Element child = parent.childrenElements.get(e.getId());
				parentsWeightMap.put(parent.getName(), child.parentWeights.get(parent.getId())) ;	
			}
			map.put(e.getName(), parentsWeightMap);
		}
		
		return map;
	}
	
	public HashMap<String, HashMap<String, Object>> getAttributesMap() {
		
		if(!checkCacheStatus(true)){
			buildCache(true);
		}
		
		HashMap<String, HashMap<String, Object>> map = new HashMap<String, HashMap<String, Object>>();	
		Collection<Element> elements = elementsNameMap.values();
		
		for(Element e:elements){
			map.put(e.getName(), e.getAttributeValues());
		}
		
		return map;
	}

	public IElement getElementByName(String name,boolean withAttributes) throws PaloException, PaloJException{

		if(!checkCacheStatus(withAttributes)){
			buildCache(withAttributes);
		}
		
		return elementsNameMap.get(name.toLowerCase());
	}
	
	@Override
	public IElement[] getElementsByName(String[] names, boolean withAttributes)
			throws PaloException, PaloJException {
		if(!checkCacheStatus(withAttributes)){
			buildCache(withAttributes);
		}
		
		IElement[] elements = new IElement[names.length];
		for(int i=0;i<names.length;i++){
			IElement e = elementsNameMap.get(names[i].toLowerCase());
			if(e==null)
				throw new PaloJException("Element " + names[i] + " does not exist.");
			else
				elements[i] = e;
		}
		return elements;
	}
	
	public boolean hasConsolidatedElements() {
		
		if(!checkCacheStatus(false)){
			buildCache(false);
		}
		return this.hasConsolidatedElements;
	}


	@SuppressWarnings("unchecked")
	public IAttribute[] getAttributes() throws PaloException, PaloJException {

		failIfNoAttributesAllowed();
		
		if(!checkCacheStatus(true)){
			this.database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).buildCache(true);
			this.attributesIdMap = (LinkedHashMap<Integer, Element>) this.database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).elementsIdMap.clone();
			this.attributesNameMap = (HashMap<String, Element>) this.database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).elementsNameMap.clone();
		}

		ArrayList<IAttribute> atts = new ArrayList<IAttribute>();
		for(Entry<Integer, Element> att : this.attributesIdMap.entrySet()){

			Element e = ((Element)att.getValue());

			atts.add(new Attribute(this.contextId,e.getId(),this.database.getId(),this.getDimensionInfo().getAttributeDimensionId(), e.getName(), e.getType()));
			//names.add(().getName());
		}
		return atts.toArray(new IAttribute[atts.size()]);
	}

	public IAttribute getAttributeByName(String name) throws PaloException, PaloJException {

		IAttribute[] attributes = getAttributes();
		for(IAttribute att:attributes)
			if(att.getName().toLowerCase().equals(name.toLowerCase()))
				return att;

		return null;
	}


	public void addElements(String[] names, ElementType[] types) throws PaloException, PaloJException {
		
		dimensionhandler.addElements(database.getId(),id,names,types);
		endTrustTime();
	}
	
	public void appendElements(IElement[] elements) throws PaloException, PaloJException {
		
		dimensionhandler.addElements(database.getId(),id,elements);
		endTrustTime();
	}
	
	@Override
	public void moveElements(IElement[] elements, Integer[] positions)
			throws PaloException, PaloJException {
		dimensionhandler.moveElements(database.getId(),id,elements, positions);
		endTrustTime();
		
	}
	
	/*public void addBaseElements(IElement[] elements) throws PaloException, PaloJException {
		
		dimensionhandler.addBaseElements(database.getId(),id,elements);
		endTrustTime();
	}*/
	
	public IElement addBaseElement(String name, ElementType type) throws PaloJException, PaloException {
		
		IElement e = dimensionhandler.addBaseElement(database,id,name,type);
		Element eObj = (Element)e;
		
		// try to update the cache if it exists
		if(withoutAttributeCacheExists){
			elementsIdMap.put(eObj.getId(), eObj);
			elementsNameMap.put(eObj.getName().toLowerCase(), eObj);
		}
		//exceptional case here, this method does not invalidate cache (bug:12181)
		//cacheExpiryTime = new Date();
		return e;
	
	}
	
	
	public IAttribute addAttribute(String name, ElementType type) throws PaloJException, PaloException {
		
		IElement e = dimensionhandler.addBaseElement(database,info.getAttributeDimensionId(),name,type);
		Element eObj = (Element)e;
		
		// try to update the cache if it exists
		if(withoutAttributeCacheExists){
			attributesIdMap.put(eObj.getId(), eObj);
			attributesNameMap.put(eObj.getName().toLowerCase(), eObj);
			this.database.getDimensionById(info.getAttributeDimensionId()).elementsIdMap.put(eObj.getId(), eObj);
			this.database.getDimensionById(info.getAttributeDimensionId()).elementsNameMap.put(eObj.getName().toLowerCase(), eObj);
		}
		
		
		Attribute att = new Attribute(this.contextId,eObj.getId(),this.database.getId(),info.getAttributeDimensionId(), eObj.getName(), eObj.getType());
		
		//exceptional case here, this method does not invalidate cache (bug:12181)
		//cacheExpiryTime = new Date();
		
		/**
		 * This is an example of wrong use of setCacheTrustExpiry 
		 * that may cause errors in combination with this method
		 */
		/*
		IElement[] elements = dim.getElements(true);
		dim.addAttribute("test1", ElementType.ELEMENT_STRING);
		dim.setCacheTrustExpiry(0);
		IElement[] elements22 = dim.getElements(false);
		dim.setCacheTrustExpiry(3600);
		IElement[] elements2 = dim.getElements(true);
		elements2[0].getAttributeValue("test1");*/
		return att;
	
	}
	

	public void addAttributes(String[] names, ElementType[] types) throws PaloJException, PaloException {
		
		failIfNoAttributesAllowed();
		dimensionhandler.addElements(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),names,types);
		endTrustTime();
		if(withAttributeCacheExists)this.info.token=-1;// writing attribute values or changing attribute dimension structure does not change the token so we have to change it ourselves

	}

	public void removeConsolidations(IElement [] elements) throws PaloException, PaloJException {

		dimensionhandler.removeConsolidations(database.getId(),id,elements);
		endTrustTime();
	}
	
	public int removeAllConsolidations() throws PaloException, PaloJException {

		ArrayList<Element> consolidatedElements = new ArrayList<Element>();
		for(Element e: this.elementsIdMap.values()){
			if(e.getChildCount()!=0){
				consolidatedElements.add(e);
			}
		}
		dimensionhandler.removeConsolidations(database.getId(),id,consolidatedElements.toArray(new Element[0]));
		endTrustTime();
		return consolidatedElements.size();
	}

	public void updateConsolidations(IConsolidation[] consolidations) throws PaloException, PaloJException {
		if(consolidations.length==0)
			throw new PaloJException("Consolidation list can be empty.");
		dimensionhandler.updateConsolidations(database.getId(),id,consolidations);
		endTrustTime();

	}

	public void addAttributeConsolidation(IAttribute parent, IAttribute child) throws PaloJException, PaloException {

		failIfNoAttributesAllowed();
		
		Element parentElement = new Element(contextId,this.database,this.getDimensionInfo().getAttributeDimensionId(),((Attribute)parent).getId(),parent.getName(),parent.getType(),0,null,null,null,null,null,-1,null,true);
		Element childElement = new Element(contextId,this.database,this.getDimensionInfo().getAttributeDimensionId(),((Attribute)child).getId(),child.getName(),child.getType(),0,null,null,null,null,null,-1,null,true);
		IConsolidation cons = new Consolidation(parentElement, childElement, 1);
		dimensionhandler.updateConsolidations(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),new IConsolidation[]{cons});
		endTrustTime();
		if(withAttributeCacheExists)this.info.token=-1;// writing attribute values or changing attribute dimension structure does not change the token so we have to change it ourselves

	}


	public void removeAttributeConsolidations(IAttribute attribute) throws PaloException, PaloJException {

		failIfNoAttributesAllowed();
		
		Element element = new Element(contextId,this.database,this.getDimensionInfo().getAttributeDimensionId(),((Attribute)attribute).getId(),attribute.getName(),attribute.getType(),0,null,null,null,null,null,-1,null,true);
		dimensionhandler.removeConsolidations(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),new Element[]{element});
		endTrustTime();
		if(withAttributeCacheExists)this.info.token=-1;// writing attribute values or changing attribute dimension structure does not change the token so we have to change it ourselves
	}

	public Consolidation newConsolidation(IElement parent,IElement child, double weight) {
		return new Consolidation((Element)parent, (Element)child, weight);

	}

	public void removeAttributes(IAttribute[] attributes) throws PaloException, PaloJException {
		
		failIfNoAttributesAllowed();
		
		if(attributes.length==0)
			return;

		if(!checkCacheStatus(false)){
			buildCache(false);
		}

		int[] ids= new int[attributes.length];
		Dimension attributeDim = database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId());
		for(int i=0;i<attributes.length;i++){
			Element e=attributeDim.getElementById(((Attribute)attributes[i]).getId());
			if(e== null)
				throw new PaloJException("Attribute " + attributes[i].getName() + " can not be deleted. It does not exist in dimension "  + getName() + ".");
			ids[i] = e.getId();

		}
		dimensionhandler.removeAttributes(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),ids);
		endTrustTime();
		if(withAttributeCacheExists)this.info.token=-1;// writing attribute values or changing attribute dimension structure does not change the token so we have to change it ourselves
	}

	public void removeElements(IElement[] elements) throws PaloException, PaloJException {
		
		if(elements.length==0)
			return;
		
		if(!checkCacheStatus(false)){
			buildCache(false);
		}

		int[] ids= new int[elements.length];
		for(int i=0;i<elements.length;i++){
			Element e=this.getElementById(((Element)elements[i]).getId());
			if(e== null)
				throw new PaloJException("Element " + elements[i].getName() + " can not be deleted. It does not exist in dimension "  + getName() + ".");
			ids[i] = e.getId();
		}
		dimensionhandler.removeElements(database.getId(),id,ids);
		endTrustTime();

	}

	public void addAttributeValues(IAttribute attribute, IElement[] elements,Object[] values) throws PaloJException, PaloException {
		
		failIfNoAttributesAllowed();
		
		if(elements.length==0)
			return;

		Attribute att = (Attribute)attribute;
		
		if(this.attributesNameMap.get(att.getName().toLowerCase()) == null)
			getAttributes();
			if(this.attributesNameMap.get(att.getName().toLowerCase()) == null)
				throw new PaloJException("Attribute " +  attribute.getName() + " does not exist in dimension "+ getName()+ ".");

		dimensionhandler.addAttributeValues(database,this.info.getAttributeCubeId(),this.info.getAttributeDimensionId(),att.getId(), elements, values);
		//endTrustTime();
		//if(withAttributeCacheExists)this.info.token=-1;// writing attribute values does not change the token so we have to change it ourselves

	}

	public void removeAttributeValues(IAttribute attribute, IElement[] elements) throws PaloJException, PaloException {
		
		failIfNoAttributesAllowed();
		
		if(elements.length==0)
			return;

		Attribute att = (Attribute)attribute;
		if(att == null)
			throw new PaloJException("Attribute " +  attribute.getName() + " does not exist in dimension "+ getName()+ ".");

		dimensionhandler.removeAttributeValues(database,this.info.getAttributeCubeId(),this.info.getAttributeDimensionId(),att.getId(), elements);
		//endTrustTime();
		//if(withAttributeCacheExists)this.info.token=-1;// writing attribute values or changing attribute dimension structure does not change the token so we have to change it ourselves
	}

  /***************************************************************************************/

	protected synchronized void buildCache(boolean withAttributes) throws PaloException, PaloJException{
		this.withAttributeCacheExists = false;
		this.withoutAttributeCacheExists = false;
		this.elementsIdMap.clear();
		this.attributesIdMap.clear();
		this.elementsNameMap.clear();
		this.attributesNameMap.clear();
		boolean checkAttributes = withAttributes && attributesAllowed();

		if(checkAttributes){
			IElement[] attributesArray = database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).getElements(false);
			for(IElement att:attributesArray){
				this.attributesIdMap.put(((Element)att).getId(), (Element)att);
				this.attributesNameMap.put(((Element)att).getName(), (Element)att);
			}
		}

		this.refreshDimensionInfo();
		int initialCapacityForElementMaps = Math.max(16,((this.info!=null?((int)(this.info.getNumberOfElements()/0.75)+1):16)));
		elementsIdMap = new LinkedHashMap<Integer,Element>(initialCapacityForElementMaps);
		elementsNameMap = new HashMap<String, Element>(initialCapacityForElementMaps);
		dimensionhandler.setElementsWithAttributes(id,this,database);
		withoutAttributeCacheExists = true;
		
		if(checkAttributes){
			this.withAttributeCacheExists = true;
		}
		setCacheTrustExpiry(cacheTrustExpiry);
	}
	
	protected boolean cacheExists(boolean withAttributes){
		return (withAttributeCacheExists || (!withAttributes && withoutAttributeCacheExists));
	}

	//validate elements names and/or attributes names cache (No attributes values check)
	protected synchronized boolean checkCacheStatus(boolean withAttributes) throws PaloException, PaloJException{
		
		boolean cacheExists = cacheExists(withAttributes);
		
		if(cacheExists && inTrustTime()){
			return true;
		}
		if (!cacheExists) {
			return false;
		}
					
		boolean hasAttributeDim = false;
		int currentAttToken = -1;
		int serverAttToken = -1;
		
		if(withAttributes && attributesAllowed()){
			hasAttributeDim = true;
			Dimension attDimension = this.database.getDimensionById(this.info.getAttributeDimensionId());
			currentAttToken = attDimension.info.getToken();
			serverAttToken = attDimension.getServerDimensionInfo().getToken();
		}
		
		int currentToken = this.info.getToken();		
		int serverToken = getServerDimensionInfo().getToken();
		
		if( (serverToken == currentToken) &&
				(!withAttributes || (hasAttributeDim && (currentAttToken == serverAttToken))))
			return true;
		else{
			return false;
		}
	}

	public int getId() {
		return id;
	}
	
	public void rename(String name) throws PaloException, PaloJException{
		
		dimensionhandler.rename(database,id,Helpers.urlEncode(name));
		this.name = name;
		endTrustTime();
	}
	
	@Override
	public void updateElementsType(IElement[] elements, ElementType type)
			throws PaloException, PaloJException {
		
		if(type == null){
			throw new PaloJException("Given type can not be null.");
		}
		
		if(type.equals(ElementType.ELEMENT_CONSOLIDATED)){
			throw new PaloJException("Only element type string or numeric can be used in updateElementsType");
		}
		
		for(IElement e:elements){
			if(e.getType().equals(ElementType.ELEMENT_CONSOLIDATED))
				throw new PaloJException("updateElementsType can be applied to consolidated elements");
			
			if(e.getType().equals(type))
				throw new PaloJException("updateElementsType with type " + type + " can't be applied on element " + e.getName()+". The element has already this type.");
		}
		
		for(IElement e:elements){
			((Element)e).setType(type);
		}
		
		dimensionhandler.removeConsolidations(database.getId(), id, elements);
		endTrustTime();
		
	}
	
	@Override
	public IElement getSingleElement(String elementName, boolean withAttributes) throws PaloException,
			PaloJException {
		return dimensionhandler.getSingleElement(database,this.id,elementName, withAttributes);
	}

	/**
	 * @throws PaloJException ***********************************************************************************/

	protected Element getElementById(int id) throws PaloException, PaloJException{
			if(this.type.equals(DimensionType.DIMENSION_SYSTEM_ID)){
				return new Element(contextId, database, this.id, id, String.valueOf(id), ElementType.ELEMENT_NUMERIC,0,null,null,null,null,null,-1,null,true);
			}
		
			if(!this.withAttributeCacheExists && !this.withoutAttributeCacheExists)
				buildCache(false);
			return elementsIdMap.get(id);
	}
	
	 protected DimensionInfo getServerDimensionInfo() throws PaloException, PaloJException{
		  return dimensionhandler.getInfo(database.getId(), id);
	}
	 
	protected DimensionInfo refreshDimensionInfo() throws PaloException, PaloJException{
		this.info = dimensionhandler.getInfo(database.getId(), id);
		return this.info;
	}

	protected Attribute getAttributeById(int id) throws PaloException, PaloJException {

		IAttribute[] attributes = getAttributes();
		for(IAttribute att:attributes)
			if(((Attribute)att).getId() == id)
				return ((Attribute)att);

		return null;
	}
	
	private boolean attributesAllowed(){
		return (type.equals(DimensionType.DIMENSION_NORMAL) || type.equals(DimensionType.DIMENSION_USERINFO));
	}
	
	private void failIfNoAttributesAllowed() throws PaloJException{
		if(!attributesAllowed()){
			throw new PaloJException("Dimension of type system or attribute can not have attributes. Only dimensions of type normal or userinfo can have attributes.");
		}
	}


	public void setWithElementPermission(boolean withPermission) {
		if(dimensionhandler.withPermission != withPermission){
			dimensionhandler.setWithElementPermission(withPermission);
			endTrustTime();
		}		
	}

	@Override
	public void resetCache() {
		withoutAttributeCacheExists = false;
		withAttributeCacheExists = false;
		endTrustTime();
	}
	
}
