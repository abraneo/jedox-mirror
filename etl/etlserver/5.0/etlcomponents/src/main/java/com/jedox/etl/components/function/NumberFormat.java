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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.function;

import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.TypeConversionUtil;

/**
 * @author khaddadin
 *
 */
public class NumberFormat extends Function {

	private String pattern;
	private String defaultValue;
	private String exponentialFormat;
	private String moveSign;
	private String groupingSeparator;
	private String decimalSeparator;

	/**
	 * Gets the resolved value according to the template and the actual input.
	 *
	 * @return the resolved value
	 */
	public Object transform(Row values) throws FunctionException {

		String input = values.getColumn(0).getValueAsString();
		if (input.isEmpty()) {
		  return "";
		}
		input = new TypeConversionUtil().convertToNumeric(input);

		// move the sign to the left if it was in the left side
		// should be done before anything else because java does
		// not recognize numbers when the sign is on the right side
		String minusSign = "-";
		if(moveSign != null && moveSign.equals("left")
				&& input.endsWith("-")){ // when it is minus and it is on the right side

			input = minusSign.concat(input.substring(0,input.length()-1));
		}


		double number = 0;

    	// first check if it is a number, if not return the default value, or
		// return the input as it is if no default value exists
		try {
			number = Double.parseDouble(input);
		} catch (Exception e) {
			if (defaultValue == null) {
				return input;
			} else {
				return defaultValue;
			}
		}

		String formatedNumber = "";

		// Apply the format pattern
		try{
		if (pattern != null) {
			DecimalFormat formatter = new DecimalFormat(pattern);
			DecimalFormatSymbols symbols = formatter.getDecimalFormatSymbols();
			if(decimalSeparator != null) symbols.setDecimalSeparator(decimalSeparator.charAt(0));
			if(groupingSeparator != null) symbols.setGroupingSeparator(groupingSeparator.charAt(0));
			formatter.setDecimalFormatSymbols(symbols);
			formatedNumber = formatter.format(number);
		} else {
			formatedNumber = input;
		}
		}
		catch(Exception e){
			throw new FunctionException("Error while applying the pattern: " + e.getMessage());
		}

		// should be in exponential form (contains E)
		// should not be E-0
		if (formatedNumber.indexOf("E-", 1) == -1 && // when it has no E-0 already
				formatedNumber.indexOf('E') != -1) { // when it is exponential

			// add a '+' when the number should be in excel format (E+0)
			// and that the number is already in exponential format
			// that contains E but not '-E'
			if (exponentialFormat.equals("E+0")
					&& formatedNumber.indexOf("E+", 1) == -1) { // when it has not already E+0

				formatedNumber = formatedNumber.substring(0,
						formatedNumber.indexOf('E') + 1).concat("+").concat(
								formatedNumber.substring(
										formatedNumber.indexOf('E') + 1, formatedNumber.length()));
			}

			// remove a '+' when the number should be in excel format (E0)
			// and that the number is already in exponential format
			// that contains E but not '-E'
			if (exponentialFormat.equals("E0")
					&& formatedNumber.indexOf("E+", 1) != -1) { // when it has already E+0

				formatedNumber = formatedNumber.substring(0,
						formatedNumber.indexOf('+')).concat(
								formatedNumber.substring(formatedNumber.indexOf('+') + 1, formatedNumber.length()));
			}

			//move the signs to the right if wanted
			if (moveSign.equals("right")
					&& formatedNumber.startsWith("-")) { // when it is minus and it is on the left side

					formatedNumber = formatedNumber.substring(1,formatedNumber.length()).concat(minusSign);
				}

		}

		return formatedNumber;
	}

	protected void validateParameters() throws ConfigurationException {

		pattern = getParameter("pattern", null);
		if(pattern != null && (!pattern.contains("E")) && getParameter("exponentialFormat", null)!= null){
			getLog().warn("In Function " + getName() + ", parameter exponentialFormat will be ignored because the pattern produces a non-scientific number notation");
		}

		defaultValue = getParameter("default", null);
		exponentialFormat = getParameter("exponentialFormat", "E0");
		if(!(exponentialFormat.equals("E0") || exponentialFormat.equals("E+0"))){
			throw new ConfigurationException("Parameter exponentialFormat has to be set either to \"E0\" or \"E+0\"");
		}

		moveSign = getParameter("moveSign", null);
		if(moveSign != null && (!moveSign.equals("left")) && !(moveSign.equals("right"))){
			throw new ConfigurationException("Parameter moveSign can be set either to \"left\" or \"right\" or \"null\"");
		}

		groupingSeparator = getParameter("groupingSeparator", null);
		if(groupingSeparator!= null && groupingSeparator.length() != 1) throw new ConfigurationException("Only one character is allowed as a grouping separator.");
		decimalSeparator = getParameter("decimalSeparator", null);
		if(decimalSeparator!= null && decimalSeparator.length() != 1) throw new ConfigurationException("Only one character is allowed as a decimal separator.");
	}

	public void validateInputs() throws ConfigurationException {
		checkInputSize(1, true);
	}
}
