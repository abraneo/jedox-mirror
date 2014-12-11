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
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

import java.io.File;
import java.io.StringReader;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;
import org.apache.xml.utils.DefaultErrorHandler;
import org.w3c.dom.Document;


import com.jedox.etl.core.component.RuntimeException;


public class XSLTUtil {

	/*
	private static void checkXslt(String xslt) throws RuntimeException {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		factory.setNamespaceAware(true);
		DocumentBuilder builder;
		try {
			builder = factory.newDocumentBuilder();
			builder.parse(new InputSource(new StringReader(xslt)));
		} 
		catch (Exception e) {
			throw new RuntimeException("XSTL could not be parsed: " + e.getMessage());
		}
	}
*/	
	
	public static Document applyXslt(Document document, String xslt) throws RuntimeException {
		StreamSource xsltSource = new StreamSource(new StringReader(xslt));
		return applyXsltCommon(document, xsltSource);
	}
	
	public static Document applyXslt(Document document, File xsltFile) throws RuntimeException {
		if(!xsltFile.exists())
			throw new RuntimeException("File " + xsltFile.getPath() + " does not exist.");
		StreamSource xsltSource = new StreamSource(xsltFile);
		return applyXsltCommon(document, xsltSource);
	}

	/**
	 * @param document
	 * @param xsltSource
	 * @return
	 * @throws TransformerFactoryConfigurationError
	 * @throws RuntimeException
	 */
	private static Document applyXsltCommon(Document document,
			StreamSource xsltSource)
			throws TransformerFactoryConfigurationError, RuntimeException {
		TransformerFactory tFactory = TransformerFactory.newInstance();
		tFactory.setErrorListener(new DefaultErrorHandler(true));
		Transformer transformer;
		try {
			transformer = tFactory.newTransformer(xsltSource);
		} catch (TransformerConfigurationException e) {
			String msgPrefix = "javax.xml.transform.TransformerException: org.xml.sax.SAXParseException;";
			String msg=e.getMessageAndLocation();
			if (e.getMessage().startsWith(msgPrefix))
				msg=msg.substring(msgPrefix.length()+1);
			throw new RuntimeException("XSLT could not be parsed: "+msg);
		}				
		DOMSource source = new DOMSource(document);
		DOMResult result = new DOMResult();
		transformer.setOutputProperty(OutputKeys.INDENT,"yes");
		try {
			transformer.transform(source, result);
		}	
		catch (TransformerException e) {
			throw new RuntimeException(e);
		}
		return (Document) result.getNode();
	}	

}
