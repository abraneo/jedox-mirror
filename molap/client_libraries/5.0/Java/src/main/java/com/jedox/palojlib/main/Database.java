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

import java.util.HashMap;
import java.util.LinkedHashMap;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.util.Helpers;

public class Database implements IDatabase{

	private final DatabaseHandler databasehandler;
	private final int id;
	private final DatabaseType type;

	/*variables needed for the cache*/
	private String name;
	private DatabaseInfo info;
	private LinkedHashMap<Key,Dimension> dimensions = new LinkedHashMap<Key,Dimension>();
	private LinkedHashMap<Key,Cube> cubes = new LinkedHashMap<Key,Cube>();

	protected Database(String contextId, int id, String name, DatabaseType type, String dimensionsNumber, String cubesNumber, String status, String token) throws PaloException, PaloJException{
		databasehandler = new DatabaseHandler(contextId);
		this.id = id;
		this.name = name;
		this.type = type;
		this.info = new DatabaseInfo(cubesNumber, dimensionsNumber, status, "-1");
	}

	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}

	public DatabaseType getType(){
		return this.type;
	}

	public IDimension addDimension(String name) throws PaloException, PaloJException{
		
		databasehandler.addDimension(this,name);
		reinitCache(getServerDatabaseInfo());
		return dimensions.get(new Key(name.toLowerCase(),-1));
	}

	public IDimension[] getDimensions() throws PaloException, PaloJException{
		
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}
		return dimensions.values().toArray(new Dimension[dimensions.size()]);
	}

	public IDimension getDimensionByName(String name) throws PaloException, PaloJException{
		
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}
		return dimensions.get(new Key(name.toLowerCase(),-1));
	}


	public ICube addCube(String name, IDimension [] dimensions) throws PaloException, PaloJException{
		
		if(dimensions.length == 0){
			throw new PaloJException("Cube " + name + "can be created,at least one dimension should be specified.");
		}
		
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}
		
		int[] dimensionIds = new int[dimensions.length];
		for(int i=0;i<dimensions.length;i++){
			Dimension d = this.dimensions.get(new Key(dimensions[i].getName().toLowerCase(),-1));
			if(d == null){
				throw new PaloJException("Cube " + name + "can be created, dimension " + dimensions[i].getName() + " does not exit.");
			}else{
				dimensionIds[i] = d.getId();
			}
		}
		databasehandler.addCube(this,name,dimensionIds);
		reinitCache(getServerDatabaseInfo());
		return cubes.get(new Key(name.toLowerCase(),-1));
	}

	public ICube[] getCubes() throws PaloException, PaloJException{
		
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}

		return cubes.values().toArray(new Cube[cubes.size()]);
	}

	public Cube getCubeByName(String name) throws PaloException, PaloJException{
		ValidationResult result = validateCache();
		if(!result.isUpToDate()){
			reinitCache(result.getServerInfo());
		}
		return cubes.get(new Key(name.toLowerCase(),-1));
	}

	public void removeCube(ICube c) throws PaloException, PaloJException {
		databasehandler.removeCube(id,(Cube)c);
	}

	public void removeDimension(IDimension d) throws PaloException, PaloJException {
		databasehandler.removeDimension(id,(Dimension)d);
	}

	public void save() throws PaloException, PaloJException {
		databasehandler.save(id);		
	}

	public int getId() {
		return id;
	}
	
	public void rename(String name) throws PaloException, PaloJException{
		databasehandler.rename(id,Helpers.urlEncode(Helpers.escapeDoubleQuotes(name)));
		this.name = name;
	}
	
	/**
	 * @throws PaloException 
	 * @throws PaloJException 
	 * @throws NumberFormatException *************************************************************************/

	private void reinitCache(ComponentInfo info) throws PaloException, PaloJException{	
		
		dimensions = databasehandler.getDimensions(this);
		cubes = databasehandler.getCubes(this);
		this.info = (DatabaseInfo)info;
	}

	private ValidationResult validateCache() throws PaloException, PaloJException{
		return new ValidationResult(this.info,getServerDatabaseInfo());
	}
	
	private DatabaseInfo getServerDatabaseInfo() throws PaloException, PaloJException{
		return databasehandler.getDatabaseInfo(id);
	}

	public DatabaseInfo getDatabaseInfo(){
		return getServerDatabaseInfo();
	}


	/**
	 * @throws PaloJException ************************************************************************/

	protected Dimension getDimensionById(int id) throws PaloException, PaloJException{
		if(this.info.token == -1)
			reinitCache(getServerDatabaseInfo());
		return dimensions.get(new Key("",id));
	}

	protected Cube getCubeById(int id) throws PaloException, PaloJException{
		if(this.info.token == -1)
			reinitCache(getServerDatabaseInfo());
		return cubes.get(new Key("",id));
	}

}
