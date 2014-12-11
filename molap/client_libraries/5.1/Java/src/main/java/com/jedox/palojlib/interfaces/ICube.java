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

import java.math.BigInteger;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;

/**
 * represent a cube in database
 * @author khaddadin
 *
 */
public interface ICube {

	/**
	 * olap cell export type
	 * @author khaddadin
	 *
	 */
	public static enum CellsExportType{
		/**
		 * only numeric cells
		 */
		ONLY_NUMERIC,
		/**
		 * only string cells
		 */
		ONLY_STRING,
		/**
		 * all cells
		 */
		BOTH
	}

	/**
	 * olap cube type
	 * @author khaddadin
	 *
	 */
	public static enum CubeType{
		/**
		 * normal
		 */
		CUBE_NORMAL, 
		/**
		 * system
		 */
		CUBE_SYSTEM,
		/**
		 * attribute
		 */
		CUBE_ATTRIBUTE,
		/**
		 * userinfo
		 */
		CUBE_USERINFO, 
		/**
		 * gpu
		 */
		CUBE_GPU
	}

	/**
	 * olap cube splash mode
	 * @author khaddadin
	 *
	 */
	public static enum SplashMode{
		/**
		 * no splashing is allowed
		 */
		SPLASH_NOSPLASHING, 
		/**
		 * default splash mode
		 */
		SPLASH_DEFAULT,
		/**
		 * add splash
		 */
		SPLASH_ADD,
		/**
		 * set splash
		 */
		SPLASH_SET
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
	 * Get the cube type {@link CubeType}
	 * @return cube type 
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
	 * Get an exporter to use later to export cells from an olap cube
	 * @param area Area contained from the cartesian product of the elements mentioned in each dimension. The dimensions in the array has the same order as in the cube. So, Element[0] refers to elements in the first dimension of the cube. If no elements are given for a certain dimension, then all elements will be used.
	 * @param context of the export {@link ICellExportContext}
	 * @return exporter to get the cells in blocks and only when needed {@link ICellsExporter}.
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICellsExporter getCellsExporter(IElement[][] area,ICellExportContext context) throws PaloException, PaloJException;

	/**
	 * Write cells back in palo server.
	 * @param paths each path specify cell coordinates.
	 * @param values each value is a cell value
	 * @param context load context {@link ICellLoadContext}
	 * @param lockedPaths paths where Splashing will not change them and their sources areas if they are consolidated.
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void loadCells(IElement[][] paths,Object[] values,ICellLoadContext context, IElement[][] lockedPaths) throws PaloException, PaloJException;
	
	/**
	 * get the value of a single cell typically using cell/value
	 * @param path path to certain cell
	 * @return cube cell with its value {@link ICell}
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public ICell getCell(IElement[] path) throws PaloException, PaloJException;
	
	/**
	 * empty cells in an area within a cube
	 * @param area Area contained from the cartesian product of the elements mentioned in each dimension. The dimensions in the array has the same order as in the cube. So, Element[0] refers to elements in the first dimension of the cube. If no elements are given for a certain dimension, then all elements will be used.
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
	 * remove a set of rules, the rules will be removed depending on their identifiers.
	 * @param rules rules to be removed
	 * @throws PaloException
	 */
	public void removeRules(IRule[] rules) throws PaloException;

	/**
	 * activate a set of rules, the rules will be changed depending on their identifiers.
	 * @param rules rules to be activated
	 * @throws PaloException
	 */
	public void activateRules(IRule[] rules) throws PaloException;

	/**
	 * deactivate a set of rules, the rules will be changed depending on their identifiers.
	 * @param rules rules to be deactivated
	 * @throws PaloException
	 */
	public void deactivateRules(IRule[] rules) throws PaloException;

	
	/**
	 * update an existing rule
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
	 * parse a definition of the rule
	 * @param definition
	 * @return XML representation of the enterprise rule.
	 * @throws PaloException
	 * @throws PaloJException
	 */
	public String parseRule(String definition) throws PaloException, PaloJException;

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
	 * clears completely the data in the cube
	 * @throws PaloException
	 */
	public  void clear() throws PaloException;
	
	/**
	 * lock the data in the cube, no reading is possible.
	 * @return id of the locked cube area, needed to do the commit later
	 * @throws PaloException
	 */
	public int lockComplete() throws PaloException;
	
	/**
	 * lock only an area in the cube, reading is still possible.
	 * @return id of the locked area, needed to do the commit later
	 * @throws PaloException
	 */
	public int lockArea(IElement[][] area) throws PaloException;
	
	/**
	 * commit/free a lock in the cube
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
	
	/**
	 * get the elements object that points to a cell path within the cube
	 * @param names the element names
	 * @return element objects
	 */
	public IElement[] getCellPath(String[] names);
	
	/**
	 * get a cell load context which is needed when writing cell values
	 * @param mode {@link ICellLoadContext#getSplashMode()}
	 * @param blockSize {@link ICellLoadContext#getBlockSize()}
	 * @param add {@link ICellLoadContext#isAdd()}
	 * @param eventProcessor {@link ICellLoadContext#isEventProcessor()}
	 * @return
	 */
	public ICellLoadContext getCellLoadContext(SplashMode mode,int blockSize,boolean add, boolean eventProcessor);

}
