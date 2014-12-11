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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.io.IOException;
import java.io.StringWriter;
import java.net.URL;
import java.util.List;
import java.util.Properties;

import javax.wsdl.Binding;
import javax.wsdl.Definition;
import javax.wsdl.Operation;
import javax.wsdl.Port;
import javax.wsdl.PortType;
import javax.wsdl.Service;
import javax.wsdl.WSDLException;
import javax.wsdl.factory.WSDLFactory;
import javax.wsdl.xml.WSDLReader;
import javax.xml.namespace.QName;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import com.jedox.etl.components.config.connection.ServiceConnectionConfigurator;
import com.jedox.etl.components.config.connection.ServiceDescriptor;
import com.jedox.etl.components.config.connection.ServiceDescriptor.MethodParameter;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.util.SSLUtil;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;

public class ServiceConnection extends Connection implements IXmlConnection {

	private static final Log log = LogFactory.getLog(ServiceConnection.class);
	private ServiceDescriptor descriptor;

	public ServiceConnection() {
		setConfigurator(new ServiceConnectionConfigurator());
	}

	public ServiceConnectionConfigurator getConfigurator() {
		return (ServiceConnectionConfigurator) super.getConfigurator();
	}

	public void close() {
		// nothing to to here
	}

	public  Document open() throws RuntimeException {
		log.debug("Start extracting from SOAP-connection " + getName() );
		checkDescriptor(descriptor);
		AxisSOAPAdaptor adaptor = new AxisSOAPAdaptor(descriptor);
		Document doc = adaptor.execute(getUser(),getPassword());
		adaptor = null;
		return doc;

		//SOAPAdaptor a = new SOAPAdaptor(descriptor);
		//return a.execute();
	}

	@SuppressWarnings("unchecked")
	private void checkDescriptor(ServiceDescriptor descriptor) throws RuntimeException {
		//check against wsdl
		Definition definition = getWSDLDefinition(descriptor.getWsdlURL());
		String targetNamespace = definition.getTargetNamespace();
		descriptor.setTargetNamespace(targetNamespace);
		if(descriptor.getServiceName() != null){
			Service service = definition.getService(new QName(targetNamespace,descriptor.getServiceName()));
			if (service == null)//we do not use the default service
				throw new RuntimeException("Service "+descriptor.getServiceName()+" not found.");
			if (descriptor.getPortName() != null) {//we do not use the default port
				Port port = service.getPort(descriptor.getPortName());
				if (port == null)
					throw new RuntimeException("Port "+descriptor.getPortName()+" not found.");
				Binding binding = port.getBinding();
				PortType portType = binding.getPortType();
				List<Operation> operations = portType.getOperations();
				boolean opFound = false;
				for (Operation operation : operations) {
					if (operation.getName().equals(descriptor.getOperationName())) {
						opFound = true;
						break;
						//QName typeName = operation.getInput().getMessage().getPart("parameters").getTypeName();
					}
				}
				if (!opFound)
					throw new RuntimeException("Operation "+ descriptor.getOperationName() +" not found.");
			}
		}
	}

	private Definition getWSDLDefinition(String wsdlURI) throws RuntimeException {
		try {

			WSDLFactory factory = WSDLFactory.newInstance();
			WSDLReader reader = factory.newWSDLReader();
			reader.setFeature("javax.wsdl.verbose", false);
			Definition definition = reader.readWSDL(wsdlURI);
			return definition;
		} catch (WSDLException e) {
			throw new RuntimeException("Error parsing wsdl file: "+e.getMessage()+", please check the WSDL url in your browser.");
		}
	}
	
	public String getMetadata(Properties properties) throws RuntimeException {
		throw new RuntimeException("Not implemented in "+this.getClass().getCanonicalName());
	}

	public MetadataCriteria[] getMetadataCriterias() {
		return null;
	}
	
	
	public void init() throws InitializationException {
		try {
			super.init();
			descriptor = getConfigurator().getServiceDescriptor();
			try {
				String sslMode = getParameter("ssl",SSLUtil.SSLModes.verify.toString());
				if (sslMode.equalsIgnoreCase(SSLUtil.SSLModes.trust.toString())) {
					SSLUtil util = new SSLUtil();
					String urlString = descriptor.getWsdlURL();
					URL url = new URL(urlString);
					util.addCertToKeyStore(url);
				}
			}
			catch (Exception e) {
				throw new InitializationException(e);
			}
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

	@Override
	public Document save(Document document, Properties properties) throws RuntimeException {
		if (document.getDocumentElement() == null) throw new RuntimeException("Document root element is not set.");
		
		try {
			//transform document to string
			TransformerFactory tFactory = TransformerFactory.newInstance();
			Transformer transformer;
			transformer = tFactory.newTransformer();
			DOMSource source = new DOMSource(document);
			StringWriter writer = new StringWriter();
			StreamResult resultStream = new StreamResult(writer);
			transformer.setOutputProperty(OutputKeys.INDENT, properties.getProperty(OutputKeys.INDENT, "no"));
			transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, properties.getProperty(OutputKeys.OMIT_XML_DECLARATION,"yes"));
			transformer.transform(source, resultStream);
			writer.close();
			//add document string as parameter
			ServiceDescriptor descriptor = getConfigurator().getServiceDescriptor();
			MethodParameter parameter = descriptor.new MethodParameter();
			parameter.setName(properties.getProperty("name",document.getDocumentElement().getNodeName()));
			parameter.setValue(new String(writer.toString().getBytes(),getEncoding()));
			descriptor.getParameter().add(parameter);
			//call service
			checkDescriptor(descriptor);
			AxisSOAPAdaptor adaptor = new AxisSOAPAdaptor(descriptor);
			Document doc = adaptor.execute(getUser(),getPassword());
			adaptor = null;
			return doc;
		} catch (TransformerConfigurationException e) {
			throw new RuntimeException(e);
		} catch (TransformerException e) {
			throw new RuntimeException(e);
		} catch (IOException e) {
			throw new RuntimeException(e);
		} catch (ConfigurationException e) {
			throw new RuntimeException(e);
		}
	}

}
