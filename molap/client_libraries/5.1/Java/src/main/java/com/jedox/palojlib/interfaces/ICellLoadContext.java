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

import com.jedox.palojlib.interfaces.ICube.SplashMode;

/**
 * represent a cell Load context object used with in {@link ICube#loadCells}  to write data to cubes
 * @author khaddadin
 *
 */
public interface ICellLoadContext {
	 
	/**
	 * get the splash mode of the load {@link SplashMode}.
	 * @return splash mode
	 */
	public SplashMode getSplashMode();

	/**
	 * get the maximum number of cells written in each server call.
	 * It should be a positive number.
	 * @return blockSize
	 */
	public int getBlockSize();

	/**
	 * get whether the load should be aggregated with the existing values. 
	 * Only relevant with numeric cells.
	 * @return isAdd
	 */
	public boolean isAdd();

	/**
	 * get whether the load should fire a supervision event call.
	 * Only relevant when supervision server is installed. 
	 * @return eventprocessor
	 */
	public boolean isEventProcessor();

}
