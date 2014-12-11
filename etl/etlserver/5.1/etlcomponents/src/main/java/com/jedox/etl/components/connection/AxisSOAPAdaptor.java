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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.net.URL;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.io.StringWriter;
import javax.xml.namespace.QName;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import org.apache.axiom.om.OMAbstractFactory;
import org.apache.axiom.om.OMElement;
import org.apache.axiom.om.OMFactory;
import org.apache.axiom.om.OMNamespace;
import org.apache.axis2.addressing.EndpointReference;
import org.apache.axis2.client.Options;
import org.apache.axis2.client.ServiceClient;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.dom.DOMSource;

import com.jedox.etl.components.config.connection.ServiceDescriptor;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.XMLUtil;

public class AxisSOAPAdaptor {

	private ServiceDescriptor descriptor;
	private ServiceClient service;
	private static final Log log = LogFactory.getLog(AxisSOAPAdaptor.class);

	public AxisSOAPAdaptor(ServiceDescriptor descriptor) throws RuntimeException {
		this.descriptor = descriptor;
		try {
			QName serviceName = null;
			if (descriptor.getServiceName() != null) {
				serviceName = new QName(descriptor.getTargetNamespace(),descriptor.getServiceName());
			}
			//endpoint url has to be in wsdl file!!
			URL url =  new URL(descriptor.getWsdlURL());
			service = new ServiceClient(null,url,serviceName,descriptor.getPortName());
		}
		catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}

	}

	private OMElement createParameter(ServiceDescriptor.MethodParameter parameter, OMFactory fac, OMNamespace omNs) {
		OMElement value = fac.createOMElement(parameter.getName(),omNs);
    	if (parameter.getParameters().isEmpty()) {
    		value.setText(parameter.getValue());
    	}
    	else {
    		for (ServiceDescriptor.MethodParameter p: parameter.getParameters()) {
    			OMElement subvalue = createParameter(p, fac, omNs);
    			value.addChild(subvalue);
    		}
    	}
    	return value;
	}
	
	private OMElement createPayLoad() {
        OMFactory fac = OMAbstractFactory.getOMFactory();
        OMNamespace omNs = fac.createOMNamespace(descriptor.getTargetNamespace(), "ns1");
        OMElement method = fac.createOMElement(descriptor.getOperationName(), omNs);
        for (ServiceDescriptor.MethodParameter parameter: descriptor.getParameter()) {
        	OMElement value = createParameter(parameter,fac,omNs);
			method.addChild(value);
		}
        log.debug("Body: " + method);
        return method;
	}
	
	private OMElement getHeaderParameter(ServiceDescriptor.MethodParameter parameter) {
		OMFactory fac = OMAbstractFactory.getOMFactory(); 
	    OMNamespace omNs = fac.createOMNamespace(descriptor.getTargetNamespace(), "ns1");
	    return createParameter(parameter, fac, omNs);
	}

	public Document execute(String username, String password) throws RuntimeException {
		try {
			QName operationName = new QName(descriptor.getTargetNamespace(),descriptor.getOperationName());
			// Create the options

			Options opts = new Options();
			//Setting action ,and which can be found from the wsdl of the service
			opts.setTo(descriptor.getServiceUrl() != null ? new EndpointReference(descriptor.getServiceUrl()) : new EndpointReference(descriptor.getWsdlURL()));
			//opts.setAction(operationName.toString());
			opts.setUserName(username);
			opts.setPassword(password);
			String namespace = operationName.getNamespaceURI();
			// for a reason, some namespaces are stored without / at the end, for our purpose they always should have this
			if(!namespace.endsWith("/"))
				namespace = namespace.concat("/");
			opts.setAction(namespace + operationName.getLocalPart());
			//opts.setProperty(org.apache.axis2.Constants.Configuration.HTTP_METHOD,HTTPConstants.HTTP_METHOD_GET);

			log.info("Executing SOAP Action: " + namespace + operationName.getLocalPart());
			//setting created option into service client
			service.setOptions(opts);
			for (ServiceDescriptor.MethodParameter parameter : descriptor.getHeaderParameter()) {
				OMElement headerParam = getHeaderParameter(parameter);
				log.debug("Added to Header:" + headerParam);
				service.addHeader(headerParam);
			}

			OMElement res = service.sendReceive(operationName,createPayLoad());
			//this will not work for transform because of a xalan bug :-(
			//StAXSource source = new StAXSource(res.getXMLStreamReader());
			StringWriter writer = new StringWriter();
			res.serialize(writer);
			service.cleanup();
			service.cleanupTransport();
			//res.serialize(System.out);
			return stripNamespaces(new StreamSource(new StringReader(writer.toString())));
			} catch (Exception e) {
				throw new RuntimeException(e.getMessage());
			}
	}

	private Document stripNamespaces(Source source) throws RuntimeException{
		try {
			InputStream is = this.getClass().getResourceAsStream("stripNamespaces.xsl");
			Source xsltSource = new StreamSource(is);
			DOMResult result = new DOMResult();
			//System.setProperty("javax.xml.transform.TransformerFactory","net.sf.saxon.TransformerFactoryImpl");
			//System.setProperty("javax.xml.transform.TransformerFactory","org.apache.xalan.xsltc.trax.TransformerFactoryImpl");

			// create an instance of TransformerFactory
			TransformerFactory transFact = TransformerFactory.newInstance();
			Transformer trans = transFact.newTransformer(xsltSource);
			trans.transform(source, result);
			StringWriter sw = new StringWriter();
			DOMSource ds = new DOMSource(result.getNode());
			transFact.newTransformer().transform(ds, new StreamResult(sw));
			// some signs like &,<,> are sometimes not correctly displayed, so this should be fixed again
			String encodedXML = XMLUtil.stringEncodeXml(sw.toString());

			if(descriptor.getXmlInfo()){
				int part = 1;
				int startIndex = 0;
				int blockSize = 800;
				int endIndex = Math.min(encodedXML.length(), blockSize);
				log.info("Starting printing Webservice output.");
				log.info("Webservice output part" + part + ":\n" + encodedXML.substring(startIndex,endIndex));
				while(endIndex < encodedXML.length()){
					startIndex = endIndex +1;
					blockSize += 800;
					part++;
					endIndex = Math.min(encodedXML.length(), blockSize);
					log.info("Webservice output part" + part + ":\n" + encodedXML.substring(startIndex,endIndex));
				}
				log.info("Finished printing Webservice output.");
			}

			DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();

			Document document = null;
			try {
				DocumentBuilder builder = factory.newDocumentBuilder();
				document = builder.parse(new java.io.ByteArrayInputStream(encodedXML.getBytes()));
			} catch (SAXException e) {
				// if this for a certain reason did not work, try to send the result as it is
				log.warn("No check on the xml is done: " + e.getMessage());
				return (Document) result.getNode();
			} catch (IOException e) {
				// if this for a certain reason did not work, try to send the result as it is
				log.warn("No check on the xml is done: " + e.getMessage());
				return (Document) result.getNode();
			} catch (ParserConfigurationException e) {
				// if this for a certain reason did not work, try to send the result as it is
				log.warn("No check on the xml is done: " + e.getMessage());
				return (Document) result.getNode();
			}
			return document;

		} catch (TransformerException e) {
			throw new RuntimeException(e.getMessage());
		}
	}

}
