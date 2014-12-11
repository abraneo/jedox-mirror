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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.load;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.load.LoadConfigurator;


/**
 * This class configures the XML tag for Load of type OLAPRule, it parses the cube name,
 * the source and the connection are handled in the super class LoadConfigurator
 * 
 */
public class RuleConfigurator extends LoadConfigurator {

	private String cubeName;
	
	/**
	 * configure the super class
	 */
	public void configure() throws ConfigurationException {
		super.configure();
		String name = getName();
		Element cube = getXML().getChild("cube");
		if (cube != null) {
			cubeName = cube.getAttributeValue("name",name).trim();
		}
		
	}

	/**
	 * get the cube name from <cube> tag
	 * @return cube name
	 */
	public String getCubeName() {
		return cubeName;
	}
	
	
}

