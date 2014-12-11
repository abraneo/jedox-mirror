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
package com.jedox.etl.components.function;

import java.util.HashMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import com.jedox.etl.components.config.function.MapConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.NamingUtil;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Map extends Function {

	private HashMap<String,String> map = new HashMap<String,String>();
	private String defaultValue;
	private String type;
	private static final Log log = LogFactory.getLog(Map.class);	
	
	
	public Map() {
		setConfigurator(new MapConfigurator());
	}
	
	public MapConfigurator getConfigurator() {
		return (MapConfigurator)super.getConfigurator();
	}
	
	protected Object transform(Row input) {
		String value = input.getColumn(0).getValueAsString();
		if (value.equals(NamingUtil.spaceValue()))
			// #space is interpreted as ' ' in mapping
			log.warn("In Function "+getName()+": Mapping cannot be applied for data "+NamingUtil.spaceValue());
		String result = map.get(value);
		if (result == null)
			result = (defaultValue == null) ? value : defaultValue;
		return result;
	}
	
	public String getValueType() {
		return type;
	}
	
	protected void validateParameters() throws ConfigurationException {
		map = getConfigurator().getMap();
		defaultValue = getConfigurator().getDefault();
		type = getParameter("type","java.lang.String");
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,false);
	}
	
	public void close() {
		if(map!=null){
			map.clear();
			map=null;
		}
	}

}
