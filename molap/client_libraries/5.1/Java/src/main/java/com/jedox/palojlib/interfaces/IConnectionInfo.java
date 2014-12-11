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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.palojlib.interfaces;

/**
 * represent the olap server connection information typically returned from server/info
 * @author khaddadin
 *
 */
public interface IConnectionInfo {
	
	/**
	 * ssl encryption types possible in olap server
	 * @author khaddadin
	 *
	 */
	public enum EncryptionType {
		/**
		 * no ssl is used
		 */
		ENCRYPTION_NONE, 
		/**
		 * both ssl and non-ssl is possible 
		 */
		ENCRYPTION_OPTIONAL, 
		/**
		 * only ssl is accepted
		 */
		ENCRYPTION_REQUIRED
	}	
	
	/**
	 * major version
	 * @return major version 
	 */
	public String getMajorVersion();
	
	/**
	 * minor version
	 * @return minor version 
	 */
	public String getMinorVersion();
	
	/**
	 * bug fix version
	 * @return bug fix version 
	 */
	public String getBugfixVersion();
	
	/**
	 * build number
	 * @return build number
	 */
	public String getBuildNumber();
	
	/**
	 * get encryption type used in olap server {@link EncryptionType}
	 * @return encryption type
	 */
	public EncryptionType getEncryptionType();
	
	/**
	 * https port if exists
	 * @return https port if exists
	 */
	public int getHttpsPort();

}
