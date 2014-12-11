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
package com.jedox.etl.service;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Enumeration;
import java.util.Vector;
import java.util.Map.Entry;

import javax.servlet.ServletContext;

import org.apache.axis2.context.MessageContext;
import org.apache.axis2.transport.http.HTTPConstants;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class InfoService {
	
	private static Log log = LogFactory.getLog(InfoService.class);

	public String[] getSystemProperties()
	{
		log.debug("getSystemProperties");
		Vector<String> res = new Vector<String>();
		for (Entry<Object,Object> e: System.getProperties().entrySet())
		{
			res.add(e.getKey().toString());
			res.add(e.getValue().toString());
		}
		String[] resArr = new String[res.size()];
		return res.toArray(resArr);
	}
	
	public ContextInfo getServletContextInfos()
	{
		log.debug("getServletContextInfos");
		MessageContext msgCtx = MessageContext.getCurrentMessageContext();
		//HttpServlet servlet = 
		//	(HttpServlet)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLET); 
		//	      ServletContext servletContext = servlet.getServletContext(); 
		
		ServletContext servletContext = (ServletContext)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLETCONTEXT);
			
		
		ContextInfo ci = new ContextInfo();
		Vector<String> attributes = new Vector<String>();
		Enumeration<?> attrs = servletContext.getAttributeNames();
		while (attrs.hasMoreElements())
		{
			String elem = (String)attrs.nextElement();
			attributes.add(elem);
			attributes.add(servletContext.getAttribute(elem).toString());
		}
		String[] attrArr = new String[attributes.size()];
		ci.setAttributes(attributes.toArray(attrArr));
		
		Vector<String> parameters = new Vector<String>();
		Enumeration<?> params = servletContext.getInitParameterNames();
		while (params.hasMoreElements())
		{
			String elem = (String)params.nextElement();
			parameters.add(elem);
			parameters.add(servletContext.getInitParameter(elem).toString());
		}
		String[] paramArr = new String[parameters.size()];
		ci.setParameters(parameters.toArray(paramArr));
		
		Integer maj = servletContext.getMajorVersion();
		Integer min = servletContext.getMinorVersion();
		ci.setVersion(maj.toString() + "." + min.toString());
		
		ci.setPath(servletContext.getRealPath("/"));
		
		ci.setInfo(servletContext.getServerInfo());
		
		log.info("IN CI" + ci.getVersion());
		
		return ci;
	}
	
	public String getTempDir() {
		log.debug("getTempDir");
		MessageContext msgCtx = MessageContext.getCurrentMessageContext();
		//HttpServlet servlet = 
		//	(HttpServlet)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLET); 
		//	      ServletContext servletContext = servlet.getServletContext(); 
		
		ServletContext servletContext = (ServletContext)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLETCONTEXT);
		File tmpDir = (File)servletContext.getAttribute("javax.servlet.context.tempdir");
		return tmpDir.getAbsolutePath().toString();
	}
	
	public String getRealPath() {
		log.debug("getRealPath");
		MessageContext msgCtx = MessageContext.getCurrentMessageContext();
		//HttpServlet servlet = 
		//	(HttpServlet)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLET); 
		//	      ServletContext servletContext = servlet.getServletContext(); 
		
		ServletContext servletContext = (ServletContext)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLETCONTEXT);
		return servletContext.getRealPath("/");
	}
	
	public String getResource(String path) {
		log.info("getResource");
		if (path == null || !path.startsWith("/")) {
			path = "/" + path;
		}
		MessageContext msgCtx = MessageContext.getCurrentMessageContext();
		//HttpServlet servlet = 
		//	(HttpServlet)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLET); 
		//	      ServletContext servletContext = servlet.getServletContext(); 
		
		ServletContext servletContext = (ServletContext)msgCtx.getProperty(HTTPConstants.MC_HTTP_SERVLETCONTEXT);
		
		try {
			URL res = servletContext.getResource(path);
			if (res == null) {
				return null;
			}
			return res.toString();
		} catch (MalformedURLException e) {
			log.error(e.getMessage());
		}
		return "empty";
	}

}
