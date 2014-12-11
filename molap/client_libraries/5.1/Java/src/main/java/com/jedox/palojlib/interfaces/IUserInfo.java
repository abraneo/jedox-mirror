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
 *   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
 */
package com.jedox.palojlib.interfaces;

import java.util.Map;

import com.jedox.palojlib.interfaces.IElement.ElementPermission;

/**
 * represent the user information typically result from server/user_info
 * @author khaddadin
 * @author afroehlich
 *
 */
public interface IUserInfo {
	
	/**
	 * right objects defined in olap server
	 * @author khaddadin
	 *
	 */
	/* The sequence here is very important*/
	public static enum Predefined_OLAP_Right_Object_Type{

		/* not used until now */
		/**
		 * user
		 */
		user,
		/**
		 * password
		 */
		password,
		/**
		 * group
		 */
		group,
		
		/* These will be needed for ETL */
		/**
		 * database
		 */
		database,
		/**
		 * cube
		 */
		cube,
		/**
		 * dimension
		 */
		dimension,
		/**
		 * dimension element
		 */
		dimension_element,
		/**
		 * cell data
		 */
		cell_data,
		
		/* not used until now */
		/**
		 * rights
		 */
		rights,
		/**
		 * system operation
		 */
		system_operations,
		/**
		 * event processor to svs
		 */
		event_processor,
		/**
		 * subset view
		 */
		sub_set_view,
		/**
		 * user info
		 */
		user_info,
		/**
		 * rule
		 */
		rule,
		/**
		 * report manager
		 */
		ste_reports,
		/**
		 * file manager
		 */
		ste_files,
		/**
		 * palo
		 */
		ste_palo,
		/**
		 * users
		 */
		ste_users,
		
		/* These will be needed for ETL */
		/**
		 * etl
		 */
		ste_etl,
		
		/* not used until now */
		/**
		 * connection manager
		 */
		ste_conns,
		
		/* These will be needed for ETL */
		/**
		 * drillthrough
		 */
		drillthrough,
		
		/* not used until now */
		/**
		 * scheduler manager
		 */
		ste_scheduler,
		/**
		 * logs
		 */
		ste_logs,
		/**
		 * licenses
		 */
		ste_licenses,
		/**
		 * mobile
		 */
		ste_mobile,
		/**
		 * analyzer
		 */
		ste_analyzer,
		/**
		 * sessions
		 */
		ste_sessions,
		/**
		 * settings
		 */
		ste_settings;
		
		/**
		 * get the type string as used in olap server
		 * @param type
		 * @return olap right object string
		 */
		public static Predefined_OLAP_Right_Object_Type customValueOf(String type){
			type = type.replace(' ', '_').replace("-", "_");
			return valueOf(type);	
		}	
	}
	
	
	
	/**
	 * @return the id
	 */
	public int getId();
	/**
	 * @return the name
	 */
	public String getName();
	/**
	 * @return the groupNames
	 */
	public String[] getGroupNames();
	/**
	 * @return the groupids
	 */
	public int[] getGroupids();
	/**
	 * get a map that maps all right objects to their permission {@link ElementPermission}
	 * @return the rightObjects map
	 */
	public Map<Predefined_OLAP_Right_Object_Type, ElementPermission> getRightObjectsMap();

}
