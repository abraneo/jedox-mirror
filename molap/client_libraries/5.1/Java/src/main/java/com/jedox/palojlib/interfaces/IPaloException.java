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
 *   You may obtain a copy of the License at
*
 *   If you are developing and distributing open source applications under the
 *   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 *   ISVs, and VARs who distribute Palo with their products, and do not license
 *   and distribute their source code under the GPL, Jedox provides a flexible
 *   OEM Commercial License.
 *
 *	Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.interfaces;

/**
 * represent the exception that comes from olap server directly
 * @author khaddadin
 *
 */

public interface IPaloException {
	
	/**
	 * get error code in OLAP server(e.g. 3  for "invalid license")
	 * @return error code
	 */
	public String getCode();

	/**
	 * get the short error message as received from OLAP server
	 * @return short error message
	 */
	public String getMessage();

	/**
	 * get the long error message as received from OLAP server
	 * @return long error message
	 */
	public String getDescription();

	/**
	 * get the error reason message as received from OLAP server (Optional)
	 * @return error reason message if doesn't exist, description is returned.
	 */
	public String getReason();

}
