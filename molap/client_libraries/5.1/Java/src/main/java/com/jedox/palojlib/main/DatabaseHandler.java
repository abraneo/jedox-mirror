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

import java.util.LinkedHashMap;
import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension.DimensionType;
import com.jedox.palojlib.util.Helpers;

/**
 * handler used by {@link Database} to make request on olap server
 * @author khaddadin
 *
 */
public final class DatabaseHandler{

	private final String contextId;
	private static final String DATABASE_INFO_REQUEST = "/database/info?";
	private static final String DATABASE_SAVE_REQUEST = "/database/save?";
	private static final String DATABASE_RENAME_REQUEST = "/database/rename?";
	private static final String CREATE_DIMENSION_REQUEST = "/dimension/create?";
	private static final String CREATE_CUBE_REQUEST = "/cube/create?";
	private static final String GET_DIMENSIONS_REQUEST  = "/database/dimensions?";
	private static final String GET_CUBES_REQUEST  = "/database/cubes?";
	private static final String DELETE_CUBE_REQUEST = "/cube/destroy?";
	private static final String DELETE_DIMENSION_REQUEST = "/dimension/destroy?";

	protected DatabaseHandler(String contextId) throws PaloException, PaloJException{

		this.contextId = contextId;
	}

	protected DatabaseInfo getDatabaseInfo(int id) throws PaloException, PaloJException{

		StringBuilder LIST_DATABASE_REQUEST_BUFFER = new StringBuilder(DATABASE_INFO_REQUEST);
		StringBuilder currentRequest = LIST_DATABASE_REQUEST_BUFFER.append("database=").append(id);
		String [][]response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		return new DatabaseInfo(response[0][3],response[0][2],response[0][4],response[0][6]);
	}

	protected void addDimension(Database database, String name) throws PaloException, PaloJException{

		StringBuilder CREATE_DIMENSION_REQUEST_BUFFER = new StringBuilder(CREATE_DIMENSION_REQUEST);
		StringBuilder currentRequest = CREATE_DIMENSION_REQUEST_BUFFER.append("new_name=").append(Helpers.urlEncode(name)).append("&database=").append(database.getId()).append("&type=0");
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	protected LinkedHashMap<Key, Dimension> getDimensions(Database database) throws PaloException, PaloJException{

		StringBuilder GET_DIMENSIONS_REQUEST_BUFFER = new StringBuilder(GET_DIMENSIONS_REQUEST);
		StringBuilder currentRequest = GET_DIMENSIONS_REQUEST_BUFFER.append("database=").append(database.getId()).append("&show_system=1&show_normal=1&show_attribute=1&show_info=1");
		String[][] responses = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		LinkedHashMap<Key,Dimension> dimensions = new LinkedHashMap<Key,Dimension>();
		for(int i=0;i<responses.length;i++){
			DimensionType type = DimensionType.DIMENSION_NORMAL;
			if(responses[i][6].equals("2")){
				type = DimensionType.DIMENSION_ATTRIBUTE;
			}else if(responses[i][6].equals("1")){
				type = DimensionType.DIMENSION_SYSTEM;
			}else if(responses[i][6].equals("3")){
				type = DimensionType.DIMENSION_USERINFO;
			}else if(responses[i][6].equals("4")){
				type = DimensionType.DIMENSION_SYSTEM_ID;
			}else{}
			if(responses[i][7].equals("")) responses[i][7]="-1";
			if(responses[i][8].equals("")) responses[i][8]="-1";
			dimensions.put(new Key(responses[i][1].toLowerCase(),Integer.parseInt(responses[i][0])),new Dimension(contextId, Integer.parseInt(responses[i][0]),responses[i][1],database,type,responses[i][7],responses[i][8],responses[i][3],responses[i][4],responses[i][5],responses[i][10],responses[i][2]));
		}
		return dimensions;
	}

	protected void addCube(Database database, String name, int[] dimensionIds) throws PaloException, PaloJException{

		String dimensions = "";
		for(int id:dimensionIds)
			dimensions = dimensions.concat(id + ",");
		dimensions = dimensions.substring(0, dimensions.length()-1);
		StringBuilder CREATE_CUBE_REQUEST_BUFFER = new StringBuilder(CREATE_CUBE_REQUEST);
		StringBuilder currentRequest = CREATE_CUBE_REQUEST_BUFFER.append("new_name=").append(Helpers.urlEncode(name)).append("&database=").append(database.getId()).append("&type=0").append("&dimensions=").append(dimensions);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}


	protected LinkedHashMap<Key, Cube> getCubes(IDatabase database) throws PaloException, PaloJException{

		StringBuilder GET_CUBES_REQUEST_BUFFER = new StringBuilder(GET_CUBES_REQUEST);
		StringBuilder currentRequest = GET_CUBES_REQUEST_BUFFER.append("database=").append(database.getId()).append("&show_system=1&show_normal=1&show_attribute=1&show_info=1&show_gputype=1");
		String[][] responses = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		LinkedHashMap<Key,Cube> cubes = new LinkedHashMap<Key,Cube>();
		Database databaseObj = (Database)database;

		for(int i=0;i<responses.length;i++){

			/*String[] dimensionIds = responses[i][3].split(",");
			int[] dimensionIdsInt = new int[dimensionIds.length];
			for(int j=0;j<dimensionIds.length;j++){
				dimensionIdsInt[j] = Integer.parseInt(dimensionIds[j]);
			}
			CubeType type = CubeType.CUBE_NORMAL;
			if(responses[i][7].equals("1"))
				type = CubeType.CUBE_SYSTEM;
			else if(responses[i][7].equals("2"))
				type = CubeType.CUBE_ATTRIBUTE;
			else if(responses[i][7].equals("3"))
				type = CubeType.CUBE_USERINFO;
			else if(responses[i][7].equals("4"))
				type = CubeType.CUBE_GPU;
			else{}*/
			String[] dimensionIds = responses[i][3].split(",");
			int[] dimensionIdsInt = new int[dimensionIds.length];
			for(int j=0;j<dimensionIds.length;j++){
				dimensionIdsInt[j] = Integer.parseInt(dimensionIds[j]);
			}
			
			//the client token will always be -1 
			CubeInfo info = new CubeInfo(dimensionIdsInt,responses[i][4],responses[i][5],responses[i][7],responses[i][8],"-1");
			cubes.put(new Key(responses[i][1].toLowerCase(),Integer.parseInt(responses[i][0])),new Cube(contextId, Integer.parseInt(responses[i][0]),responses[i][1],databaseObj, info));
		}
		return cubes;
	}

	protected  void removeCube(int databaseId,Cube c) throws PaloException, PaloJException {
		StringBuilder DELETE_CUBE_REQUEST_BUFFER = new StringBuilder(DELETE_CUBE_REQUEST);
		StringBuilder currentRequest = DELETE_CUBE_REQUEST_BUFFER.append("database=").append(databaseId).append("&cube=").append(((Cube)c).getId());
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	protected void removeDimension(int databaseId,Dimension d) throws PaloException, PaloJException {
		StringBuilder DELETE_DIMENSION_REQUEST_BUFFER = new StringBuilder(DELETE_DIMENSION_REQUEST);
		StringBuilder currentRequest = DELETE_DIMENSION_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(((Dimension)d).getId());
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);

	}

	public void save(int databaseId) throws PaloException, PaloJException {
		StringBuilder DATABASE_SAVE_REQUEST_BUFFER = new StringBuilder(DATABASE_SAVE_REQUEST);
		StringBuilder currentRequest = DATABASE_SAVE_REQUEST_BUFFER.append("database=").append(databaseId);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
	}

	public void rename(int id, String name) throws PaloException, PaloJException {
		StringBuilder RENAME_DATABASE_REQUEST_BUFFER = new StringBuilder(DATABASE_RENAME_REQUEST);
		StringBuilder currentRequest = RENAME_DATABASE_REQUEST_BUFFER.append("&database=").append(id).append("&new_name=").append(name);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
	}

}
