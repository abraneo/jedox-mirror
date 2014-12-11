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

import org.apache.axis2.AxisFault;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.connection.IRelationalConnection;

public class MetadataUtil {
	
	private static Log log = LogFactory.getLog(MetadataUtil.class);
	
	/**
	 * gets the database MataData from an existing (olap) connection.
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param mask a binary 1-bit filter criterion on the types of databases. "0" the exclude system database, "1" to include system databases
	 * @return a {@link ResultDescriptor}. Result contains database information from an OLAP-System as csv. ("Id"; "Name"; "isSystemDatabase")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapDatabases(String locator, String mask) {
		log.info("getting OLAP databases from " + locator);
		ResultDescriptor d = new ResultDescriptor();
		try {
			IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(locator),null);
			if (component instanceof IOLAPConnection) {
				d.setResult(((IOLAPConnection)component).getDatabases(mask).toString());
			}
			else
				d.setErrorMessage(locator +" does not address an OLAPConnection.");
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to get OLAP-Databases for "+locator+": "+e.getMessage());
		}
		log.info("finished");
		return d;
	}

	/**
	 * get the cube MetaData from an existing (olap) connection
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param mask a binary 4-bit filter criterion on types of cubes. null for no filter. 1st bit = AttributeCubes, 2nd bit = SubSetCubes, 3rd bit = SystemCubes, 4th bit = ViewCube. Turn the bits on ("1") to include and off ("0") to exclude.
	 * @return a {@link ResultDescriptor}. Result contains cube information from an OLAP-System as csv. ("Id"; "Name"; "isAttributeCube"; "isSubSetCube"; "isSystemCube"; "isViewCube")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapCubes(String locator, String mask) {
		log.info("getting OLAP cubes from " + locator);
		ResultDescriptor d = new ResultDescriptor();
		try {
			IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(locator),null);
			if (component instanceof IOLAPConnection) {
				d.setResult(((IOLAPConnection)component).getCubes(null,null,mask).toString());
			}
			else
				d.setErrorMessage(locator +" does not address an OLAPConnection.");
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to get OLAP-Cubes for "+locator+": "+e.getMessage());
		}
		log.info("finished");
		return d;
	}

	/**
	 * get the dimension MetaData from an existing (olap) connection
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param database the name of the database on the olap server holding the dimensions
	 * @param mask abinary 3-bit filter criterion on types of dimensions. null for no filter. 1st = AttributeDimensions, 2nd bit = SubSetDimensions, 3rd bit = isSystemDimension. Turn the bits on ("1") to include and off ("0") to exclude.
	 * @return a {@link ResultDescriptor}. Result contains dimension information from an OLAP-System as csv. ("Id"; "Name";"maxDepth"; "maxLevel"; "isAttributeDimension"; "isSubSetDimensio"; "isSystemDimension")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapDimensions(String locator, String mask) {
		log.info("getting OLAP dimensions from " + locator);
		ResultDescriptor d = new ResultDescriptor();
		try {
			IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(locator),null);
			if (component instanceof IOLAPConnection) {
				d.setResult(((IOLAPConnection)component).getDimensions(null, mask).toString());
			}
			else
				d.setErrorMessage(locator +" does not address an OLAPConnection.");
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to get OLAP-Dimensions for "+locator+": "+e.getMessage());
		}
		log.info("finished");
		return d;
	}

	/**
	 * get the dimension MetaData of a given cube from an existing (olap) connection
	 * @param locator the path to the connection. e.g: myProject.connections.myConnection
	 * @param cube the name of the cube within the database
	 * @param mask abinary 3-bit filter criterion on types of dimensions. null for no filter. 1st = AttributeDimensions, 2nd bit = SubSetDimensions, 3rd bit = isSystemDimension. Turn the bits on ("1") to include and off ("0") to exclude.
	 * @return a {@link ResultDescriptor}. Result contains dimension information from an OLAP-System as csv. ("Id"; "Name";"maxDepth"; "maxLevel"; "isAttributeDimension"; "isSubSetDimensio"; "isSystemDimension")
	 * @throws AxisFault
	 * @deprecated
	 */
	public ResultDescriptor getOlapCubeDimensions(String locator, String cube, String mask) {
		log.info("getting OLAP cube dimensions from " + locator);
		ResultDescriptor d = new ResultDescriptor();
		try {
			IComponent component = ConfigManager.getInstance().getComponent(Locator.parse(locator),null);
			if (component instanceof IOLAPConnection) {
				d.setResult(((IOLAPConnection)component).getCubeDimensions(null, cube, mask).toString());
			}
			else
				d.setErrorMessage(locator +" does not address an OLAPConnection.");
		}
		catch (Exception e) {
			d.setErrorMessage("Failed to get OLAP-Dimensions for "+locator+": "+e.getMessage());
		}
		log.info("finished");
		return d;
	}

}
