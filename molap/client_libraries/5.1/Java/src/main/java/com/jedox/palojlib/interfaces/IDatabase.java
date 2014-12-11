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

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;

/**
 * represent a database in olap server
 * @author khaddadin
 *
 */
public interface IDatabase {


	/**
	 * database types in olap server
	 * @author khaddadin
	 *
	 */
	public static enum DatabaseType{
		/**
		 * normal
		 */
		DATABASE_NORMAL, 
		/**
		 * system e.g. System database
		 */
		DATABASE_SYSTEM, 
		/**
		 * userinfo e.g. config,fgrp1
		 */
		DATABASE_USERINFO
	}

	/**
	 * Get the id of the database
	 * @return the id
	 */
	public int getId();


	/**
	 * Get the name of the database
	 * @return the name of the database
	 */
	public String getName();
	
	/**
	 * database info {@link IDatabaseInfo}
	 * @return database info
	 */
	public IDatabaseInfo getDatabaseInfo();

	/**
	 * Get the type of the database {@link DatabaseType}
	 * @return type of the database 
	 */
	public DatabaseType getType();


	/**
	 * Get the list of the dimensions in the database including all types of dimensions
	 * @return list of the dimensions
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IDimension[] getDimensions() throws PaloException, PaloJException;

	/**
	 * Get the list of the cubes  in the database including all types of cubes: normal,attribute,system, userinfo and gpu.
	 * @return list of the cubes
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICube[] getCubes() throws PaloException, PaloJException;
	
	/**
	 * Get the list of the cubes which include the given dimension.
	 * @param dim dimension
	 * @return list of the cubes
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICube[] getCubes(IDimension dim) throws PaloException, PaloJException;


	/**
	 * Create a new dimension with type normal
	 * @param name name of the new dimension
	 * @return the newly created dimension
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IDimension addDimension(String name) throws PaloException, PaloJException;

	/**
	 * Create a new cube with type normal
	 * @param name name of the new cube
	 * @param dimensions list of dimensions
	 * @return the newly created cube
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICube addCube(String name, IDimension []  dimensions) throws PaloException, PaloJException;


	/**
	 * Get a certain dimension by it is name, return null if does not exist.
	 * @param name name of the needed dimension
	 * @return dimension
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IDimension getDimensionByName(String name) throws PaloException, PaloJException;

	/**
	 * Get a certain cube by it is name, return null if does not exist.
	 * @param name name of the needed cube
	 * @return cube
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICube getCubeByName(String name) throws PaloException, PaloJException;

	/**
	 * remove a certain cube, the cube will be moved depending on its id.
	 * @param cube cube to be removed
	 * @throws PaloJException 
	 */
	public void removeCube(ICube cube) throws PaloException, PaloJException;

	/**
	 * remove a certain dimension, the dimension will be moved depending on its id.
	 * @param dimension dimension to be removed
	 * @throws PaloJException 
	 */
	public void removeDimension(IDimension dimension) throws PaloException, PaloJException;
	
	/**
	 * save the database to disk
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void save() throws PaloException, PaloJException;
	
	/**
	 * rename a database
	 * @param newname new database name
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void rename(String newname) throws PaloException, PaloJException;
	
	/**
	 * set the default cache trust time for the components
	 * @param databaseExpiry time in ms in which database cache will be trusted
	 * @param cubeExpiry time in ms in which cube cache will be trusted
	 * @param dimensionExpiry time in ms in which dimension cache will be trusted
	 */
	public void setCacheTrustExpiries(int databaseExpiry, int cubeExpiry, int dimensionExpiry);
	
	/**
	 * clear any cached information if exists, the expiry trust time will not be affected
	 */
	public void resetCaches();

}
