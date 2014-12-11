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
package com.jedox.etl.core.connection;

import java.io.StringWriter;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.palojlib.interfaces.IDatabase;

/**
 * Interface for OLAP based Connections to implement
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IOLAPConnection extends IConnection {

	//public org.palo.api.Connection open() throws RuntimeException;
	/**
	 * gets the OLAP Databases in csv format
	 * @param mask a three bit binary mask to include ("1") / exclude ("0") certain types of databases. Bit 1: NormalDatabases, Bit 2: SystemDatabases, Bit3: UserInfoDatabases
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getDatabases(String mask) throws RuntimeException;
	/**
	 * gets the OLAP-Cubes of a given database in csv format
	 * @param database the name of the database
	 * @param mask a five bit binary mask to include ("1") / exclude ("0") certain types of cubes. Bit 1: NormalCubes, Bit 2: AttributeCubes, Bit 3: UserInfoCubes, Bit4: SystemCubes, Bit 5: GpuCubes
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getCubes(String database, String dimension, String mask) throws RuntimeException;
	/**
	 * gets the OLAP-Dimensions of a given database in csv format
	 * @param database the name of the database
	 * @param mask a 4 bit binary mask to include ("1") / exclude ("0") certain types of Dimensions. Bit 1: NormalDimensions, Bit 2: AttributeDimensions, Bit 3: SystemDimensions, Bit4: UserInfoDimensions
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getDimensions(String database, String mask) throws RuntimeException;
	/**
	 * gets the OLAP-Dimensions used in a given cube of a given database in csv format
	 * @param database the name of the database
	 * @param cube thename of the cube
	 * @param mask a 4 bit binary mask to include ("1") / exclude ("0") certain types of Dimensions. Bit 1: NormalDimensions, Bit 2: AttributeDimensions, Bit 3: SystemDimensions, Bit4: UserInfoDimensions
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getCubeDimensions(String database, String cube, String mask) throws RuntimeException;

	public boolean isOlderVersion(int Major, int Minor, int BuildNumber);

	/**
	 * get the database object 
	 * @param createIfNotExist if true database will be created if it does not exists
	 * @param throwExceptionIfNotExists if true, and exception will be thrown if no database exists, make since only when createIfNotExist=false
	 * @return the database object or null if database does not exist and createIfNotExist=false and throwExceptionIfNotExists=false
	 * @throws RuntimeException
	 */
	public IDatabase getDatabase(boolean createIfNotExist, boolean throwExceptionIfNotExists) throws RuntimeException;
}
