package com.jedox.etl.core.node.xml;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.DimensionInfo;

public class XMLTreeManager implements IDimension {
	
	private class Consolidation implements IConsolidation {
		private IElement parent;
		private IElement child;
		private double weight;
		
		public Consolidation(IElement parent, IElement child, double weight) {
			this.parent = parent;
			this.child = child;
			this.weight = weight;
		}

		@Override
		public IElement getParent() {
			return parent;
		}

		@Override
		public IElement getChild() {
			return child;
		}

		@Override
		public double getWeight() {
			return weight;
		}
		
	}
	
	private Document manager;
	private String name;
	private Map<String,XMLElement> elements = new HashMap<String,XMLElement>();
	private Map<String,IAttribute> attributes = new HashMap<String,IAttribute>();
	private int elementCount = 0;
	private boolean handleDocumentNode = false;
	private static final Log log = LogFactory.getLog(XMLTreeManager.class);
	
	
	public XMLTreeManager(Document document) {
		setDocument(document);
	}
	
	public XMLTreeManager(IDimension dimension) throws RuntimeException {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		try {
			manager = factory.newDocumentBuilder().newDocument();
		} catch (ParserConfigurationException e) {
			throw new RuntimeException(e);
		}
		//import the elements.
		for (IElement element : dimension.getElements(false)) {
			addXMLElement(element);
		}
		//prepare consolidations and attributes
		Element root = null;
		IElement[] rootElements = dimension.getRootElements(true);
		if (rootElements.length == 1) { //single root node. consider it as unique document node AND regular element;
			root = addXMLElement(rootElements[0]);
			setXMLAttributes(rootElements[0]);
			rootElements = rootElements[0].getChildren();
			handleDocumentNode = true;
		} else { //multiple root nodes. Use unique additional root node.
			root = createElement(dimension.getName());
			handleDocumentNode = false;
		}
		manager.appendChild(root);
		List<IConsolidation> consolidations = new ArrayList<IConsolidation>();
		for (IElement element: rootElements) {
			XMLElement xmlElement = this.elements.get(element.getName());
			manager.getDocumentElement().appendChild(xmlElement.getElement());
			Map<String,IElement[]> subtree = element.getSubTree();
			//import consolidations to xml
			for (String s : subtree.keySet()) {
				IElement[] children = subtree.get(s);
				XMLElement parentElement = this.elements.get(s);
				for (IElement child : children) {
					IConsolidation c = this.newConsolidation(parentElement, child, 1);
					consolidations.add(c);
				}
			}
			if (!handleDocumentNode) setXMLAttributes(element);
		}
		this.updateConsolidations(consolidations.toArray(new IConsolidation[consolidations.size()]));
		IAttribute[] attributeDefs = dimension.getAttributes();
		String[] aNames = new String[attributeDefs.length];
		ElementType[] aTypes = new ElementType[attributeDefs.length];
		for (int i=0; i<aTypes.length; i++) aTypes[i] = attributeDefs[i].getType();
		for (int i=0; i<aNames.length; i++) aNames[i] = attributeDefs[i].getName();
		addAttributes(aNames,aTypes);	
	}
	
	public XMLTreeManager(String name, String rootName) throws RuntimeException {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		try {
			manager = factory.newDocumentBuilder().newDocument();
		} catch (ParserConfigurationException e) {
			throw new RuntimeException(e);
		}
		if (rootName != null) {
			Element root = createElement(rootName);
			manager.appendChild(root);
			handleDocumentNode = false;
		}
		this.name = name;
	}
	
	private Element addXMLElement(IElement element) {
		Element e = createElement(element.getName());
		XMLElement xmlElement = new XMLElement(e);
		this.elements.put(element.getName(), xmlElement);
		return e;
	}
	
	private void setXMLAttributes(IElement element) {
		//import attributes to xml
		Map<String, HashMap<String, Object>> attributes = element.getSubTreeAttributes();
		for (String s : attributes.keySet()) {
			XMLElement aElement = this.elements.get(s);
			HashMap<String, Object> eAttributes = attributes.get(s);
			for (String aName : eAttributes.keySet()) {
				Object value = eAttributes.get(aName);
				if (value != null) {
					aElement.getElement().setAttribute(aName, value.toString());
					//System.out.println(aElement.getName()+" "+aName+":"+value);
				}
			}
		}
	}
	
	protected void setDocument(Document document) {
		this.manager = document;
		handleDocumentNode = true;
		elements = new HashMap<String,XMLElement>();
		attributes = new HashMap<String,IAttribute>();
		elementCount = 0;
		name = document.getDocumentElement().getTagName();
		Iterator<Element> descendants = XMLElement.getDescendants(document.getDocumentElement(),handleDocumentNode).iterator();
		while (descendants.hasNext()) {
			Element e = descendants.next();
			elements.put(XMLElement.getTagName(e), new XMLElement(e));
			List<String> attributeList = new ArrayList<String>();
			NamedNodeMap xmlAttributes = e.getAttributes();
			for (int i=0; i<xmlAttributes.getLength(); i++) {
				Node attribute = xmlAttributes.item(i);
				if (!attributes.containsKey(attribute.getNodeName())) {
					attributeList.add(attribute.getNodeName());
				}
			}
			ElementType[] types = new ElementType[attributeList.size()];
			for (int i=0; i<types.length; i++) types[i] = ElementType.ELEMENT_STRING;
			addAttributes(attributeList.toArray(new String[attributeList.size()]),types);
		}
	}
	
	// Element name is generated constant with element counter
	private Element createElement(String name) {	
		elementCount++;
		String newName = "etl_"+elementCount;
		log.debug("Changing illegal tag name etl_"+name+" to "+newName+".");
		Element element = manager.createElement(newName);
		element.setAttribute(XMLElement.origAttributeName, name);
		return element;
	}


	@Override
	public void addAttributeConsolidation(IAttribute arg0, IAttribute arg1)
			throws PaloJException, PaloException {
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");
	}

	@Override
	public void addAttributeValues(IAttribute attribute, IElement [] elements, Object[] values) throws PaloJException, PaloException {
		if (elements.length != values.length) throw new PaloJException("Array elements and values have to be of same size.");
		for (int i = 0; i < elements.length; i++) {
			XMLElement e = getElementByName(elements[i].getName(),true);
			String value = values[i] != null ? values[i].toString() : "";
			e.getElement().setAttribute(attribute.getName(),value);
		}
	}

	@Override
	public void addAttributes(String[] names, ElementType[] types)
			throws PaloJException, PaloException {
		if (names.length != types.length) throw new PaloJException("Array names and types have to be of same size.");
		for (int i = 0; i < names.length; i++) {
			Attribute column = new Attribute(names[i],types[i]);
			attributes.put(column.getName(), column);
		}

	}

	@Override
	public IElement addBaseElement(String name, ElementType type) throws PaloJException, PaloException {
		Element e = createElement(name);
		manager.getDocumentElement().appendChild(e);
		XMLElement element = new XMLElement(e);
		elements.put(name, element);
		element.setType(type);
		return element;
	}

	@Override
	public void addElements(String[] names, ElementType[] types)
			throws PaloException, PaloJException {
		if (names.length != types.length) throw new PaloJException("Array names and types have to be of same size.");
		for (int i = 0; i < names.length; i++) {
			addBaseElement(names[i],types[i]);
		}

	}


	@Override
	public IAttribute getAttributeByName(String name) throws PaloException,
			PaloJException {
		return attributes.get(name);
	}

	@Override
	public IAttribute[] getAttributes() throws PaloException, PaloJException {
		return attributes.values().toArray(new IAttribute[attributes.values().size()]);
	}

	@Override
	public DimensionInfo getDimensionInfo() throws PaloException {
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");
	}

	@Override
	public XMLElement getElementByName(String name,boolean withAttributes)
			throws PaloException, PaloJException {
		return elements.get(name);
	}

	@Override
	public IElement[] getElements(boolean withAttributes) throws PaloException,
			PaloJException {
		return elements.values().toArray(new IElement[elements.values().size()]);
	}

	@Override
	public int getId() {
		return -1;
	}

	@Override
	public String getName() {
		return name == null ? manager.getDocumentElement().getNodeName() : name;
	}

	@Override
	public IElement[] getRootElements(boolean withAttributes) throws PaloException,
			PaloJException {
		List<IElement> result = new ArrayList<IElement>();
		if (!handleDocumentNode) {
			for (Element e : XMLElement.getChildren(manager.getDocumentElement())) {
				XMLElement element = elements.get(XMLElement.getTagName(e));
				if (element != null)
					result.add(element);
			}
		}
		else {
			result.add(elements.get(XMLElement.getTagName(manager.getDocumentElement())));
		}
		return result.toArray(new IElement[result.size()]);
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
	public void removeAttributeConsolidations(IAttribute arg0)
			throws PaloException, PaloJException {
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");

	}
	
	@Override
	public void removeAttributeValues(IAttribute attribute, IElement [] elements)
			throws PaloJException, PaloException {
		// TODO Auto-generated method stub
		for (IElement element : elements) {
			XMLElement e = this.elements.get(element.getName());
			if (e != null)
				e.getElement().removeAttribute(attribute.getName());
		}

	}
	
	@Override
	public void removeAttributes(IAttribute[] attributes) throws PaloException,
			PaloJException {
		//NOTE: manager removes only the attribute definitions, not the actual attributes already assigned to elements. However, you will not be able to get them any more, unless you re-add the definition.
		for (IAttribute attribute : attributes) {
			this.attributes.remove(attribute.getName());
		}

	}

	@Override
	public void removeConsolidations(IElement[] elements) throws PaloException,
			PaloJException {
		for (IElement element : elements) {
			XMLElement e = this.elements.get(element.getName());
			if (e != null) {
				Element detachedElement = (Element)e.getElement();
				if (detachedElement.getParentNode() != null)
					detachedElement.getParentNode().removeChild(detachedElement);
				manager.getDocumentElement().appendChild(detachedElement);
			}
		}

	}

	@Override
	public void removeElements(IElement[] elements) throws PaloException,
			PaloJException {
		for (IElement e : elements) {
			XMLElement element = this.elements.remove(e.getName());
			if (element != null) {
				Node parent = (Element)element.getElement().getParentNode();
				if (parent != null) parent.removeChild(element.getElement());
			}
		}

	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		this.name = name;
	}

	@Override
	public void updateConsolidations(IConsolidation[] consolidations)
			throws PaloException, PaloJException {
		//only reattach child to new parent. do not detach all elements. 
		for (IConsolidation consolidation : consolidations) {
			XMLElement parent = null;
			if (consolidation.getParent() != null) parent = elements.get(consolidation.getParent().getName());
			XMLElement child = elements.get(consolidation.getChild().getName());
			Element detachedChild = (Element)child.getElement();
			if (detachedChild.getParentNode() != null)
				detachedChild.getParentNode().removeChild(detachedChild);
			if (parent != null) {
				parent.getElement().appendChild(detachedChild);
			}
			else {
				if (manager.getDocumentElement().getElementsByTagName(detachedChild.getNodeName()).getLength() == 0)
					manager.getDocumentElement().appendChild(detachedChild);
			}
		}

	}
	
	public Document getDocument() {
		return manager;
	}

	@Override
	public HashMap<String, IElement[]> getChildrenMap() throws PaloException {
		HashMap<String,IElement[]> result = new HashMap<String,IElement[]>();
		for (XMLElement e : elements.values()) {
			result.put(e.getName(), e.getChildren());
		}
		return result;
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getAttributesMap()throws PaloException {
		HashMap<String, HashMap<String, Object>> result = new HashMap<String, HashMap<String, Object>>();
		for (XMLElement e : elements.values()) {
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
		for (XMLElement e : elements.values()) {
			HashMap<String, Double> weights = new HashMap<String, Double>();
			for (IElement p : e.getParents()) {
				weights.put(p.getName(), e.getWeight(p));
			}
			if (!weights.isEmpty()) result.put(e.getName(), weights);
		}
		return result;
	}

	@Override
	public void setCacheTrustExpiry(int arg0) {
		//do nothing, since we are not expireing any cache. we ARE the backend;		
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
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");
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
	public IAttribute addAttribute(String arg0, ElementType arg1)
			throws PaloJException, PaloException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public IElement getSingleElement(String arg0, boolean arg1)
			throws PaloException, PaloJException {
		// TODO Auto-generated method stub
		return null;
	}

}
