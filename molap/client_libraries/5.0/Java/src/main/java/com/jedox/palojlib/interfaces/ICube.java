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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.palojlib.interfaces;

import java.math.BigInteger;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;

/**
 * Interface that represents a cube in database
 * @author khaddadin
 *
 */
public interface ICube {

	public static enum CellsExportType{
		ONLY_NUMERIC,ONLY_STRING,BOTH
	}

	public static enum CubeType{
		CUBE_NORMAL, CUBE_SYSTEM, CUBE_ATTRIBUTE,CUBE_USERINFO, CUBE_GPU
	}

	public static enum SplashMode{
		SPLASH_NOSPLASHING, SPLASH_DEFAULT, SPLASH_ADD,SPLASH_SET
	}

    /**
     * Get the name of the cube.
     * @return name of the cube
     */
	public String getName();

	/**
	 * Get the ID of the Cube
	 * @return Cube ID
	 */
	public int getId();

	/**
	 * Get the cube type. Normal, system, attribute, userinfo or gpu cube.
	 * @return cube type {@link CubeType}
	 * @throws PaloException 
	 */
	public CubeType getType() throws PaloException;

	/**
	 * Get the number of cells, filled and empty ones.
	 * @return overall number of possible cells in the cube
	 * @throws PaloException
	 */
	public BigInteger getNumberOfCells() throws PaloException;

	/**
	 * Get the number of filled cells in the cube.
	 * @return number of filled cells.
	 * @throws PaloException
	 */
	public BigInteger getNumberOfFilledCells() throws PaloException;


	/**
	 * Get the list of dimensions in this cube.
	 * @return list of dimensions
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IDimension[] getDimensions() throws PaloException, PaloJException;

	/**
	 * Get a specific dimension in this cube
	 * @param name name of the dimension
	 * @return dimension
	 * @throws PaloException 
	 * @throws JpaloException
	 */
	public IDimension getDimensionByName(String name) throws PaloJException, PaloException;

	/**
	 * Get an exporter to use later to export cells from a palo cube
	 * @param area Area contained from the cartesian product of the elements mentioned in each dimension. The array has the same order as the dimension in the cube. So, Element[0] refers to elements in the first dimension of the cube. If no elements are given for a certain dimension, then all elements will be used.
	 * @param context of the export
	 * @return exporter to get the cells in blocks and only when needed.
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICellsExporter getCellsExporter(IElement[][] area,ICellExportContext context) throws PaloException, PaloJException;

	/**
	 * Write cells back in palo server.
	 * @param paths each path specify cell coordinates.
	 * @param values each value is a cell value
	 * @param context load context
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void loadCells(IElement[][] paths,Object[] values,ICellLoadContext context) throws PaloException, PaloJException;
	
	/**
	 * get the value of a single cell using palo call cell/value
	 * @param path path to certain cell
	 * @return cube cell with its value {@link ICell}
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICell getCell(IElement[] path) throws PaloException, PaloJException;
	
	/**
	 * empty cells in palo server.
	 * @param area Area contained from the cartesian product of the elements mentioned in each dimension. The array has the same order as the dimension in the cube. So, Element[0] refers to elements in the first dimension of the cube. If no elements are given for a certain dimension, then all elements will be used.
	 * @throws PaloException
	 */

	public void clearCells(IElement[][] area) throws PaloException;

	/**
	 * add a rule to cube.
	 * @param definition definition of the rule
	 * @param activate true when rule should be active, false otherwise.
	 * @param externalIdentifier optional external ID
	 * @param comment optional comment
	 * @throws PaloException
	 */
	public void addRule(String definition,boolean activate,String externalIdentifier,String comment) throws PaloException;

	/**
	 * remove all rules.
	 * @throws PaloException
	 */
	public void removeRules() throws PaloException;
	
	/**
	 * remove a set of rules, the rules will be moved depending on their ids.
	 * @param rule rule to be removed
	 * @throws PaloException
	 */
	public void removeRules(IRule[] rules) throws PaloException;

	/**
	 * activate a set of rules, the rules will be changed depending on their ids.
	 * @param rule rule to be removed
	 * @throws PaloException
	 */
	public void activateRules(IRule[] rules) throws PaloException;

	/**
	 * deactivate a set of rules, the rules will be changed depending on their ids.
	 * @param rule rule to be removed
	 * @throws PaloException
	 */
	public void deactivateRules(IRule[] rules) throws PaloException;

	
	/**
	 * update a rule
	 * @param id id of the rule.
	 * @param definition definition of the rule
	 * @param activate true, if the rule is active, false otherwise.
	 * @param externalIdentifier optional external ID
	 * @param comment optional comment
	 * @throws PaloException
	 */
	public void updateRule(int id,String definition,boolean activate,String externalIdentifier,String comment) throws PaloException;

	/**
	 * get the list of the rules for this cube
	 * @return rules {@link IRule}
	 * @throws PaloException 
	 */
	public IRule[] getRules() throws PaloException ;

	/**
	 * convert cube type from normal to gpu or other way around
	 * @param type {@link CubeType}
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void convert(CubeType type) throws PaloException, PaloJException;

	/**
	 * save the cube to the csv files
	 * @throws PaloException
	 */
	public void save() throws PaloException;


	/**
	 * clears completly the data in the cube
	 * @throws PaloException
	 */
	public  void clear() throws PaloException;
	
	/**
	 * lock completly the data in the cube, no reading is possible.
	 * @return id of the locked cube area, needed to do the commit later
	 * @throws PaloException
	 */
	public int lockComplete() throws PaloException;
	
	/**
	 * lock only an area in the cube, reading is possible.
	 * @return id of the locked area, needed to do the commit later
	 * @throws PaloException
	 */
	public int lockArea(IElement[][] area) throws PaloException;
	
	/**
	 * clears completely the data in the cube
	 * @param lockId id of the locked area in the cube
	 * @throws PaloException
	 */
	public  void commitLock(int lockId) throws PaloException;
	
	/**
	 * rename a cube
	 * @param newname new cube name
	 * @throws PaloException
	 */
	public void rename(String newname) throws PaloException;
	
	/**
	 * get current Palo_CB Token
	 * @throws PaloException 
	 */
	public long getCBToken() throws PaloException;
	
	/**
	 * get current Palo_CC Token
	 * @throws PaloException 
	 */
	public long getCCToken() throws PaloException;

}
