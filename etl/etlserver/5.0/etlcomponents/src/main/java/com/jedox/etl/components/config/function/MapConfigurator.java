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
import java.util.StringTokenizer;
import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.function.FunctionConfigurator;
import com.jedox.etl.core.util.NamingUtil;

public class MapConfigurator extends FunctionConfigurator {
	
	private HashMap<String,String> getMapFromString(String mapSpec) {
		//mapSpec=mapSpec.replaceAll(NamingUtil.spaceValue(), " ");
		HashMap<String,String> map = new HashMap<String,String>();
		StringTokenizer t = new StringTokenizer(mapSpec, ",");
		while (t.hasMoreTokens()) {
			String entity = t.nextToken();
			String[] pair = entity.split("=");
			String input = pair[0].trim();
			if (input.equals(NamingUtil.spaceValue()))
				input=" ";
			// Map to space if right side is empty 
			String output = "";
			if (pair.length==2)
				output = pair[1].trim();
			if (output.equals(NamingUtil.spaceValue()))
				output=" ";			
			map.put(input, output);
		}
		return map;
	}
	
	public HashMap<String,String> getMap() throws ConfigurationException {
		Element mapping = getXML().getChild("mapping");
		if (mapping != null) {
			// configuration via mapping xml structure is not used currently
			HashMap<String,String> map = new HashMap<String,String>();
			for (Element m : getChildren(mapping,"map")) {
				String input = m.getAttributeValue("from").trim();
				map.put(input, m.getAttributeValue("to"));
			}
			return map;
		}
		return getMapFromString(getParameter("map",""));
	}
	
	public String getDefault() throws ConfigurationException {
		Element mapping = getXML().getChild("mapping");
		if (mapping != null)
			return mapping.getAttributeValue("default");
		return getParameter("default",null);
	}
}
