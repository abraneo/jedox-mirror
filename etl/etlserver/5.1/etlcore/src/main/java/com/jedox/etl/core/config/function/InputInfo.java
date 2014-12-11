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
package com.jedox.etl.core.config.function;

import java.util.List;
import java.util.ArrayList;

import com.jedox.etl.core.component.ConfigurationException;

import org.jdom.Element;

/**
 * Helper class for Function Configurators getting input field names, types and values.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class InputInfo {
	
	private String[] inputNames;
	private String[] inputValues;
	private String[] inputTypes;
	
	private void fill(List<Element> inputs) throws ConfigurationException {
		inputNames = new String[inputs.size()];
		inputValues = new String[inputs.size()];
		inputTypes = new String[inputs.size()];
		for (int j=0; j<inputs.size(); j++) {
			Element input = inputs.get(j);
			String inputref = input.getAttributeValue("nameref");
			if (inputref == null) { //constant
				String constant = input.getAttributeValue("constant");
				if (constant != null) {
					inputref = "constant";
					inputValues[j] = constant;
				}
				else 
					throw new ConfigurationException("Input has to have either attribute nameref or constant.");
			}
			inputNames[j] = inputref;
			inputTypes[j] = input.getAttributeValue("type");
		}
	}
	
	private List<Element> getInputs(List<Element> inputs, String target) {
		ArrayList<Element> result = new ArrayList<Element>();
		for (int j=0; j<inputs.size(); j++) {
			Element input = inputs.get(j);
			String t = input.getAttributeValue("target",target);
			if (t.equalsIgnoreCase(target)) {
				result.add(input);
			}
		}
		return result;
	}
	
	public InputInfo(List<Element> inputs) throws ConfigurationException {
		fill(inputs);
	}
	
	public InputInfo(List<Element> inputs, String target) throws ConfigurationException {
		fill(getInputs(inputs, target));
	}
	
	/**
	 * gets the input names
	 * @return the input names
	 */
	public String[] getInputNames() {
		return inputNames;
	}
	
	/**
	 * gets the input values
	 * @return the input values
	 */
	public String[] getInputValues() {
		return inputValues;
	}
	
	/**
	 * gets the input types
	 * @return the input types
	 */
	public String[] getInputTypes() {
		return inputTypes;
	}

}
