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

package com.jedox.palojlib.main;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
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

public class Dimension implements IDimension{

	/* final variables are not part of the cache*/
	private final DimensionHandler dimensionhandler;
	private final int id;
	private final Database database;
	private final DimensionType type;
	private final String contextId;

	/*variables needed for the cache*/
	private DimensionInfo info;
	private String name;
	protected boolean withAttributeCacheExists;
	protected boolean withoutAttributeCacheExists;
	protected LinkedHashMap<Integer,Element> attributesIdMap  = new LinkedHashMap<Integer,Element>();
	protected LinkedHashMap<Integer,Element> elementsIdMap = new LinkedHashMap<Integer,Element>();
	protected HashMap<String,Element> elementsNameMap = new HashMap<String,Element>();
	protected HashMap<String,Element> attributseNameMap = new HashMap<String,Element>();
	private Date cacheExpiryTime = new Date();


	protected Dimension(String contextId, int id, String name,Database database, DimensionType type, String attributeDimensionId,String attributeCubeId,String maximumLevel,String maximumIndent,String maximumDepth,String token) throws PaloException, PaloJException{
		dimensionhandler = new DimensionHandler(contextId);
		this.contextId = contextId;
		this.id = id;
		this.name = name;
		this.database = database;
		this.type = type;
		this.withAttributeCacheExists = false;
		this.withoutAttributeCacheExists = false;
		this.info = new DimensionInfo(attributeDimensionId,attributeCubeId,maximumLevel, maximumIndent, maximumDepth,token);
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
		return this.info;
	}

	public IElement[] getElements(boolean withAttributes) throws PaloException, PaloJException{

		if(!validateCache(withAttributes)){
			reinitCache(withAttributes);
		}
		return elementsIdMap.values().toArray(new Element[elementsIdMap.size()]);

	}

	public IElement[] getRootElements(boolean withAttributes) throws PaloException, PaloJException{

		if(!validateCache(withAttributes)){
			reinitCache(withAttributes);
		}
			ArrayList<Element> roots = new ArrayList<Element>();
			for(Element e: elementsIdMap.values()){
				if(e.getParentsIds().length == 0){
					roots.add(e);
				}
			}
			return roots.toArray(new Element[roots.size()]);
	}
	
	public HashMap<String, IElement[]> getChildrenMap() {
		
		if(!validateCache(false)){
			reinitCache(false);
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
		
		if(!validateCache(false)){
			reinitCache(false);
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
				parentsWeightMap.put(parent.getName(), child.weight) ;	
			}
			map.put(e.getName(), parentsWeightMap);
		}
		
		return map;
	}
	
	public HashMap<String, HashMap<String, Object>> getAttributesMap() {
		
		if(!validateCache(true)){
			reinitCache(true);
		}
		
		HashMap<String, HashMap<String, Object>> map = new HashMap<String, HashMap<String, Object>>();	
		Collection<Element> elements = elementsNameMap.values();
		
		for(Element e:elements){
			map.put(e.getName(), e.getAttributeValues());
		}
		
		return map;
	}

	public IElement getElementByName(String name,boolean withAttributes) throws PaloException, PaloJException{

		if(!validateCache(withAttributes)){
			reinitCache(withAttributes);
		}
		
		return elementsNameMap.get(name.toLowerCase());
	}


	@SuppressWarnings("unchecked")
	public IAttribute[] getAttributes() throws PaloException, PaloJException {

		failIfNoAttributesAllowed();
		
		if(!validateCache(true)){
			this.database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).reinitCache(true);
			this.attributesIdMap = (LinkedHashMap<Integer, Element>) this.database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).elementsIdMap.clone();
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
		cacheExpiryTime = new Date();

	}
	
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

	public void addAttributes(String[] names, ElementType[] types) throws PaloJException, PaloException {
		
		failIfNoAttributesAllowed();
		dimensionhandler.addElements(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),names,types);
		cacheExpiryTime = new Date();

	}

	public void removeConsolidations(IElement [] elements) throws PaloException, PaloJException {

		dimensionhandler.removeConsolidations(database.getId(),id,elements);
		cacheExpiryTime = new Date();
	}

	public void updateConsolidations(IConsolidation[] consolidations) throws PaloException, PaloJException {
		
		dimensionhandler.updateConsolidations(database.getId(),id,consolidations);
		cacheExpiryTime = new Date();

	}

	public void addAttributeConsolidation(IAttribute parent, IAttribute child) throws PaloJException, PaloException {

		failIfNoAttributesAllowed();
		
		Element parentElement = new Element(contextId,this.database,this.getDimensionInfo().getAttributeDimensionId(),((Attribute)parent).getId(),parent.getName(),parent.getType(),null,null,null,null,null,-1);
		Element childElement = new Element(contextId,this.database,this.getDimensionInfo().getAttributeDimensionId(),((Attribute)child).getId(),child.getName(),child.getType(),null,null,null,null,null,-1);
		IConsolidation cons = new Consolidation(parentElement, childElement, 1);
		dimensionhandler.updateConsolidations(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),new IConsolidation[]{cons});
		cacheExpiryTime = new Date();

	}


	public void removeAttributeConsolidations(IAttribute attribute) throws PaloException, PaloJException {

		failIfNoAttributesAllowed();
		
		Element element = new Element(contextId,this.database,this.getDimensionInfo().getAttributeDimensionId(),((Attribute)attribute).getId(),attribute.getName(),attribute.getType(),null,null,null,null,null,-1);
		dimensionhandler.removeConsolidations(database.getId(),this.getDimensionInfo().getAttributeDimensionId(),new Element[]{element});
		cacheExpiryTime = new Date();
	}

	public Consolidation newConsolidation(IElement parent,IElement child, double weight) {
		return new Consolidation((Element)parent, (Element)child, weight);

	}

	public void removeAttributes(IAttribute[] attributes) throws PaloException, PaloJException {
		
		failIfNoAttributesAllowed();
		
		if(attributes.length==0)
			return;

		if(!validateCache(false)){
			reinitCache(false);
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
		cacheExpiryTime = new Date();
	}

	public void removeElements(IElement[] elements) throws PaloException, PaloJException {
		
		if(elements.length==0)
			return;
		
		if(!validateCache(false)){
			reinitCache(false);
		}

		int[] ids= new int[elements.length];
		for(int i=0;i<elements.length;i++){
			Element e=this.getElementById(((Element)elements[i]).getId());
			if(e== null)
				throw new PaloJException("Element " + elements[i].getName() + " can not be deleted. It does not exist in dimension "  + getName() + ".");
			ids[i] = e.getId();
		}
		dimensionhandler.removeElements(database.getId(),id,ids);
		cacheExpiryTime = new Date();

	}

	public void addAttributeValues(IAttribute attribute, IElement[] elements,Object[] values) throws PaloJException, PaloException {
		
		failIfNoAttributesAllowed();
		
		if(elements.length==0)
			return;

		Attribute att = (Attribute)attribute;
		
		if(this.attributesIdMap.get(att.getId()) == null)
			throw new PaloJException("Attribute " +  attribute.getName() + " does not exist in dimension "+ getName()+ ".");

		dimensionhandler.addAttributeValues(database,this.getDimensionInfo().getAttributeCubeId(),this.getDimensionInfo().getAttributeDimensionId(),att.getId(), elements, values);
		cacheExpiryTime = new Date();

	}

	public void removeAttributeValues(IAttribute attribute, IElement[] elements) throws PaloJException, PaloException {
		
		failIfNoAttributesAllowed();
		
		if(elements.length==0)
			return;

		Attribute att = (Attribute)attribute;
		if(att == null)
			throw new PaloJException("Attribute " +  attribute.getName() + " does not exist in dimension "+ getName()+ ".");

		dimensionhandler.removeAttributeValues(database,this.getDimensionInfo().getAttributeCubeId(),this.getDimensionInfo().getAttributeDimensionId(),att.getId(), elements);
		cacheExpiryTime = new Date();
	}
	
	public IElement[] getBasesElements(boolean withAttributes) throws PaloException, PaloJException{

		if(!validateCache(withAttributes)){
			reinitCache(withAttributes);
		}
			ArrayList<Element> bases = new ArrayList<Element>();
			for(Element e: elementsIdMap.values()){
				if(e.getChildCount() == 0){
					bases.add(e);
				}
			}
			return bases.toArray(new Element[bases.size()]);
	}

  /***************************************************************************************/

	private void reinitCache(boolean withAttributes) throws PaloException, PaloJException{
		this.withAttributeCacheExists = false;
		this.withoutAttributeCacheExists = false;
		this.elementsIdMap.clear();
		this.attributesIdMap.clear();
		this.elementsNameMap.clear();
		this.attributseNameMap.clear();
		boolean checkAttributes = withAttributes && attributesAllowed();

		if(checkAttributes){
			IElement[] attributesArray = database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId()).getElements(false);
			for(IElement att:attributesArray){
				this.attributesIdMap.put(((Element)att).getId(), (Element)att);
				this.attributseNameMap.put(((Element)att).getName(), (Element)att);
			}
		}

		this.refreshDimensionInfo();
		dimensionhandler.setElementsWithAttributes(id,this,database);
		withoutAttributeCacheExists = true;
		
		if(checkAttributes){
			this.withAttributeCacheExists = true;
		}
		

	}
	
	protected boolean hasCache(boolean withAttributes){
		return (withAttributeCacheExists || (!withAttributes && withoutAttributeCacheExists));
	}
	
	protected boolean isValidCache(){
		return cacheExpiryTime.after(new Date());
	}

	//validate elements names and/or attributes names cache (No attributes values check)
	protected boolean validateCache(boolean withAttributes) throws PaloException, PaloJException{
		
		boolean cacheExists = hasCache(withAttributes);
		
		if(cacheExists && isValidCache()){
			return true;
		}
		
		boolean hasAttributeDim = false;
		int currentAttToken = -1;
		int serverAttToken = -1;
		
		if(withAttributes && attributesAllowed()){
			hasAttributeDim = true;
			Dimension attDimension = this.database.getDimensionById(this.getDimensionInfo().getAttributeDimensionId());
			currentAttToken = attDimension.getDimensionInfo().getToken();
			serverAttToken = attDimension.getServerDimensionInfo().getToken();
		}
		
		int currentToken = getDimensionInfo().getToken();		
		int serverToken = getServerDimensionInfo().getToken();
		
		if( cacheExists && (serverToken == currentToken) &&
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
		
		dimensionhandler.rename(database,id,Helpers.urlEncode(Helpers.escapeDoubleQuotes(name)));
		this.name = name;
		cacheExpiryTime = new Date();
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
		cacheExpiryTime = new Date();
		
	}

	/**
	 * @throws PaloJException ***********************************************************************************/

	protected Element getElementById(int id) throws PaloException, PaloJException{
			if(!this.withAttributeCacheExists && !this.withoutAttributeCacheExists)
				reinitCache(false);
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


	public void setCacheTrustExpiry(int seconds) {
		if(withoutAttributeCacheExists){
			if(withAttributeCacheExists){
				if(!validateCache(true)){
					withAttributeCacheExists=false;
					withoutAttributeCacheExists=false;
				}
			}else{
				if(!validateCache(false)){
					withoutAttributeCacheExists=false;
				}
			}
		}
		cacheExpiryTime.setTime((new Date()).getTime()+(seconds*1000));	
	}

}
