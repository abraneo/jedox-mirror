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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.connection;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ConnectException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;
import com.jedox.etl.core.connection.IXmlConnection;


public class XmlFileConnection extends Connection implements IXmlConnection {

	private static final Log log = LogFactory.getLog(XmlFileConnection.class);
	private String filename;
	private SSLModes sslMode;
	
	protected void setFilename(String filename) {
		this.filename = filename;
	}

	protected String getFilename() {
		return filename;
	}
	
	public String getDatabase() {
		String database = getDatabaseString();
		if (getHost() == null && FileUtil.isRelativ(database)) {
			database = Settings.getInstance().getDataDir() + File.separator + database;
			database = database.replace("/", File.separator);
			database = database.replace("\\", File.separator);
		}
		return database;
	}
	
	protected String getDatabaseString() {
		return super.getDatabase();
	}

	/**
	 * initialize the connection by setting the file name
	 */
	public void init() throws InitializationException {
		try {
			super.init();
			setFilename(getDatabase()); //Relative();
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.trust.toString()));			
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}


	/**
	 * open a connection to the XML file and parse it using DOM
	 * @return parsed XML document
	 */
	@Override
	public Document open() throws RuntimeException{
		log.debug("Start reading XML-File " + getFilename() );
		checkSSL();
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		factory.setNamespaceAware(true);
		DocumentBuilder builder;
		Document doc = null;
		FileUtil.test(getFilename(),false);
		try {
			builder = factory.newDocumentBuilder();
			InputSource source = new InputSource(new InputStreamReader(FileUtil.getInputStream(getFilename()),getEncoding()));
			doc=builder.parse(source);
		} 
		catch (Exception e) {
			throw new RuntimeException("XML-File "+getFilename()+" could not be parsed: " + e.getMessage());
		}

		return doc;
	}

	@Override
	public void close() {}
	
	public String getMetadata(Properties properties) throws RuntimeException {
		throw new RuntimeException("Not implemented in "+this.getClass().getCanonicalName());
	}

	public MetadataCriteria[] getMetadataCriterias() {
		return null;
	}
		
	private List<Element> getRootNodes(Document existing, String root) {
		List<Element> result = new ArrayList<Element>();
		if (root != null) {
			NodeList rootList = existing.getElementsByTagName(root);
			for (int i=0; i<rootList.getLength(); i++) if (rootList.item(i) instanceof Element) result.add((Element)rootList.item(i));
		} else {
			result.add(existing.getDocumentElement());
		}
		if (result.isEmpty()) log.warn("No root nodes with name '"+root+"' found in document.");
		return result;
	}
	
	
	@Override
	public Document save(Document document, Properties properties) throws RuntimeException {
		String mode = properties.getProperty("mode","create");
		String root = properties.getProperty("root");
		boolean sync = properties.getProperty("resync", "false").equalsIgnoreCase("true");
		try {
			if (mode.equalsIgnoreCase("add")) {
				Document existing = open();
				NodeList nodeList = document.getDocumentElement().getChildNodes();
				for (Element n : getRootNodes(existing,root)) {
					for (int i=0; i<nodeList.getLength(); i++) {
						Node imported = existing.adoptNode(nodeList.item(i));
						n.appendChild(imported);
					}
				}
				document = existing;
			}
			if (mode.equalsIgnoreCase("insert")) {
				Document existing = open();
				NodeList nodeList = document.getDocumentElement().getChildNodes();
				for (Element n : getRootNodes(existing,root)) {
					Set<Node> toBeDeleted = new HashSet<Node>();
					Set<Node> toBeImported = new HashSet<Node>();
					for (int i=0; i<nodeList.getLength(); i++) {
						Node imported = existing.adoptNode(nodeList.item(i));
						NodeList sourceList = n.getElementsByTagName(imported.getNodeName());
						for (int j=0; j<sourceList.getLength();j++) {
							toBeDeleted.add(sourceList.item(j));
						}
						toBeImported.add(imported);
					}
					for (Node d : toBeDeleted) {
						n.removeChild(d);
					}
					for (Node imported : toBeImported) {
						n.appendChild(imported);
					}
					
				}
				document = existing;
			}
			if (mode.equalsIgnoreCase("update") && (root != null)) { //only update existing root nodes. Else create document anew.
				Document existing = open();
				NodeList nodeList = document.getDocumentElement().getChildNodes();
				for (Element n : getRootNodes(existing,root)) {
					Set<Node> toBeDeleted = new HashSet<Node>();
					NodeList sourceList = n.getChildNodes();
					for (int j=0; j<sourceList.getLength();j++) {
						toBeDeleted.add(sourceList.item(j));
					}
					for (int i=0; i<nodeList.getLength(); i++) {
						Node imported = existing.adoptNode(nodeList.item(i));
						n.appendChild(imported);
					}
					for (Node d : toBeDeleted) {
						n.removeChild(d);
					}
				}
				document = existing;
			}
			if (mode.equalsIgnoreCase("delete")) {
				Document existing = open();
				NodeList nodeList = document.getDocumentElement().getChildNodes();
				for (Element n : getRootNodes(existing,root)) {
					Set<Node> toBeDeleted = new HashSet<Node>();
					for (int i=0; i<nodeList.getLength(); i++) {
						Node imported = existing.adoptNode(nodeList.item(i));
						NodeList sourceList = n.getElementsByTagName(imported.getNodeName());
						for (int j=0; j<sourceList.getLength();j++) {
							toBeDeleted.add(sourceList.item(j));
						}
					}
					for (Node d : toBeDeleted) {
						n.removeChild(d);
					}
				}
				document = existing;
			}
		}
		catch (RuntimeException e) {
			if (mode.equalsIgnoreCase("delete")) {
				log.warn("Can not delete given nodes from existing document: "+e.getMessage());
				return null;
			}
			log.warn(e.getMessage()+": Creating new document file.");
		}
		
		try {
			 TransformerFactory tFactory = TransformerFactory.newInstance();
			 Transformer transformer;
			 InputStream is = this.getClass().getResourceAsStream("stripSpaces.xsl");
			 Source xsltSource = new StreamSource(is);
			 transformer = tFactory.newTransformer(xsltSource);
			 DOMSource source = new DOMSource(document);
			 FileOutputStream out = new FileOutputStream(filename);
			 OutputStreamWriter writer = new OutputStreamWriter(out,getEncoding());
			 StreamResult resultStream = new StreamResult(writer);
			 String indent = properties.getProperty(OutputKeys.INDENT, "yes");
			 transformer.setOutputProperty(OutputKeys.INDENT, indent);
			 if (indent.equalsIgnoreCase("yes")) transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", properties.getProperty("indent-amount", "2"));
			 transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, properties.getProperty(OutputKeys.OMIT_XML_DECLARATION,"no"));
			 transformer.transform(source, resultStream);
			 writer.close();
			 out.close();
			 if (sync) { //shall we return the transformed document?
				 DOMResult result = new DOMResult();
				 transformer.transform(source, result);
				 return (Document)result.getNode();
			 }
			 return document; //return unchanged document.
		} catch (TransformerConfigurationException e) {
			throw new RuntimeException(e);
		} catch (TransformerException e) {
			throw new RuntimeException(e);
		} catch (FileNotFoundException e) {
			throw new RuntimeException("File "+getFilename()+" not found or file cannot be created: "+e.getMessage());
		} catch (IOException e) {
			throw new RuntimeException("Error writing to file "+getFilename()+": "+e.getMessage());
		}
	}

	protected void checkSSL() throws RuntimeException{		
		String urlString = getFilename();
		if (SSLUtil.supportsSSL(urlString) && sslMode.equals(SSLUtil.SSLModes.trust)) {
			try {
				URL url = new URL(urlString);
				SSLUtil util = new SSLUtil();					
				try {
					util.addCertToKeyStore(url);
				}
				catch (Exception e) {
					if (e instanceof UnknownHostException)
						throw new RuntimeException("Host "+urlString+" is unknown.");
					if (e instanceof ConnectException)
						throw new RuntimeException("Could not connect to host "+urlString+" : "+e.getMessage());
					if (e instanceof MalformedURLException) 
						throw new RuntimeException("Filename "+getFilename()+" is not a legal URL. SSL trust mode is only available for URLs.");
					throw new RuntimeException(e);						
				}
			}			
			catch (Exception e) {
				throw new RuntimeException(e);
			}			
		}
	}

}

