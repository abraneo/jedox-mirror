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

import java.math.BigInteger;
import java.util.ArrayList;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellExportContext;
import com.jedox.palojlib.interfaces.ICellLoadContext;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IRule;
import com.jedox.palojlib.util.Helpers;

public class Cube implements ICube{

	/* final variables are not part of the cache*/
	private final CubeHandler cubehandler;
	private final int id;
	private final Database database;
	private final String contextId;
	
	private String name;
	private long CB_token;
	private long CC_token;

	protected Cube(String contextId, int id, String name,Database database) throws PaloException, PaloJException{
		cubehandler = new CubeHandler(contextId);
		this.contextId = contextId;
		this.id = id;
		this.name = name;
		this.database = database;
		//refreshTokens();//because we need the 2 tokens CB and CC
		//this.token = Integer.parseInt(token);
	}

	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}

	public IDimension[] getDimensions() throws PaloException, PaloJException {

		ArrayList<Dimension> cubeDims = new ArrayList<Dimension>();
		int[] dimensionsIds = getDimensionIds();
		for(int id:dimensionsIds){
			cubeDims.add(database.getDimensionById(id));
		}
		return cubeDims.toArray(new Dimension[cubeDims.size()]);
	}

	/*protected ICell[] extractCells(IElement[][] area,ExportType type,int blockSize,boolean useRules, boolean onlyBases, boolean skipEmpty){
		return cubehandler.extractCells(database.getId(),id,type, blockSize, useRules, onlyBases, skipEmpty, area,this.getDimensions());
	}*/

	public BigInteger getNumberOfFilledCells() throws PaloException{
		return cubehandler.getCubeInfo(database.getId(), id).getNumberOfFilledCells();
	}

	public BigInteger getNumberOfCells() throws PaloException{
		return cubehandler.getCubeInfo(database.getId(), id).getNumberOfCells();
	}

	public CubeType getType() throws PaloException{
		return cubehandler.getCubeInfo(database.getId(), id).getType();
	}
	
	public int[] getDimensionIds() throws PaloException{
		return cubehandler.getCubeInfo(database.getId(), id).getDimensionIds();
	}

	public void clear() throws PaloException {
		cubehandler.clear(database, name,id,null);
	}

	public void clearCells(IElement[][] area) throws PaloException {
		if(area==null) throw new PaloJException("Parameter area can not be null");
		cubehandler.clear(database, name,id,area);
	}

	public void save() throws PaloException {
		cubehandler.save(database, name,id);
	}
	
	public int lockComplete() throws PaloException {
		return cubehandler.lock(database, id);
	}
	
	public int lockArea(IElement[][] area) throws PaloException {
		return cubehandler.lock(database, id,area);
	}

	public void commitLock(int lockId) throws PaloException {
		cubehandler.commitLock(database, id,lockId);		
	}

	public void convert(CubeType cubetype) throws PaloJException, PaloException{
		CubeType type = this.getType();
		if(type.equals(CubeType.CUBE_ATTRIBUTE) || type.equals(CubeType.CUBE_USERINFO) || type.equals(CubeType.CUBE_SYSTEM))
			throw new PaloJException("Cube " + name + " has not the correct type, convert can only be applied to cube of type normal or gpu.");

		if((type.equals(CubeType.CUBE_NORMAL) && cubetype.equals(CubeType.CUBE_NORMAL)) || (type.equals(CubeType.CUBE_GPU) && cubetype.equals(CubeType.CUBE_GPU)))
			throw new PaloJException("Cube " + name + " can not be converted to the same type.");

		cubehandler.convert(database, name, cubetype,id);
	}

	public void loadCells(IElement[][] paths, Object[] values, ICellLoadContext context) throws PaloJException, PaloException {
		cubehandler.loadCells(database.getId(),id,paths, values,context.getBlockSize(),context.isAdd(), context.getSplashMode(),context.isEventProcessor());
	}

	public IRule[] getRules() throws PaloException {
		return cubehandler.getRules(database, id);
	}

	public IDimension getDimensionByName(String name) throws PaloException, PaloJException{
		int[] dimensionsIds = getDimensionIds();
		
		for(int id:dimensionsIds){
			Dimension dim = database.getDimensionById(id);
			if(dim.getName().toLowerCase().equals(name.toLowerCase()))
				return dim;
		}
		return null;
	}

	public int getId(){
		return id;
	}

	public void addRule(String definition, boolean activate, String externalIdentifier,String comment) throws PaloException {
		cubehandler.addRule(database,this,definition,activate,externalIdentifier,comment);
	}

	public void updateRule(int id, String definition, boolean activate,String externalIdentifier,String comment) throws PaloException {
		cubehandler.updateRule(database,this,id,definition,activate,externalIdentifier,comment);

	}

	public void removeRules(IRule[] rules) throws PaloException {	
		cubehandler.removeRules(database,this,rules);
	}
	
	@Override
	public void removeRules() throws PaloException {
		cubehandler.removeRules(database,this,null);
	}

	public CellsExporter getCellsExporter(IElement[][] area, ICellExportContext context) throws PaloException, PaloJException {
		return new CellsExporter(area, context.getCellsExportType(), context.getBlockSize(), context.isUseRules(), context.isOnlyBases(), context.isSkipEmpty(), cubehandler,this.getDimensions(),database.getId(),id,contextId);
	}
	
	public ICell getCell(IElement[] path) throws PaloException, PaloJException {
		return cubehandler.getCell(database,this,path);
	}
	
	public void rename(String name) throws PaloException{
		cubehandler.rename(database,id,Helpers.urlEncode(Helpers.escapeDoubleQuotes(name)));
		this.name = name;
	}
	
	public long getCBToken() throws PaloException{
		refreshTokens();
		return this.CB_token;
	}
	
	public long getCCToken() throws PaloException{
		refreshTokens();
		return this.CC_token;
	}
	
	@Override
	public void activateRules(IRule[] rules) throws PaloException {
		cubehandler.changeRuleActivation(database,this,rules,true);
		
	}

	@Override
	public void deactivateRules(IRule[] rules) throws PaloException {
		cubehandler.changeRuleActivation(database,this,rules,false);	
	}

	/**
	 * @throws PaloException 
	 * @throws PaloJException ******************************************************************************/

	public Dimension getDimensionById(int dimId) throws PaloException, PaloJException {
		
		int[] dimensionsIds = getDimensionIds();	
		for(int id:dimensionsIds){
			Dimension dim = database.getDimensionById(dimId);
			if(dim.getId() == id)
				return dim;
		}
		return null;
	}
	
	protected void refreshTokens() throws PaloException{
		CubeInfo info = cubehandler.getCubeInfo(database.getId(), id);
		this.CC_token = info.getCC_Token();
		this.CB_token = info.getCB_Token();
	}
	
	protected long getCurrentCBToken(){
		return this.CB_token;
	}
	
	protected long getCurrentCCToken(){
		return this.CC_token;
	} 
	
	protected String getCompoundToken(){
		return String.valueOf(this.CB_token) + "-" + String.valueOf(this.CC_token);
	}
}
