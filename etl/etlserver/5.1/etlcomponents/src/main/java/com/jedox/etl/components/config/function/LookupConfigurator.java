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
package com.jedox.etl.components.config.function;

import java.util.HashMap;
import java.util.Map;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.config.function.FunctionConfigurator;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.ViewSource;

public class LookupConfigurator extends FunctionConfigurator {
	
	String[] fromColumns = null;
	
	public String getSeparator() {
		return ",";
	}
	
	public String[] getFromColumns() throws ConfigurationException {
		if(fromColumns==null){
			String froms = getParameter("from",null);
			if(froms!=null && !froms.trim().isEmpty()){
				fromColumns = froms.split(getSeparator());
			}
		}
		return fromColumns;
	}

	public ISource getSource() throws ConfigurationException {
		Element source = getXML().getChild("source");
		if (source == null) {
			// Use parameter notation of functions
			String sourceName = getParameter("source",null);
			if (sourceName == null ) {
				throw new ConfigurationException("No source is defined in Lookup.");				
			}
			// create source element from parameter
			source = new Element("source");
			source.setAttribute("nameref", sourceName);
		}	
		try {
			return SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), this, getContext(), source);
		}
		catch (CreationException e) {
			throw new ConfigurationException(e.getMessage());
		}
	}
	
	public String getSourceName() throws ConfigurationException {
		Element s = getXML().getChild("source");
		if (s != null)
			return s.getAttributeValue("nameref");
		return getParameter("source",null);
	}
	
	public String getDefault() throws ConfigurationException {
		Element mapping = getXML().getChild("mapping");
		if (mapping != null)
			return mapping.getAttributeValue("default");
		return getParameter("default",null);
	}
	
	public Map<String,String> getMap() throws ConfigurationException {
		HashMap<String,String> map = new HashMap<String,String>();
		Element mapping = getXML().getChild("mapping");
		if (mapping != null) {
			for (Element m : getChildren(mapping,"map")) {
				map.put(m.getAttributeValue("from"), m.getAttributeValue("to"));
			}
			return map;
		}
		else {
			map.put(getParameter("from",null), getParameter("to",null));
		}
		return map;
	}

	public String getTreeFormat() throws ConfigurationException {
		return getParameter("treeformat","EA");
	}
	
	
}
