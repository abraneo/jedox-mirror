package com.jedox.etl.core.node.tree;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.HashMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.node.INamedValue;
import com.jedox.etl.core.node.NamedValue;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.DimensionInfo;

public class TreeManagerNG implements ITreeManager {
	
	public class TreeManagerExporter implements ITreeExporter {
		private int bulkSize=100000;
		private boolean withAttributes=false;
		private Iterator<Element> iterator;
	
		public TreeManagerExporter(){			
//			this.bulkSize = bulkSize;
//			this.withAttributes = withAttributes;
			this.iterator = lookup.values().iterator();
		}
		
		public Element[] getNextBulk() {
			if(this.iterator.hasNext())
				return getElementIgnoreCase(withAttributes, iterator, bulkSize);
			else
				return new Element[]{};
		}
		
		public boolean hasNext() {
			return iterator.hasNext();
		}
		
		public void reset(){
			this.iterator = lookup.values().iterator();
		}
		
		public void setWithAttributes(boolean withAttributes) {
			this.withAttributes = withAttributes;
		}
		
		public void setBulkSize(int bulkSize) {
			this.bulkSize=bulkSize;
		}

		public Attribute[] getAttributes() {
			return TreeManagerNG.this.getAttributes();
		}		
		
	}
	
	private String name;
	//private String rootName;

	private Map<String,Element> lookup = new LinkedHashMap<String,Element>();
	private Map<String,Attribute> attributes = new LinkedHashMap<String,Attribute>();
	private List<IConsolidation> consolidationBuffer = new ArrayList<IConsolidation>();
	private Map<String,List<INamedValue<Object>>> attributeBuffer = new LinkedHashMap<String,List<INamedValue<Object>>>();
	private static final Log log = LogFactory.getLog(TreeManagerNG.class);
	private boolean autoCommit = true;
	
	
	public TreeManagerNG(String name) {
		this.name = name;
	}
	
	public TreeManagerNG(IDimension dimension) {
		this(dimension,true);
	}

	public TreeManagerNG(IDimension dimension, boolean withAttributes) {
		this(dimension.getName());
		setAutoCommit(false);
		if (withAttributes) {
			IAttribute[] dimAttributes = dimension.getAttributes();
			for (IAttribute a : dimAttributes) {
				Attribute attribute = new Attribute(a);
				attributes.put(attribute.getName(), attribute);
			}
		}
		//add elements and attributes
		for (IElement dimElement : dimension.getElements(withAttributes)) {
			Element e = provideElement(dimElement);
			if (withAttributes) {
				for (IAttribute a : attributes.values()) {
					addAttributeValue(a.getName(),e.getName(),dimElement.getAttributeValue(a.getName()));
				}
				commitAttributeValuesInternal();
			}
			IElement[] dimChildren = dimElement.getChildren();
			for (int i=0; i<dimChildren.length; i++) {
				IElement child = dimChildren[i];
				double weight = child.getWeight(dimElement);
				addConsolidation(dimElement,child,weight);
			}
		}
		commitConsolidationsInternal();
		setAutoCommit(true);
	}

	@Override
	public String getName() {
		return name;
	}
	
	// Used as unique starting element for recursive tree calls as multiple root elements are allowed
	private Element getRoot() {
		Element element = new Element(name,ElementType.ELEMENT_CONSOLIDATED);
		element.setChildren(getRootElements(false));
		return element;
	}
	
	public Element provideElement(IElement element) {
		Element result = lookup.get(element.getName());
		if (result == null) {
			result = new Element(element.getName(),element.getType());
			lookup.put(result.getName(), result);
		}
		return result;
	}
	
	public Element provideElement(String name, ElementType type) throws PaloJException {
		if (name.isEmpty()) {
			throw new PaloJException("Empty element name is not allowed.");
		}
		Element result = lookup.get(name);
		if (result == null) {
			result = new Element(name,type);
			lookup.put(result.getName(), result);
		} else {
			if (!result.getType().equals(type) && !result.getType().equals(ElementType.ELEMENT_CONSOLIDATED) && !type.equals(ElementType.ELEMENT_CONSOLIDATED)) {
				log.warn("Element "+name+" is already present in with different type "+result.getType().toString()+". Ignoring new type "+type.toString()+".");
			}
		}
		return result;
	}
	
	public Element getElement(String name) {
		return lookup.get(name);
	}
	
	private Element verifyElement(IElement element) throws PaloJException {
		return findElement(element.getName());
	}
	
	private List<ITreeElement> getAncestors(IElement element) throws PaloJException {
		List<ITreeElement> result = new ArrayList<ITreeElement>();
		if (element.getParentCount() > 0) for (IElement p: element.getParents()) 
		{
			Element e = getElement(p.getName());
			if (e != null) {
				result.addAll(getAncestors(e));
				result.add(e);
			}
		}
		return result;
	}
	
	public List<ITreeElement> getAncestors(IElement element, boolean includeElement) throws PaloJException {
		if (element == null) element = getRoot();
		List<ITreeElement> result = new ArrayList<ITreeElement>();
		if (includeElement) result.add(verifyElement(element));
		result.addAll(getAncestors(element));
		return result;
	}
	
	private LinkedHashSet<ITreeElement> getDescendants(IElement element) throws PaloJException {
		LinkedHashSet<ITreeElement> result = new LinkedHashSet<ITreeElement>();
		if (element.getChildCount() > 0) for (IElement c: element.getChildren())
		{
			result.add(verifyElement(c));
			result.addAll(getDescendants(c));
		}
		return result;
	}
	
	public List<ITreeElement> getDescendants(IElement element, boolean includeElement) throws PaloJException {
		if (element == null) element = getRoot();
		List<ITreeElement> result = new ArrayList<ITreeElement>();
		if (includeElement) result.add(verifyElement(element));
		result.addAll(getDescendants(element));
		return result;
	}
	
	public Element getChild(IElement element, String childName) throws PaloJException {
		if (element == null) return getElement(childName);
		return verifyElement(element).getChild(childName);
	}
	
	public Element getParent(IElement element, String parentName) throws PaloJException {
		if (element == null) return getElement(parentName);
		return verifyElement(element).getParent(parentName);
	}
	
	private boolean isCyclic(IConsolidation consolidation) {
		if (consolidation.getChild().equals(consolidation.getParent())) {
			log.warn("Element "+consolidation.getChild().getName()+" cannot be consolidated under itself in source "+getName()+". The consolidation is ignored.");
			return true;
		}
		List<ITreeElement> ancestors = getAncestors(consolidation.getParent());
		for (IElement ancestor : ancestors) {
			if (ancestor.getName().equals(consolidation.getChild().getName())) {
				log.warn("Failed to consolidate node "+consolidation.getChild().getName()+" with parent "+consolidation.getParent().getName()+" due to a circular reference in source "+getName()+".");
				return true;
			}
		}
		return false;
	}
	
	private Element findElement(String name) throws PaloJException {
		Element elementColumn = getElement(name);
		if (elementColumn == null) throw new PaloJException("Element with name "+name+" not found in "+this.getClass().getName()+" "+getName());
		return elementColumn;
	}
	
	private Attribute findAttributeDefinition(String name) throws PaloJException {
		Attribute attributeColumn = getAttributeByName(name);
		if (attributeColumn == null) {
			throw new PaloJException("Attribute with name "+name+" not found in "+this.getClass().getName()+" "+getName());
		}
		return attributeColumn;
	}
	
	private Map<IElement,Double> compactWeights(Map<IElement,Double> weights) {
		if (weights == null || weights.isEmpty()) return null;
		Map<IElement,Double> compactWeights = new HashMap<IElement,Double>(weights.size(),1);
		compactWeights.putAll(weights);
		return compactWeights;
	}
	
	private HashMap<String,Object> compactAttributes(HashMap<String,Object> values) {
		if (values != null && !values.isEmpty()) {
			HashMap<String,Object> compactValues = new HashMap<String,Object>(values.size(),1);
			compactValues.putAll(values);
			return compactValues;
		}
		return null;
	}

	@Override
	public void addAttributeConsolidation(IAttribute arg0, IAttribute arg1)
			throws PaloJException, PaloException {
		throw new PaloException("Not supported by "+this.getClass().getCanonicalName()+" backend.");

	}
	
	private void addAttributeToBuffer(INamedValue<Object> attribute, String elementName) {
		List<INamedValue<Object>> attributes = attributeBuffer.get(elementName);
		if (attributes == null) {
			attributes = new ArrayList<INamedValue<Object>>();
			attributeBuffer.put(elementName, attributes);
		}
		attributes.add(attribute);
	}
	
	private void commitAttributeValuesInternal() throws PaloJException {
		for (String s : attributeBuffer.keySet()) {
			Element e = findElement(s);
			HashMap<String,Object> newValues = new HashMap<String,Object>();
			if (e.getAttributes() != null) newValues.putAll(e.getAttributes());
			for (INamedValue<Object> a : attributeBuffer.get(s)) {
				if (a.getValue() != null && !a.getValueAsString().isEmpty())
					newValues.put(a.getName(), a.getValue());
			}
			e.setAttributes(compactAttributes(newValues));
		}
		attributeBuffer.clear();
	}
	
	public void commitAttributeValues() throws PaloJException {
		if (isAutoCommit()) log.warn("Explicit commit is called on TreeManager "+getName()+" but autocommit is still active.");
		commitAttributeValuesInternal();
	}
	
	public void addAttributeValue(String attribute, String element, Object value) throws PaloJException {
		Attribute a = this.findAttributeDefinition(attribute);
		findElement(element);
		NamedValue<Object> newAttribute = new NamedValue<Object>(a.getName(),value);
		addAttributeToBuffer(newAttribute,element);
		if (isAutoCommit()) commitAttributeValuesInternal();
	}

	@Override
	public void addAttributeValues(IAttribute attribute, IElement [] elements, Object[] values) throws PaloJException, PaloException {
		if (elements.length != values.length) throw new PaloJException("Array elements and values have to be of same size.");
		for (int i = 0; i < elements.length; i++) {
			addAttributeToBuffer(new NamedValue<Object>(attribute.getName(),values[i]),elements[i].getName());
		}
		if (isAutoCommit()) commitAttributeValuesInternal();
	}

	@Override
	public void addAttributes(String[] names, ElementType[] types)
			throws PaloJException, PaloException {
		if (names.length != types.length) throw new PaloJException("Array names and types have to be of same size.");
		for (int i = 0; i < names.length; i++) {
			Attribute column = new Attribute(names[i]);
			column.setType(types[i]);
			attributes.put(column.getName(), column);
		}
	}
	
	public void addAttributes(IAttribute[] attributes, boolean override) {
		if (attributes != null) for (IAttribute a : attributes) {
			addAttribute(a.getName(), a.getType(), override);
		}
	}
	
	private Attribute addAttribute(String name, ElementType type, boolean override) throws PaloJException {
		if (name.isEmpty()) {
			throw new PaloJException("Attribute name may not be empty.");
		}
		Attribute a = new Attribute(name,type);
		if (!override) {
			Attribute existing = getAttributeByName(a.getName());
			if (existing != null && (!existing.getType().equals(a.getType()) || !existing.getMode().equals(a.getMode()))) {
				log.warn("Incompatible attribute "+a.getName()+" already exists in tree. Using "+existing.getType().toString()+","+existing.getMode().toString()); 
			}
		}
		this.attributes.put(a.getName(),a);
		return a;
	}
	
	public Attribute addAttribute(String name, ElementType type) throws PaloJException {
		return addAttribute(name,type,false);
	}

	@Override
	public Element addBaseElement(String name, ElementType type) throws PaloJException, PaloException {
		return provideElement(name,type); 
	}

	@Override
	public void addElements(String[] names, ElementType[] types)
			throws PaloException, PaloJException {
		if (names.length != types.length) throw new PaloJException("Array names and types have to be of same size.");
		for (int i = 0; i < names.length; i++) {
			provideElement(names[i],types[i]);
		}
	}
	
	public void addElements(IElement[] elements, boolean withConsolidations) {
		boolean autocommit = isAutoCommit();
		setAutoCommit(false);
		Set<IElement> givenSet = new HashSet<IElement>();
		if (withConsolidations) {
			for (IElement e : elements) {
				givenSet.add(e);
				provideElement(e);
			}
		}
		for (IElement dimElement : elements) {
			//add attributes
			Element e = provideElement(dimElement);
			for (IAttribute a : attributes.values()) {
				Object value = dimElement.getAttributeValue(a.getName());
				addAttributeValue(a.getName(), e.getName(), value);
			}
			commitAttributeValuesInternal();
			if (withConsolidations) {
				IElement[] dimChildren = dimElement.getChildren();
				for (int i=0; i<dimChildren.length; i++) {
					IElement child = dimChildren[i];
					if (givenSet.contains(child)) {
						double weight = child.getWeight(dimElement);
						addConsolidation(dimElement,child,weight);
					}
				}
			}
		}
		if (withConsolidations) commitConsolidationsInternal();
		setAutoCommit(autocommit);
	}
	
	public void retainElements(String[] elementNames) throws PaloJException {
		Map<String,Element> elements = new HashMap<String,Element>();
		List<IConsolidation> consolidations = new ArrayList<IConsolidation>();
		for (String name : elementNames) {
			Element e = findElement(name);
			elements.put(name, e);
		}
		for (Element e : elements.values()) {
			for (Element c : e.getChildren()) {
				if (elements.containsKey(c.getName())) {
					consolidations.add(new Consolidation(e,c,c.getWeight(e)));
				}
			}
		}
		lookup.clear();
		lookup.putAll(elements);
		updateConsolidations(consolidations.toArray(new IConsolidation[consolidations.size()]));
	}
	
	private Element mergeElement(Element element, HashMap<String,Object> newAttributeValues) {
		if (newAttributeValues == null) newAttributeValues = new HashMap<String,Object>();
		HashMap<String,Object> existingAttributes = element.getAttributes();
		if (existingAttributes != null) {
			for (String s : existingAttributes.keySet()) {
				Object o = existingAttributes.get(s);
				if (o != null && !o.toString().isEmpty()) newAttributeValues.put(s,o);
			}
		}
		element.setAttributes(compactAttributes(newAttributeValues));
		return element;
	}
	
	
	public void addSubtree(IElement subTreeRoot, IConsolidation[] consolidations) throws PaloJException {
		//add elements and attributes
		boolean autocommit = isAutoCommit();
		setAutoCommit(false);
		Element subTreeRootElement = provideElement(subTreeRoot);
		//subTreeRootElement.setParents(null);
		Map<String,HashMap<String,Object>> attributes = subTreeRoot.getSubTreeAttributes();
		Map<String,IElement[]> subTree = subTreeRoot.getSubTree();
		mergeElement(subTreeRootElement,attributes.get(subTreeRoot.getName()));
		Set<IElement> processed = new HashSet<IElement>();
		
		for (IElement[] elements : subTree.values()) {
			if (elements != null) for (IElement e : elements) {
				if (!processed.contains(e)) {
					processed.add(e);
					HashMap<String,Object> newAttributeValues = attributes.get(e.getName());
					mergeElement(provideElement(e),newAttributeValues);
				}
			}
		}
		
		for (String s : subTree.keySet()) {
			IElement[] children = subTree.get(s);
			if (children != null) for (IElement child : children) {
				IElement parent = null;
				for (IElement p : child.getParents()) {
					if (s.equals(p.getName())) {
						parent = p;
						break;
					}
				}
				if (parent != null && child != null)
					addConsolidation(verifyElement(parent),verifyElement(child),child.getWeight(parent));
			}
		}
		addConsolidations(consolidations);
		commitConsolidationsInternal();
		setAutoCommit(autocommit);
	}

	@Override
	public Attribute getAttributeByName(String name) throws PaloException, PaloJException {
		return attributes.get(name);
	}
	
	@Override
	public DimensionInfo getDimensionInfo() throws PaloException {
		throw new PaloException("Not supported by "+this.getClass().getCanonicalName()+" backend.");
	}

	@Override
	public Element getElementByName(String name,boolean withAttributes) throws PaloException, PaloJException {
		return lookup.get(name); 
	}

	@Override
	public Element[] getElements(boolean withAttributes) throws PaloException,
			PaloJException {
		return lookup.values().toArray(new Element[lookup.values().size()]);
	}
	
	public Element[] getElementsIgnoreCase(boolean withAttributes) {
		return getElementIgnoreCase(withAttributes, lookup.values().iterator(), -1);
	}

	public Collection<? extends IElement> getElementsSet() {
		return lookup.values();
//		Set<IElement> set = new HashSet<IElement>();
//		set.addAll(lookup.values());
//		return set;
		//not possible due to type mismatch: return lookup.values();
	}
	
	private Element[] getElementIgnoreCase(boolean withAttributes,Iterator<Element> lookupIterator, int bulkSize){
		
		LinkedHashMap<String,Element> map = new LinkedHashMap<String,Element>();
		int count = 0;
		while (lookupIterator.hasNext() && (count < bulkSize || bulkSize==-1)) {			
			Element e = lookupIterator.next();
			String key = e.getName().toLowerCase();
			if (!map.containsKey(key)) {
				map.put(key, e);
				count++;
			}
			else {
				log.warn("Element with name "+e.getName()+" is duplicat in case insensitive form.");
				if (withAttributes) {
					Element oldElement = map.get(key);
					Element newElement = oldElement.clone();
					newElement.setName(newElement.getName().toLowerCase());
					mergeElement(newElement,e.getAttributes());
					map.remove(key);
					map.put(newElement.getName(), newElement);
				}
			}
		}
		return map.values().toArray(new Element[map.values().size()]);
		
	}

	@Override
	public int getId() {
		return getName().hashCode();
	}

	@Override
	public Element[] getRootElements(boolean withAttributes) throws PaloException, PaloJException {
		List<Element> result = new ArrayList<Element>(lookup.size());
		for (Element e : lookup.values()) {
			//Element e = lookup.get(s);
			if (e.getParentCount() == 0) result.add(e);
		}
		return result.toArray(new Element[result.size()]);
	}

	@Override
	public DimensionType getType() {
		return DimensionType.DIMENSION_NORMAL;
	}

	@Override
	public IConsolidation newConsolidation(IElement parent,IElement child, double weight) {
		return new Consolidation(parent,child,weight);
	}

	@Override
	public void removeAttributeConsolidations(IAttribute arg0) throws PaloException, PaloJException {
		throw new PaloException("Not supported by "+this.getClass().getCanonicalName()+" backend.");

	}

	@Override
	public void removeAttributeValues(IAttribute attribute, IElement [] elements) throws PaloJException, PaloException {
		commitAttributeValuesInternal();
		for (IElement element : elements) {
			Element e = verifyElement(element);
			if (e.getAttributes() != null) e.getAttributes().remove(attribute.getName());
		}
	}
	
	public Object getAttributeValue(String attributeName, String elementName, boolean verify) throws PaloJException {
		Element e = findElement(elementName);
		Object value = e.getAttributeValue(attributeName);
		if (verify) {
			Attribute a = findAttributeDefinition(attributeName);
			try {
				value = TypeConversionUtil.convertAttribute(a, e);
			}
			catch (NumberFormatException nfe) {
				log.warn("Attribute "+a.getName()+" of Element "+elementName+" is of type numeric, but the attribute value is not numeric: "+value.toString()+ "; therefore this value will be ignored.");
				//set value to null to ignore it
				return null;
			}
		}
		return value;
	}

	@Override
	public void removeAttributes(IAttribute[] attributes) throws PaloException, PaloJException {
		//NOTE: manager removes only the attribute definitions, not the actual attributes already assigned to elements. However, you will not be able to get them any more, unless you re-add the definition.
		for (IAttribute attribute : attributes) {
			this.attributes.remove(attribute.getName());
		}

	}
	
	public void removeConsolidation(IElement element) throws PaloJException {
		commitConsolidationsInternal();
		Element e = verifyElement(element);
		Element[] children = e.getChildren();
//		Element[] parents = e.getParents();
		e.setChildren(new Element[0]);
		for (Element c : children) {
			c.removeParent(e);
		}
/*		
		for (Element p : parents) {
			p.removeChild(e);
		}
*/		
	}
	
	private void removeParentConsolidation(IElement element) throws PaloJException {
		commitConsolidationsInternal();
		Element e = verifyElement(element);
		Element[] parents = e.getParents();
		e.setParents(new Element[0]);
		for (Element c : parents) {
			c.removeChild(e);
		}	
	}

	

	@Override
	public void removeConsolidations(IElement[] elements) throws PaloException, PaloJException {
		for (IElement element : elements) {
			Element e = verifyElement(element);
			removeConsolidation(e);
		}
	}
	
	public void removeConsolidations(IConsolidation[] consolidations) throws PaloException, PaloJException {
		Set<IConsolidation> existing = getConsolidationSet();
		existing.addAll(consolidationBuffer);
		for (IConsolidation c : consolidations) {
			existing.remove(c);
		}
		clearConsolidations();
		addConsolidationsInternal(existing);
	}
	
	public void clearConsolidations() {
		for (Element e : lookup.values()) {
			e.setParents(null);
			e.setChildren(null);
		}
		consolidationBuffer.clear();
	}
	
	public Element renameElement(String oldName, String newName) throws PaloJException {
		Element e = findElement(oldName);
		e.rename(newName);
		lookup.remove(oldName);
		lookup.put(newName, e);
		return e;
	}

	@Override
	public void removeElements(IElement[] elements) throws PaloException,
			PaloJException {
		for (IElement element : elements) {
			removeConsolidation(element);
			removeParentConsolidation(element);			
			lookup.remove(element.getName());
		}
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		this.name = name;
	}
	
	private Set<IConsolidation> getConsolidationSet() {
		Set<IConsolidation> consolidations = new LinkedHashSet<IConsolidation>();
		for (Element e : lookup.values()) {
			for (IElement c : e.getChildren()) {
				if (getElement(c.getName()) != null) consolidations.add(new Consolidation(e,c,c.getWeight(e)));
			}
		}
		return consolidations;
	}
	
	
	private Set<IConsolidation> getConsolidationSetInternal(IElement node, HashSet<ITreeElement> subtree, boolean unique) {
		Set<IConsolidation> result = new LinkedHashSet<IConsolidation>();
		if (unique) {
			IElement[] parents = node.getParents();
			for (int i=node.getParentCount()-1; i>=0; i--) {
				IElement p = parents[i];
				if (subtree.isEmpty() || subtree.contains(p)) {
					IConsolidation cons = new UniqueConsolidation(p,node,node.getWeight(p));
					result.add(cons);
				}			
			}
			for (IElement c : node.getChildren()) {
				result.addAll(getConsolidationSetInternal(c,subtree,unique));
			}
		} else {
			for (IElement c : node.getChildren()) {
				result.add(new Consolidation(node,c,c.getWeight(node)));
				if (c.getChildCount() > 0) result.addAll(getConsolidationSetInternal(c,subtree, unique));
			}
		}
		return result;
	}
	
	public Set<IConsolidation> getConsolidationSet(IElement node, boolean unique) {
		if (node == null) {
			Set<IConsolidation> result = new LinkedHashSet<IConsolidation>();
			for (IElement r : getRootElements(false)) {
				result.addAll(getConsolidationSetInternal(r,new HashSet<ITreeElement>(),unique));
			}
			return result;
		} else {
			HashSet<ITreeElement> descendants = getDescendants(node);
			descendants.add(verifyElement(node));
			return getConsolidationSetInternal(node,descendants,unique);
		}
	}
	
	public IConsolidation[] getConsolidations() {
		Set<IConsolidation> consolidations = getConsolidationSet();
		return consolidations.toArray(new IConsolidation[consolidations.size()]);
	}
		
	public void addConsolidation(IElement parent, IElement child, double weight) {
		Consolidation c = new Consolidation(parent,child,weight);
		consolidationBuffer.add(c);
		if (isAutoCommit()) commitConsolidationsInternal();
	}
	
	public void addConsolidations(IConsolidation[] consolidations) {
		if (consolidations != null && consolidations.length > 0) {
			for (IConsolidation c : consolidations) consolidationBuffer.add(c);
		}
		if (isAutoCommit()) commitConsolidationsInternal();
	}
	
	private void addConsolidationsInternal(Collection<IConsolidation> consolidations) {
		if (consolidations.size() > 0) {
			Map<Element,Set<IConsolidation>> parentsLookup = new HashMap<Element,Set<IConsolidation>>();
			Map<Element,Set<IConsolidation>> childrenLookup = new HashMap<Element,Set<IConsolidation>>();
			//pre-process the new consolidations to be able to add all parents / children at once 
			for (IConsolidation consolidation : consolidations) {
				Element child = (consolidation.getChild() != null) ? verifyElement(consolidation.getChild()) : null;
				Element parent = (consolidation.getParent() != null) ? verifyElement(consolidation.getParent()) : null;
				if (parent != null && child != null) {
					Set<IConsolidation> parents = parentsLookup.get(child);
					if (parents == null) {
						parents = new LinkedHashSet<IConsolidation>();
						parentsLookup.put(child, parents);
					}
					parents.add(consolidation);
					Set<IConsolidation> children = childrenLookup.get(parent);
					if (children == null) {
						children = new LinkedHashSet<IConsolidation>();
						childrenLookup.put(parent,children);
					}
					children.add(consolidation);
				}
			}
			for (Element e : parentsLookup.keySet()) { //first add all parent information and check cycles (which is dependent only on parents)
				Set<IConsolidation> parents = parentsLookup.get(e);
				if (parents != null) {
					Set<Element> newParents = new LinkedHashSet<Element>(e.getParentCount()+parents.size());
					Map<IElement,Double> weights = e.getWeights();
					for (Element p : e.getParents()) {
						newParents.add(p);
					}
					for (IConsolidation consolidation : parents) {
						Element p = verifyElement(consolidation.getParent());
						if (isCyclic(consolidation) || !newParents.add(p)) { //remove consolidation, if it is cyclic or duplicate
							childrenLookup.get(p).remove(consolidation);
						} else {
							if (consolidation.getWeight() != 1.0) weights.put(p,consolidation.getWeight());
						}
					}
					e.setParents(newParents.toArray(new Element[newParents.size()]));
					e.setWeights(compactWeights(weights));
				}
			}
			for (Element e : childrenLookup.keySet()) { //then add children info
				Set<IConsolidation> children = childrenLookup.get(e);
				if (children != null) {
					List<Element> newChildren = new ArrayList<Element>(e.getChildCount()+children.size());
					for (Element c : e.getChildren()) {
						newChildren.add(c);
					}
					for (IConsolidation consolidation : children) {
						newChildren.add(verifyElement(consolidation.getChild()));
					}
					e.setChildren(newChildren.toArray(new Element[newChildren.size()]));
				}
			}
		}
	}
	
	
	private void commitConsolidationsInternal() throws PaloJException {
		addConsolidationsInternal(consolidationBuffer);
		consolidationBuffer.clear();
	}
	
	public void commitConsolidations() throws PaloJException {
		if (isAutoCommit()) log.warn("Explicit commit is called on TreeManager "+getName()+" but autocommit is still active.");
		commitConsolidationsInternal();
	}

	@Override
	public void updateConsolidations(IConsolidation[] consolidations) throws PaloException, PaloJException {
		clearConsolidations();
		addConsolidations(consolidations);
		commitConsolidationsInternal();
	}

	@Override
	public Attribute[] getAttributes() throws PaloException, PaloJException {
		return attributes.values().toArray(new Attribute[attributes.values().size()]);
	}
	
	@Override
	public HashMap<String, IElement[]> getChildrenMap() throws PaloException {
		HashMap<String,IElement[]> result = new HashMap<String,IElement[]>();
		for (IElement e : getElements(false)) {
			result.put(e.getName(), e.getChildren());
		}
		return result;
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getAttributesMap()throws PaloException {
		HashMap<String, HashMap<String, Object>> result = new HashMap<String, HashMap<String, Object>>();
		for (IElement e : getElements(false)) {
			HashMap<String, Object> attributes = new HashMap<String, Object>();
			for (IAttribute a : getAttributes()) {
				Object value = e.getAttributeValue(a.getName());
				if (value != null) attributes.put(a.getName(), value);
			}
			if (!attributes.isEmpty()) result.put(e.getName(), attributes);
		}
		return result;
	}

	@Override
	public HashMap<String, HashMap<String, Double>> getWeightsMap() throws PaloException {
		HashMap<String, HashMap<String, Double>> result = new HashMap<String, HashMap<String, Double>>();
		for (IElement e : getElements(false)) {
			HashMap<String, Double> weights = new HashMap<String, Double>();
			for (IElement p : e.getParents()) {
				weights.put(p.getName(), e.getWeight(p));
			}
			if (!weights.isEmpty()) result.put(e.getName(), weights);
		}
		return result;
	}
	
	public void clear() {
		for (Element e : lookup.values()) {
			e.setParents(null);
			e.setChildren(null);
			e.setAttributes(null);
			e.setWeights(null);
		}
		lookup.clear();
		attributes.clear();
	}

	// returns the maximal depth i.e. the length of the longest path for the tree
	public int getLevelsCount() {
		ITreeElement root = getRoot();
		return getLevelsCount(root);
	}

	public int getLevelsCount(ITreeElement element) {
		ITreeElement[] children =  element.getChildren();
		if (children.length == 0) {
			return 0;
		}
		int maxlevel = 0;
		for (ITreeElement c : children) {
			int level = getLevelsCount(c);
			if (level>maxlevel)
				maxlevel=level;				
		}
		return 1+maxlevel;		
	}

	@Override
	public void setCacheTrustExpiry(int arg0) {
		//do nothing, since we are not expireing any cache. we ARE the backend;
	}	
	
	public List<ITreeElement>  getElementsByAttribute(String attribute, String value) {
		List<ITreeElement> result = new ArrayList<ITreeElement>();		
		for (Element e : lookup.values()) {		
			Object attributeValue = getAttributeValue(attribute, e.getName(), false);
			if (attributeValue != null && attributeValue.equals(value)) {
				result.add(e);
			}
		}
		return result;
	}

	public void setAutoCommit(boolean autoCommit) {
		this.autoCommit = autoCommit;
	}

	public boolean isAutoCommit() {
		return autoCommit;
	}

	@Override
	public boolean hasConsolidatedElements() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void setWithElementPermission(boolean arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void updateElementsType(IElement[] arg0, ElementType arg1)
			throws PaloException, PaloJException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public IElement[] getBasesElements(boolean arg0) throws PaloException,
			PaloJException {
		List<Element> result = new ArrayList<Element>(lookup.size());
		for (Element e : lookup.values()) {
			//Element e = lookup.get(s);
			if (e.getChildCount() == 0) result.add(e);
		}
		return result.toArray(new Element[result.size()]);
	}

	@Override
	public void resetCache() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public int removeAllConsolidations() {
		// TODO Auto-generated method stub
		return 0;
	}

	public ITreeExporter getExporter() {
		return new TreeManagerExporter();
	}

	@Override
	public void appendElements(IElement[] arg0) throws PaloException,
			PaloJException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public IElement[] getElementsByName(String[] arg0, boolean arg1)
			throws PaloException, PaloJException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void moveElements(IElement[] arg0, Integer[] arg1)
			throws PaloException, PaloJException {
		// TODO Auto-generated method stub
		
	}

		@Override
		public IElement getSingleElement(String arg0, boolean arg1)
				throws PaloException, PaloJException {
			// TODO Auto-generated method stub
			return null;
		}
	
}
