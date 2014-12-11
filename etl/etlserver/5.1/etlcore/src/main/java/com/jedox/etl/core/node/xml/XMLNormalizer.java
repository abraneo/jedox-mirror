package com.jedox.etl.core.node.xml;

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.StringEscapeUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

import com.jedox.etl.core.component.RuntimeException;

public class XMLNormalizer extends XMLTreeManager {
	
	public static final String defaultElementTarget = "_origName";
	public static final String defaultContentTarget = "_origContent";
	
	private String elementTarget;
	private String contentTarget;
	
	public XMLNormalizer() throws RuntimeException {
		super("Normalizer",null);
	}
	
	public void setElementTarget(String elementTarget) {
		this.elementTarget = elementTarget;
	}
	public String getElementTarget() {
		return elementTarget;
	}
	
	public void setContentTarget(String contentTarget) {
		this.contentTarget = contentTarget;
	}
	public String getContentTarget() {
		return contentTarget;
	}
	
	private boolean isActive() {
		return elementTarget != null || contentTarget != null;
	}
	
	private int getNextLevelId(int level, Map<Integer,Integer> levelids) {
		Integer id = levelids.get(level);
		if (id == null) {
			id = 0;
		}
		id++;
		levelids.put(level, id);
		return id;
	}
	
	private String getNameSuffix(int level, int id) {
		return "L"+level+"N"+id;
	}
	
	private void createElement(Node parent, Element element, int level, Map<Integer,Integer> levelids) {
		String nameSuffix = getNameSuffix(level,getNextLevelId(level,levelids));
		Element result = getDocument().createElement(element.getTagName()+nameSuffix);
		parent.appendChild(result);
		NamedNodeMap attributes = element.getAttributes();
		for (int i=0; i<attributes.getLength();i++) {
			Node a = attributes.item(i);
			result.setAttribute(a.getNodeName(), a.getNodeValue());
		}
		if (elementTarget != null) {
			result.setAttribute(elementTarget, element.getTagName());
		}
		NodeList children = element.getChildNodes();
		for (int i=0; i<children.getLength(); i++) {
			Node c = children.item(i);
			if (contentTarget != null && c instanceof Text) {
				Text t = (Text)c;
				if (!t.isElementContentWhitespace()) {
					String data = t.getData();
					if (data != null) data = data.replaceAll("\\r\\n|\\r|\\n", " ").trim();
					if (data != null && !data.isEmpty()) {
						String escapedContent = StringEscapeUtils.escapeXml(t.getData());
						result.setAttribute(contentTarget, escapedContent);
					}
				}
			} 
			else if (c instanceof Element) {
				createElement(result,(Element)c,level+1,levelids);
			}
		}
	}
	
	
	public Document normalize(Document document) {
		if (isActive()) {
			Map<Integer, Integer> ids = new HashMap<Integer,Integer>();
			createElement(getDocument(),document.getDocumentElement(),0,ids);
			setDocument(getDocument());
		}
		else {
			setDocument(document);
		}
		return getDocument();
	}
	
	

}
