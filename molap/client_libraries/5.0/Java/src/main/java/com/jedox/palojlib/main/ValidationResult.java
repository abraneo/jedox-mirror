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
package com.jedox.palojlib.main;

/**
 * @author khaddadin
 *
 */
public class ValidationResult {
	
	private final ComponentInfo currentInfo;
	private final ComponentInfo serverInfo;
	
	public ValidationResult(ComponentInfo current, ComponentInfo server){
		this.currentInfo = current;
		this.serverInfo = server;
	}

	/**
	 * @return the serverInfo
	 */
	public ComponentInfo getServerInfo() {
		return serverInfo;
	}
	
	/**
	 * validate the Info
	 * @return whether the server token up-to-date
	 */
	public boolean isUpToDate(){
		
		if(this.currentInfo.getToken() == this.serverInfo.getToken()){
			return true;
		}
		return false;
	}

}
