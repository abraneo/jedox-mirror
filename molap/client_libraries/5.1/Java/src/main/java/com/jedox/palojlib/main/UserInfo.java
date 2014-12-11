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

import java.util.HashMap;

import com.jedox.palojlib.interfaces.IElement.ElementPermission;
import com.jedox.palojlib.interfaces.IUserInfo;

/**
 * @author khaddadin
 *
 */
public class UserInfo implements IUserInfo {
		
	private final int id;
	private final String name;
	private final int[] groupIds;
	private final String[] groupNames;
	private HashMap<Predefined_OLAP_Right_Object_Type,ElementPermission> rightObjectsMap;
	
	public UserInfo(String id,String name,String[] groupIds,String[] groupNames,String rightObjectsStr){
		this.id = Integer.parseInt(id);
		this.name = name;
		this.groupIds = new int[groupIds.length];
		this.groupNames = new String[groupNames.length];
		
		if(groupIds.length!=groupNames.length){
			throw new RuntimeException("The number of group ids is not equal to group names.");
		}
		
		for(int i=0;i<groupIds.length;i++){
			this.groupIds[i] = Integer.parseInt(groupIds[i]);
			this.groupNames[i] = groupNames[i];
		}
		
		rightObjectsMap = null;
		if(rightObjectsStr!=null && !rightObjectsStr.isEmpty()){
			rightObjectsMap = new HashMap<Predefined_OLAP_Right_Object_Type, ElementPermission>();
			String[] rightsObjectSet = rightObjectsStr.split(",");
			for(int i=0;i<rightsObjectSet.length;i++){
				String[] pair = rightsObjectSet[i].split(":");
				Predefined_OLAP_Right_Object_Type type = Predefined_OLAP_Right_Object_Type.customValueOf(pair[0].replaceAll("\"", ""));
				ElementPermission permission = ElementPermission.valueOf(pair[1].replaceAll("\"", ""));
				rightObjectsMap.put(type,permission);
			}	
		}
	}
	
	/**
	 * @return the id
	 */
	public int getId() {
		return id;
	}
	/**
	 * @return the name
	 */
	public String getName() {
		return name;
	}
	/**
	 * @return the groupNames
	 */
	public String[] getGroupNames() {
		return groupNames;
	}
	/**
	 * @return the groupids
	 */
	public int[] getGroupids() {
		return groupIds;
	}

	/**
	 * @return the rightObjects
	 */
	public HashMap<Predefined_OLAP_Right_Object_Type, ElementPermission> getRightObjectsMap() {
		return rightObjectsMap;
	}
	

}
