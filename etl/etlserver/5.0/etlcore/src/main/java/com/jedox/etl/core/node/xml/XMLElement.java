package com.jedox.etl.core.node.xml;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;

public class XMLElement implements IElement {
	
	public static final String origAttributeName = "_etlOrigName";
	
	private Element element;
	private ElementType type = ElementType.ELEMENT_NUMERIC;
	
	public XMLElement(Element element) {
		this.element = element;
	}

	public Element getElement() {
		return element;
	}
	
	public static List<Element> getDescendants(Element node, boolean includeElement) {
		List<Element> result = new ArrayList<Element>();
		NodeList nodeList = node.getChildNodes();
		for (int i=0; i<nodeList.getLength(); i++) {
			Node child = nodeList.item(i);
			if (child instanceof Element) {
				result.add((Element)child);
				result.addAll(getDescendants((Element)child,false));
			}
		}
		if (includeElement) result.add(0, node);
		return result;
	}
	
	public static List<Element> getChildren(Element node) {
		List<Element> result = new ArrayList<Element>();
		NodeList nodeList = node.getChildNodes();
		for (int i=0; i<nodeList.getLength(); i++) {
			Node child = nodeList.item(i);
			if (child instanceof Element) {
				result.add((Element)child);
			}
		}
		return result;
	}
	
	public static String getTagName(Element element) {
		String internalName = element.getAttribute(origAttributeName);
		return (internalName == null || internalName.isEmpty()) ? element.getTagName() : internalName;
	}
	
	@Override
	public Object getAttributeValue(String name) throws PaloJException,
			PaloException {
		return element.getAttribute(name);
	}

	@Override
	public int getChildCount() throws PaloException, PaloJException {
		return getChildren(element).size();
	}

	@Override
	public IElement[] getChildren() throws PaloException, PaloJException {
		List<IElement> result = new ArrayList<IElement>();
		for (Element o : getChildren(element)) {
			result.add(new XMLElement(o));
		}
		return result.toArray(new IElement[result.size()]);
	}

	@Override
	public String getName() {
		return getTagName(element);
	}

	@Override
	public int getParentCount() throws PaloException, PaloJException {
		return (element.getParentNode() != null) ? 1 : 0;
	}

	@Override
	public IElement[] getParents() throws PaloException, PaloJException {
		Node parent = element.getParentNode();
		return (parent != null && parent instanceof Element) ? new IElement[]{new XMLElement((Element)parent)} : new IElement[0];
	}
	
	private IElement[] toElements(List<Element> elements) {
		List<IElement> result = new ArrayList<IElement>();
		for (Element o : elements) {
			result.add(new XMLElement(o));
		}
		return result.toArray(new IElement[result.size()]);
	}

	@Override
	public HashMap<String, IElement[]> getSubTree() throws PaloException,
			PaloJException {
		HashMap<String,IElement[]> result = new HashMap<String,IElement[]>();
		Iterator<Element> i = getDescendants(element,true).iterator();
		while (i.hasNext()) {
			Element e = (Element) i.next();
			result.put(getTagName(e), toElements(getChildren(e)));
		}
		return result;
	}

	private HashMap<String, Object> getAttributeValueMap(Element element) {
		HashMap<String, Object> result = new HashMap<String, Object>();
		NamedNodeMap m = element.getAttributes();
		for (int i=0; i<m.getLength(); i++) {
			Node a = m.item(i);
			result.put(a.getNodeName(),a.getNodeValue());
		}
		return result;
	}
	
	@Override
	public HashMap<String, HashMap<String, Object>> getSubTreeAttributes()
			throws PaloException, PaloJException {
		HashMap<String,HashMap<String,Object>> result = new HashMap<String,HashMap<String, Object>>();
		Iterator<Element> i = getDescendants(element,true).iterator();
		while (i.hasNext()) {
			Element e = (Element) i.next();
			result.put(getTagName(e), getAttributeValueMap(e));
		}
		return result;
	}

	@Override
	public ElementType getType() {
		return type;
	}
	
	public void setType(ElementType type) {
		this.type = type;
	}

	@Override
	public double getWeight(IElement element) throws PaloException, PaloJException {
		return 1;
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		throw new PaloJException("Renaming nodes is not supported by DOM");
	}

}
