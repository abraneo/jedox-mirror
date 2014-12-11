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

import java.util.List;

public class ServiceDescriptor {

	public class MethodParameter {
		private String name;
		private String value;
		private List<MethodParameter> parameters;
		
		public String getName() {
			return name;
		}
		public void setName(String name) {
			this.name = name;
		}
		public String getValue() {
			return value;
		}
		public void setValue(String value) {
			this.value = value;
		}
		
		public void setParameters(List<MethodParameter> parameters) {
			this.parameters = parameters;
		}
		public List<MethodParameter> getParameters() {
			return parameters;
		}
	}

	private String serviceName;
	private String portName;
	private String operationName;
	private String wsdlURL;
	private String serviceUrl;
	private String targetNamespace;
	private boolean xmlInfo=false;
	private List<MethodParameter> parameter;
	private List<MethodParameter> headerParameter;

	public String getServiceName() {
		return serviceName;
	}
	public void setServiceName(String serviceName) {
		this.serviceName = serviceName;
	}
	public String getPortName() {
		return portName;
	}
	public void setPortName(String portName) {
		this.portName = portName;
	}
	public String getOperationName() {
		return operationName;
	}
	public void setOperationName(String operationName) {
		this.operationName = operationName;
	}
	public String getWsdlURL() {
		return wsdlURL;
	}
	public void setWsdlURL(String wsdlURL) {
		this.wsdlURL = wsdlURL;
	}
	public List<MethodParameter> getParameter() {
		return parameter;
	}
	public void setParameter(List<MethodParameter> parameter) {
		this.parameter = parameter;
	}
	public String getTargetNamespace() {
		return targetNamespace;
	}
	public void setTargetNamespace(String targetNamespace) {
		this.targetNamespace = targetNamespace;
	}
	public void setXmlInfo(String xmlInfo) {
		this.xmlInfo = Boolean.valueOf(xmlInfo);
	}

	public boolean getXmlInfo(){
		return xmlInfo;
	}
	public void setServiceUrl(String serviceUrl) {
		this.serviceUrl = serviceUrl;
	}
	public String getServiceUrl() {
		return serviceUrl;
	}
	public void setHeaderParameter(List<MethodParameter> headerParameter) {
		this.headerParameter = headerParameter;
	}
	public List<MethodParameter> getHeaderParameter() {
		return headerParameter;
	}

}
