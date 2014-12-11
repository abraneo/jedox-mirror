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

package com.jedox.etl.core.config;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.net.URL;

import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import com.jedox.etl.core.component.ConfigurationException;


/**
 * Reader Class for XML
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class XMLReader {

	private URL url;

	public XMLReader() {
	}

	/**
	 * gets an XML-Document from a file
	 * @param filename
	 * @return the XML Document
	 * @throws ConfigurationException
	 */
	public Document readDocument(String filename) throws ConfigurationException {
		try {
			URL url = new URL(filename);
			return readDocument(url);
		}
		catch (Exception e) {};
		try {
			File f = new File(filename);
			return readDocument(f.toURI().toURL());
		} catch (Exception e)
		{
			throw new ConfigurationException(e);
		}
	}

	/**
	 * gets an XML-Document from a file
	 * @param reader Reader that contains that String
	 * @return the XML Document
	 * @throws ConfigurationException
	 */
	public Document readDocument(Reader reader) throws ConfigurationException {
		try {
			Document document = new SAXBuilder().build(reader);
			return document;
		}
		catch (Exception e) {
			throw new ConfigurationException(e.getMessage());
		}
	}

	/**
	 * gets the URL of the XML resource read by this reader
	 * @return the URL
	 */
	public URL getURL() {
		return url;
	}

	/**
	 * reads a XML Document from an URL
	 * @param url
	 * @return the XML Document
	 * @throws IOException
	 * @throws JDOMException
	 */
	private Document readDocument(URL url) throws IOException, JDOMException {
		this.url = url;
		InputStreamReader isr = new InputStreamReader(url.openStream(),"UTF8");
		Reader r = new BufferedReader(isr);
		Document document = new SAXBuilder().build(r);
		r.close();
		isr.close();
		return document;
		//return ConfigConverter.getInstance().convert(document);
	}
}
