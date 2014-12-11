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

import com.jedox.palojlib.interfaces.ICube.CellsExportType;

/**
 * represent a cell export context object used with {@link ICellsExporter}  to read data from cubes
 * @author khaddadin
 *
 */
public interface ICellExportContext {
	

	/**
	 * get the type of the export {@link CellsExportType}.
	 * @return type of the export
	 */
	public CellsExportType getCellsExportType();

	/**
	 * get the maximum number of cells returned for each server call.
	 * @return blocksize
	 */
	public int getBlockSize();

	/**
	 * get whether the export should contain only base cells.
	 * true means only numeric/string base cells and string consolidated cells (everything in olap csv files), false means all cells.
	 * @return onlyBases
	 */
	public boolean isOnlyBases();

	/**
	 * get whether rule-based cells should be exported.
	 * true means rule based cells should be exported, otherwise false
	 * @return useRules
	 */
	public boolean isUseRules();

	/**
	 * get whether empty cells should be exported.
	 * true means empty cells are not exported e.g. zero for numeric and "" for string cells, false means all cells.
	 * @return skipEmpty
	 */
	public boolean isSkipEmpty();

}
