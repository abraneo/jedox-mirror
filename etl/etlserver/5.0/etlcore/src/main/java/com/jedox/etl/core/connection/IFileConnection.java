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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.connection;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.IWriter;

/**
 * Interface for file based connections to implement
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IFileConnection extends IRelationalConnection {

	/**
	 * gets the writer to write data to the file based backend
	 * @param filename the name of the file to write to
	 * @param append true if data should be appended, false if a new file is to be created
	 * @return
	 */
	public IWriter getWriter(boolean append) throws RuntimeException;

	/**
	 * sets the connection writable (true) or readable (false). Default is readable.
	 * @param isWriteable the writable status (true/false)
	 */
	public void setWritable(boolean isWriteable);
	
	/**
	 * sets a parameter from external (such as columns, skip, end, start)
	 * @param key
	 * @param value
	 */
	public void setExternalParameter(String key, String value);

}
