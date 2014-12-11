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
package com.jedox.etl.core.load;

import com.jedox.etl.core.connection.IConnectable;
import com.jedox.etl.core.execution.IExecutable;

/**
 * Interface of all Loads to implement.
 * A Load is responsible for the task of exporting data calculated by the ETLServer to a target system.
 * An Load is executable and needs to have a {@link com.jedox.etl.core.connection.IConnection Connection}.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface ILoad extends IExecutable, IConnectable {

	/**
	 * Specifies the available load modes. Every load has to implement its own handling for these modes.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum Modes {
		/**
		 * Destroys all structures in the target system before exporting them. Useful if the structures defined in ETLServer have changed and do not correspond to the target structures any more.
		 */
		CREATE,
		/**
		 * Adds the structures from ETLServer to the structures in the target system. If the data is numeric it is added to the existing data in the target system. In contrast to CREATE, it leaves structures not existent in ETLServer but existent in the target system untouched.
		 */
		ADD,
		/**
		 * Synchronizes structures in the target system with the structures from ETLServer, so that only data from ETLServer is present in the target system.
		 */
		UPDATE,
		/**
		 * Inserts the structures provided by ETLServer into the structures in the target system. Existing data is replaced.
		 */
		INSERT,
		/**
		 * Deletes the structures provided by ETLServer from the target system.
		 */
		DELETE,
		/**
		 * Simply streams out data as is. Intended for internal use only.
		 */
		FILL,
		/**
		 * Performs like FILL, but creates and destroys all needed structures. Intended for internal use only.
		 */
		TEMPORARY
	}

	/**
	 * Gets the export {@link Modes mode} set for this load.
	 * @return the export mode.
	 */
	public Modes getMode();
}
