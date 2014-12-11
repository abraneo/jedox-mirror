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
import com.jedox.palojlib.interfaces.ISvsInfo;


/**
 * @author khaddadin
 *
 */



public class SvsInfo implements ISvsInfo {
	
	private boolean svsActive;
	private LOGIN_MODE loginMode;
	private boolean cubeWorkerActive;
	private boolean enableDrillthrough;
	private boolean enableDimensionWorker;
		
	protected SvsInfo(String svsActive,String loginMode,String cubeWorkerActive,String enableDrillthrough,String enableDimensionWorker){
		this.svsActive = (svsActive.equals("0")?false:true);
		
		switch(Integer.parseInt(loginMode)){
		case 0:
			this.loginMode = LOGIN_MODE.NONE;
			break;
		case 1:
			this.loginMode = LOGIN_MODE.INFO;
			break;
		case 2:
			this.loginMode = LOGIN_MODE.AUTHENTIFICATION;
			break;
		case 3:
			this.loginMode = LOGIN_MODE.AUTHORIZATION;
			break;
		default:
				throw new PaloJException("LOGIN_MODE can be " + loginMode);
		}
		this.cubeWorkerActive = (cubeWorkerActive.equals("0")?false:true);
		this.enableDrillthrough = (enableDrillthrough.equals("0")?false:true);
		this.enableDimensionWorker = (enableDimensionWorker.equals("0")?false:true);
	}
	
	public boolean isSvsActive() {
		return svsActive;
	}

	public LOGIN_MODE getLoginMode() {
		return loginMode;
	}

	public boolean isCubeWorkerActive() {
		return cubeWorkerActive;
	}

	public boolean isEnableDrillthrough() {
		return enableDrillthrough;
	}

	public boolean isEnableDimensionWorker() {
		return enableDimensionWorker;
	}

}
