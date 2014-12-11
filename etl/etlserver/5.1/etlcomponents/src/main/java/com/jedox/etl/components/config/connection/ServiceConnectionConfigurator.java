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
package com.jedox.etl.components.config.connection;

import com.jedox.etl.components.config.connection.ServiceDescriptor.MethodParameter;
import com.jedox.etl.core.config.connection.ConnectionConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import java.util.ArrayList;
import java.util.List;
import org.jdom.Element;


public class ServiceConnectionConfigurator extends ConnectionConfigurator {

	@SuppressWarnings("unchecked")
	public ServiceDescriptor getServiceDescriptor() throws ConfigurationException {
		ServiceDescriptor descriptor = new ServiceDescriptor();
		//analyse service document
		String serviceName = getParameter("service",null);
		String wsdlURL = getParameter("wsdl",null);
		String serviceURL = getParameter("url",null);
		if (wsdlURL == null || wsdlURL.equals("")) {// a url should be defined
			throw new ConfigurationException("WSDL Url is not defined in connection " + getName());
		}
		String portName = getParameter("port",null);
		String xmlInfo = getParameter("xmlInfo",null);
		String operationName = getParameter("operation",null);
		if (operationName == null || operationName.equals("")) {//a method or operation should be defined
			throw new ConfigurationException("Operation name is not defined in connection " + getName());
		}
		List<MethodParameter> parameter = new ArrayList<MethodParameter>();
		Element parameters = getXML().getChild("parameters");
		if (parameters != null) {
			parameter.addAll(getElements(parameters.getChildren(),descriptor));
		}

		List<MethodParameter> headerParameter = new ArrayList<MethodParameter>();
		if (getXML().getChild("header") != null) {
			headerParameter.addAll(getElements(getXML().getChild("header").getChildren(),descriptor));
		}

		//set descriptor
		descriptor.setServiceName(serviceName);
		descriptor.setPortName(portName);
		descriptor.setOperationName(operationName);
		descriptor.setWsdlURL(wsdlURL);
		descriptor.setParameter(parameter);
		descriptor.setHeaderParameter(headerParameter);
		descriptor.setServiceUrl(serviceURL);
		descriptor.setXmlInfo(xmlInfo);
		return descriptor;
	}
	
	@SuppressWarnings("unchecked")
	private List<MethodParameter> getElements(List<Element> parameterElements, ServiceDescriptor descriptor) {
		List<MethodParameter> parameter = new ArrayList<MethodParameter>();
		if (parameterElements != null) for (Element e : parameterElements) {
			MethodParameter p = descriptor.new MethodParameter();
			p.setName(e.getAttributeValue("name"));
			p.setParameters(getElements(e.getChildren(),descriptor));
			String value = e.getTextTrim();
			if (!value.isEmpty())
				p.setValue(value);
			parameter.add(p);
		}
		return parameter;
	}
	
}
