package com.jedox.etl.core.node.xml;

import java.util.HashMap;
import java.util.Map;


import org.apache.commons.lang.StringEscapeUtils;
import org.w3c.dom.CDATASection;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

import com.jedox.etl.core.node.xml.XMLElement;

import com.jedox.etl.core.component.RuntimeException;

public class XMLDeNormalizer extends XMLTreeManager {
	
	private String elementSource;
	private String contentSource;
	
	public XMLDeNormalizer() throws RuntimeException {
		super("Denormalizer",null);
	}
	
	public void setElementSource(String elementSource) {
		this.elementSource = elementSource;
	}
	public String getElementSource() {
		return elementSource;
	}
	
	public void setContentSource(String contentSource) {
		this.contentSource = contentSource;
	}

	public String getContentSource() {
		return contentSource;
	}
	
	private boolean isActive() {
		return elementSource != null || contentSource != null;
	}
	
	private void createElement(Node parent, Element element, int level, Map<Integer,Integer> levelids) {
		String elementName = null;
		if (elementSource != null) {
			elementName = element.getAttribute(elementSource);
		}
		if (elementName == null || elementName.isEmpty()) {
			elementName = element.getTagName();
		}
		Element result = getDocument().createElement(elementName);
		String content = null;
		if (contentSource != null) {
			content = element.getAttribute(contentSource);
		}
		if (content != null && !content.isEmpty()) {
			String unescapedContent = StringEscapeUtils.unescapeXml(content);
			String escapedContent = StringEscapeUtils.escapeXml(content);						
			if (!content.equals(unescapedContent)) { //use CDATA section if necessary.
				CDATASection cdata = getDocument().createCDATASection(unescapedContent);
				result.appendChild(cdata);
			} else if (!content.equals(escapedContent)) { //use CDATA section if document contains escaped content
				CDATASection cdata = getDocument().createCDATASection(content);
				result.appendChild(cdata);
			} else {				
				Text text = getDocument().createTextNode(content);
				result.appendChild(text);
			}
		}
		parent.appendChild(result);
		NamedNodeMap attributes = element.getAttributes();
		for (int i=0; i<attributes.getLength();i++) {
			Node a = attributes.item(i);
			String name=a.getNodeName();
			if (!name.equals(elementSource) && !name.equals(contentSource) && !name.equals(XMLElement.origAttributeName)) {
				result.setAttribute(a.getNodeName(), a.getNodeValue());
			}	
		}
		NodeList children = element.getChildNodes();
		for (int i=0; i<children.getLength(); i++) {
			Node c = children.item(i);
			if (c instanceof Element) {
				createElement(result,(Element)c,level+1,levelids);
			}
		}
	}
	
	
	public Document denormalize(Document document) {
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
