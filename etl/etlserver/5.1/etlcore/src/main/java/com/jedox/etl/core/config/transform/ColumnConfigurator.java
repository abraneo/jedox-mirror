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
package com.jedox.etl.core.config.transform;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;
/**
 * Helper class for the configuration of output fields used in transforms 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ColumnConfigurator {
	
	private static final Log log = LogFactory.getLog(ColumnConfigurator.class);
	private String name;
	
	public ColumnConfigurator(String name) {
		this.name = name;
	}
	
	/**
	 * gets the name of the transform
	 * @return the transform name
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * gets the name of the column
	 * @param column the XML defining the column
	 * @param inputName the input name used as fallback, if no column name is specified
	 * @return the column name
	 */
	public String getColumnName(Element column, String inputName) {
		return column.getAttributeValue("name",inputName);
	}
	
	private String getInputNameInternal(Element column) {
		Element input = column.getChild("input");
		//constant column may have have their constant as text and thus need attribute name
		if (input == null) 
			return column.getAttributeValue("name");
		//variable columns have their input name in the input element
		String name = input.getAttributeValue("nameref");
		if (name != null) return name;
		//return a default name for constant inputs
		return "constant";
	}
	
	/**
	 * gets the name of the input field for this output column
	 * @param column the XML defining the output column
	 * @return the input field name
	 */
	public String getInputName(Element column) {
		String name = getInputNameInternal(column);
		if (name == null)
			log.error("Target "+getName()+" has column without name!");
		return name;
	}
	
	private String getText(Element element) {
		String text = element.getTextTrim();
		if (text.equals("")) return null;
		return text;
	}
	
	/**
	 * gets the input value for a constant field used as input for this output column 
	 * @param column the XML defining the output column
	 * @return the input field value
	 */
	public String getInputValue(Element column) {
		Element input = column.getChild("input");
		//constant column may have have their constant as text
		if (input == null) 
			return getText(column);
		//still support format, where input has the constant
		return input.getAttributeValue("constant");
	}
	
	public boolean isInputConstant(Element column) {
		Element input = column.getChild("input");
		if (input == null) 
			return false;
		
		return (input.getAttribute("constant")!=null);
	}

}
