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
package com.jedox.palojlib.main;

import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IDatabaseInfo;

/**
 * @author khaddadin
 *
 */

public class DatabaseInfo extends ComponentInfo implements IDatabaseInfo{
	
	private final int dimensionsNumber;
	private final int cubesNumber;
	private final DatabaseStatus status;
	
	public DatabaseInfo(String cubesNum,String dimsNum,String status,String token){
		this.dimensionsNumber = Integer.parseInt(dimsNum);
		this.cubesNumber = Integer.parseInt(cubesNum);
		switch(Integer.parseInt(status)){
		case 0:
			this.status = DatabaseStatus.UNLOADED;
			break;
		case 1:
			this.status = DatabaseStatus.LOADED;
			break;
		case 2:
			this.status = DatabaseStatus.CHANGED;
			break;
			default:
				throw new PaloJException("Database status can be " + status);
		}
		
		super.token = Integer.parseInt(token);
	}

	/**
	 * @return the cubesNumber
	 */
	public int getCubesNumber() {
		return cubesNumber;
	}

	/**
	 * @return the dimensionsNumber
	 */
	public int getDimensionsNumber() {
		return dimensionsNumber;
	}

	/**
	 * @return the status
	 */
	public DatabaseStatus getStatus() {
		return status;
	}
	

}
