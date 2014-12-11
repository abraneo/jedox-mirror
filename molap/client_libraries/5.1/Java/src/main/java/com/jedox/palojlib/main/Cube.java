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

public class Cube extends CachedComponent implements ICube{

	public static final int defaultExpiryDuration = 60;
	
	/* final variables are not part of the cache*/
	private final CubeHandler cubehandler;
	private final int id;
	private final Database database;
	private final String contextId;
	private CubeInfo cubeInfo;
	private String name;
	
	private long cbTokenCache;
	private long ccTokenCache;

	protected Cube(String contextId, int id, String name,Database database, CubeInfo info)  throws PaloException, PaloJException{
		cubehandler = new CubeHandler(contextId);
		this.contextId = contextId;
		this.id = id;
		this.name = name;
		this.database = database;
		this.cubeInfo = info;
		cbTokenCache = this.cubeInfo.getCB_Token();
		ccTokenCache = this.cubeInfo.getCC_Token();
		cacheTrustExpiry = defaultExpiryDuration;
	}

	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}
	
	public CubeInfo getCubeInfo() {
		checkCache();
		return cubeInfo;
	}

	public IDimension[] getDimensions() throws PaloException, PaloJException {

		ArrayList<Dimension> cubeDims = new ArrayList<Dimension>();
		int[] dimensionsIds = getDimensionIds();
		for(int id:dimensionsIds){
			cubeDims.add(database.getDimensionById(id));
		}
		return cubeDims.toArray(new Dimension[cubeDims.size()]);
	}
	
	public IElement[] getCellPath(String[] names) throws PaloException, PaloJException {
		
		int[] dimensionsIds = getDimensionIds();
	
		if(names.length!=dimensionsIds.length)
			throw new PaloJException("The number of given names should match the number of dimensions in the cube.");
		
		IElement[] result = new IElement[names.length];
			
		for(int i=0;i<dimensionsIds.length;i++){
			Dimension dim = database.getDimensionById(dimensionsIds[i]);
			IElement e = dim.getElementByName(names[i], false);
			
			if(e==null)
				throw new PaloJException("Element " + names[i] + " does not exist in dimension "+ dim.getName());
			
			result[i]=e;		
		}
		
		return result;
	}

	/*protected ICell[] extractCells(IElement[][] area,ExportType type,int blockSize,boolean useRules, boolean onlyBases, boolean skipEmpty){
		return cubehandler.extractCells(database.getId(),id,type, blockSize, useRules, onlyBases, skipEmpty, area,this.getDimensions());
	}*/

	public BigInteger getNumberOfFilledCells() throws PaloException{
		//always get them new
		endTrustTime();
		return getCubeInfo().getNumberOfFilledCells();
	}

	public BigInteger getNumberOfCells() throws PaloException{
		//always get them new
		endTrustTime();
		return getCubeInfo().getNumberOfCells();
	}

	public CubeType getType() throws PaloException{
		//always get them new
		endTrustTime();
		return getCubeInfo().getType();
	}
	
	public int[] getDimensionIds() throws PaloException{
		return getCubeInfo().getDimensionIds();
	}

	public void clear() throws PaloException {
		cubehandler.clear(database, name,id,null);
		endTrustTime();
	}

	public void clearCells(IElement[][] area) throws PaloException {
		if(area==null) throw new PaloJException("Parameter area can not be null");
		cubehandler.clear(database, name,id,area);
		endTrustTime();
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
		endTrustTime();
	}

	public void loadCells(IElement[][] paths, Object[] values, ICellLoadContext context, IElement[][] lockedPaths) throws PaloJException, PaloException {
		cubehandler.loadCells(database.getId(),id,paths, values,context.getBlockSize(),context.isAdd(), context.getSplashMode(),context.isEventProcessor(), lockedPaths);
		endTrustTime();
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
		endTrustTime();
	}

	public void updateRule(int id, String definition, boolean activate,String externalIdentifier,String comment) throws PaloException {
		cubehandler.updateRule(database,this,id,definition,activate,externalIdentifier,comment);
		endTrustTime();
	}
	
	public String parseRule(String definition) throws PaloException, PaloJException {
		if (definition == null || definition.trim().isEmpty()) throw new PaloJException("Rule may not be empty.");
		return cubehandler.parseRule(database, this, definition);
	}

	public void removeRules(IRule[] rules) throws PaloException {
		if(rules==null || rules.length==0)
			throw new PaloJException("rules parameter can not be null or empty.");
		cubehandler.removeRules(database,this,rules);
		endTrustTime();
	}
	
	public void removeRules() throws PaloException {
		cubehandler.removeRules(database,this,null);
		endTrustTime();
	}

	public CellsExporter getCellsExporter(IElement[][] area, ICellExportContext context) throws PaloException, PaloJException {
		return new CellsExporter(area, context.getCellsExportType(), context.getBlockSize(), context.isUseRules(), context.isOnlyBases(), context.isSkipEmpty(), cubehandler,this.getDimensions(),database.getId(),id,contextId);
	}
	
	public ICell getCell(IElement[] path) throws PaloException, PaloJException {
		return cubehandler.getCell(database,this,path);
	}
	
	public void rename(String name) throws PaloException{
		cubehandler.rename(database,id,Helpers.urlEncode(name));
		this.name = name;
		endTrustTime();
	}
	
	public long getCBToken() throws PaloException{
		checkCache();
		return cubeInfo.getCB_Token();
	}
	
	public long getCCToken() throws PaloException{
		checkCache();
		return cubeInfo.getCC_Token();
	}
	
	@Override
	public void activateRules(IRule[] rules) throws PaloException {
		if(rules==null || rules.length==0)
			throw new PaloJException("rules parameter can not be null or empty.");
		cubehandler.changeRuleActivation(database,this,rules,true);
		
	}

	@Override
	public void deactivateRules(IRule[] rules) throws PaloException {
		if(rules==null || rules.length==0)
			throw new PaloJException("rules parameter can not be null or empty.");
		cubehandler.changeRuleActivation(database,this,rules,false);	
	}
	
	@Override
	public ICellLoadContext getCellLoadContext(SplashMode mode, int blockSize,
			boolean add, boolean eventProcessor) {
		return new CellLoadContext(mode, blockSize, add, eventProcessor);
	}

	/**
	 * @throws PaloException 
	 * @throws PaloJException ******************************************************************************/
	
	protected boolean checkCCTokenChange(){
		if(ccTokenCache == getCubeInfo().getCC_Token() && ccTokenCache!=-1)
			return true;
		
		return false;
	}
	
	protected boolean checkCBTokenChange(){
		if(cbTokenCache == getCubeInfo().getCB_Token())
			return true;
		
		return false;
	}
	
	public Dimension getDimensionById(int dimId) throws PaloException, PaloJException {
		
		int[] dimensionsIds = getDimensionIds();	
		for(int id:dimensionsIds){
			Dimension dim = database.getDimensionById(dimId);
			if(dim.getId() == id)
				return dim;
		}
		return null;
	}
	
	public void resetCache() {
		cacheTrustExpiry = defaultExpiryDuration;
		endTrustTime();
		cubeInfo = null;
	}
	
	private boolean cacheExists(){
		return cubeInfo != null;
	}
	
	private synchronized void checkCache(){
		if ( !cacheExists() || !inTrustTime()) {
			cubeInfo = cubehandler.getCubeInfo(database.getId(), id);
			setCacheTrustExpiry(cacheTrustExpiry);
		}
	}
}
