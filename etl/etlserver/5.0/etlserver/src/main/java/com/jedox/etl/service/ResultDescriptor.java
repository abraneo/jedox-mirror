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
package com.jedox.etl.service;

/**
 * SOAP Transport class for simple Server API calls.
 * Read the result, if call was successful.
 * Read the error message, if call was not successful.
 * @author chris
 *
 */
public class ResultDescriptor {
	
	private boolean valid = true;
	private String errorMessage;
	private String result;
	
	public ResultDescriptor() {
	}
	
	/**
	 * determines, if the result is valid. The result is valid, if no error occurred, while it was calculated.
	 * @return true if valid, false otherwise
	 */
	public boolean getValid() {
		return valid;
	}
	
	/**
	 * sets the validity of the result
	 * @param valid
	 */
	public void setValid(boolean valid) {
		this.valid = valid;
	}
	/**
	 * gets the error message of this result. The message describes the reason why the result is not valid.
	 * @return the error message
	 */
	public String getErrorMessage() {
		return errorMessage;
	}
	/**
	 * sets an error message describing why the result is not valid. Setting an error message invalidates this result.
	 * @param errorMessage
	 */
	public void setErrorMessage(String errorMessage) {
		setValid(false);
		this.errorMessage = errorMessage;
	}
	/**
	 * gets the result data. What it contain is generic and depends on the API-Method where it is used.
	 * @return the result data.
	 */
	public String getResult() {
		return result;
	}
	/**
	 * sets the result data.
	 * @param result
	 */
	public void setResult(String result) {
		this.result = result;
	}
	
	

}
