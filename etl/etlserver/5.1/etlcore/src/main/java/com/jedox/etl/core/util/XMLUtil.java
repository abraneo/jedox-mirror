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
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right
*   (commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

import java.io.StringReader;
import java.io.StringWriter;
import java.io.IOException;
import java.util.List;
import java.util.Properties;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.JDOMException;
import org.jdom.output.XMLOutputter;

import com.jedox.etl.core.component.RuntimeException;

public class XMLUtil {

	public static String jdomToString(Element element) throws IOException {
		Element root = (Element) element.clone();
		StringWriter writer = new StringWriter();
		Document document = new Document();
		document.setRootElement(root);
		XMLOutputter outputter = new XMLOutputter(Format.getPrettyFormat());
		//XMLOutputter outputter = new XMLOutputter(Format.getCompactFormat());
		outputter.output(document, writer);
		return writer.toString();
	}
	
	public static String w3cDocumentToString(org.w3c.dom.Document doc) throws RuntimeException{

		try{
			DOMSource domSource = new DOMSource(doc);
			StringWriter writer = new StringWriter();
			StreamResult result = new StreamResult(writer);
			TransformerFactory tf = TransformerFactory.newInstance();
			Transformer transformer = tf.newTransformer();
			transformer.transform(domSource, result);
			return writer.toString();
		} catch (TransformerException e) {
			throw new RuntimeException(e.getMessage());
		}
	}
	
	/**
	 * compare the 2 elements if they result in the same string then, they are equal.
	 * @param element1
	 * @param element2
	 * @return
	 * @throws IOException 
	 */
	public static boolean compare(Element element1,Element element2) throws IOException{
		if(jdomToString(element1).equals(jdomToString(element2)))
			return true;
			
		return false;
		
	}

	public static Element stringTojdom(String xmlString) throws IOException, JDOMException {
		Document doc = new SAXBuilder().build(new StringReader(xmlString));
		return doc.getRootElement();
	}

	 public static String stringEncodeXml(String orig)
	 {
		 if (orig == null)
		 {
			 return "";
		 }

		 orig = orig.replaceAll("&amp;", "&").replaceAll("&quot;", "\"").replaceAll("&apos;", "\'").replaceAll("&lt;", "<").replaceAll("&gt;", ">").replaceAll("&amp;", "&");

		 // no prolog should be saved within the xml file, this will cause the SAX parsing to fail
		 int i = orig.indexOf("<?xml",1);
		 while(i != -1){
			 orig = orig.replace(orig.substring(i,orig.indexOf("?>",i)+2),"");
			 int old_index = i;
			 i = orig.indexOf("<?xml",old_index +1);
		 }


		 return orig;
	 }
	 public static String xmlEncodeString(String orig)
	 {
		 if (orig == null)
		 {
			 return "";
		 }

		 char[] chars = orig.toCharArray();

		 StringBuffer strBuf = new StringBuffer();
		 for (int i = 0; i < chars.length; i++)
		 {
			 switch (chars[i])
			 {
			 case '&' : strBuf.append("&amp;");
			 break;
			 case '\"' : strBuf.append("&quot;");
			 break;
			 case '\'' : strBuf.append("&apos;");
			 break;
			 case '<' : strBuf.append("&lt;");
			 break;
			 case '>' : strBuf.append("&gt;");
			 break;
			 case '\n' : // Line Feed is OK
			 case '\r' : // Carriage Return is OK
			 case '\t' : // Tab is OK
				 // These characters are specifically OK, as exceptions to
				 // the general rule below:
				 strBuf.append(chars[i]);
				 break;
			 default :
				 if (((chars[i] >= 0x20) && (chars[i] <= 0xD7FF)) ||((chars[i] >= 0xE000) && (chars[i] <= 0xFFFD))) {
					 strBuf.append(chars[i]);
				 	}
			 // For chars outside these ranges (such as controlchars),
			 // do nothing; it's not legal XML to print these chars,
			 // even escaped
			 }
		 }

		 return strBuf.toString();
	 }
	 
	 public static Properties parseProperties(List<?> parameters) {
		Properties properties = new Properties();
		for (int j=0; j < parameters.size(); j++) {
			Element parameter = (Element) parameters.get(j);
			if(parameter.getName().equals("variable") && parameter.getChild("default")!=null){
				properties.put(parameter.getAttributeValue("name"),parameter.getChild("default").getTextTrim());
			}else
			properties.put(parameter.getAttributeValue("name"), parameter.getTextTrim());
		}
		return properties;
	 }

}
