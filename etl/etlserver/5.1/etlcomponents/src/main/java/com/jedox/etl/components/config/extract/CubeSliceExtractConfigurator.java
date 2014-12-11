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
**
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.extract;

import java.util.Random;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;

public class CubeSliceExtractConfigurator extends CubeExtractConfigurator {
	
	Random rand = null;
	int min = 0;
	int max = 0;
	
	public enum NO_FILTER_MODE{
		generateRoots,
		generateBases,
		exclude,
		emptySource
	}

	
	NO_FILTER_MODE mode;
	private String fixValue="0";
	private boolean isRandomPaths;

	public NO_FILTER_MODE getNoFilterMode()  {
		return mode;
	}
	
	public boolean isRandomPaths() {
		return isRandomPaths;
	}

	public String getFixValue() {
		if(rand!=null){
			return String.valueOf(rand.nextDouble()*((double)max-(double)min)+min);
		}
		return fixValue;
	}	
	
	public void configure() throws ConfigurationException {
		super.configure();
		Element cube = null;
		Element query = getXML().getChild("query");
		if (query != null){
			mode = NO_FILTER_MODE.valueOf(query.getAttributeValue("mode","exclude"));
			isRandomPaths = Boolean.parseBoolean(query.getAttributeValue("randomPaths","false"));
			cube = query.getChild("cube");
		}
		cubeName = getName();
		if (cube != null) {
			cubeName = cube.getAttributeValue("name").trim();
			valueName = cube.getAttributeValue("valuename",valueName).trim();
			fixValue = cube.getAttributeValue("fixValue","0").trim();
			if(fixValue.startsWith("#random")){
				try{
					min = Integer.parseInt(fixValue.substring(8, fixValue.indexOf(',')));
					max = Integer.parseInt(fixValue.substring(fixValue.indexOf(',')+1,fixValue.length()-1));
					if(min>=max){
						throw new Exception ("the minimum value is greater or equal to the maximum one");
					}
					rand = new Random();
				}catch(Exception e){
					throw new ConfigurationException("Error while evaluating the "+ fixValue +" expression: " + e.getMessage());
				}
			}
		}
	}

}