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

import java.util.ArrayList;
import java.util.HashMap;

import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IRule;
import com.jedox.palojlib.interfaces.ICell.CellType;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.interfaces.ICube.SplashMode;
import com.jedox.palojlib.util.Helpers;

public class CubeHandler {

	private final String contextId;
	private static final String GET_CUBE_INFO_REQUEST  = "/cube/info?";
	private static final String CELLS_EXPORT_REQUEST  = "/cell/export?";
	private static final String CELL_VALUE_REQUEST  = "/cell/value?";
	private static final String CELLS_REPLACE_BULK_REQUEST  = "/cell/replace_bulk?";
	private static final String CLEAR_CUBE_REQUEST  = "/cube/clear?";
	private static final String SAVE_CUBE_REQUEST  = "/cube/save?";
	private static final String LOCK_CUBE_REQUEST  = "/cube/lock?";
	private static final String COMMIT_CUBE_REQUEST  = "/cube/commit?";
	private static final String RENAME_CUBE_REQUEST  = "/cube/rename?";
	private static final String CONVERT_CUBE_REQUEST  = "/cube/convert?";
	private static final String GET_CUBE_RULES_REQUEST  ="/cube/rules?";
	private static final String ADD_CUBE_RULE_REQUEST  ="/rule/create?";
	private static final String MODIFY_CUBE_RULE_REQUEST  ="/rule/modify?";
	private static final String DELETE_CUBE_RULE_REQUEST  ="/rule/destroy?";
	private static final String PARSE_CUBE_RULE_REQUEST  ="/rule/parse?";

	protected CubeHandler(String contextId) throws PaloException, PaloJException{
		this.contextId = contextId;
	}

	protected CubeInfo getCubeInfo(int databaseId,int cubeId) throws PaloException{
		StringBuilder GET_CUBE_INFO_REQUEST_BUFFER = new StringBuilder(GET_CUBE_INFO_REQUEST);
		
		StringBuilder currentRequest = GET_CUBE_INFO_REQUEST_BUFFER.append("database=").append(databaseId).append("&cube=").append(cubeId);
		StringBuilder currentRequestBackup = new StringBuilder(currentRequest.toString());
		
		String [][]response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		String [][] onlyToken = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequestBackup,true,true);
		
		String[] dimensionIds = response[0][3].split(",");
		int[] dimensionIdsInt = new int[dimensionIds.length];
		for(int i=0;i<dimensionIds.length;i++){
			dimensionIdsInt[i] = Integer.parseInt(dimensionIds[i]);
		}

		return new CubeInfo(dimensionIdsInt,response[0][4],response[0][5],response[0][7],response[0][8],onlyToken[0][0]);
	}

	protected HashMap<String,Cell> extractCellsMap(int databaseId,int cubeId,CellsExportType type, int blockSize, boolean useRules,
			boolean onlyBases, boolean skipEmpty, IElement[][] area, IDimension[] cubeDims) throws PaloException, PaloJException {

		String 	stringtype = parseTypeToString(type);
		StringBuilder stringareabuffer= fillAreaBuffer(area);

		StringBuilder GET_CELLS_REQUEST_BUFFER = new StringBuilder(CELLS_EXPORT_REQUEST);
		StringBuilder currentRequest = GET_CELLS_REQUEST_BUFFER.append("database=").append(databaseId).append("&cube=").append(cubeId).append("&blocksize=").append(blockSize).append("&type=").append(stringtype).append("&use_rules=").append((useRules?"1":"0")).append("&base_only=").append((onlyBases?"1":"0")).append("&skip_empty=").append((skipEmpty?"1":"0")).append("&area=").append(stringareabuffer.toString());

		Dimension [] dimensions = new Dimension[cubeDims.length];
		
		for(int j=0;j<cubeDims.length;j++){
			dimensions[j] = ((Dimension)cubeDims[j]);
		}

		return extractCellsMapFromPath(currentRequest, null, dimensions);

	}

	protected String parseTypeToString(CellsExportType type) {

		switch(type){
			case BOTH: return "0";
			case ONLY_NUMERIC:return "1";
			case ONLY_STRING:return "2";
		}

		return null;
	}

	private HashMap<String,Cell> extractCellsMapFromPath(StringBuilder request, String path, Dimension[]  cubeDims) throws PaloException, PaloJException {

		if(path == null){
			path = "";
		}else{
			path="&path="+path;
		}

		StringBuilder originalRequest = new StringBuilder(request);
		StringBuilder currentRequest = originalRequest.append(path);
		String [][]response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		HashMap<String,Cell> cellsMap = new HashMap<String,Cell>();

		for(int i=0;i<response.length;i++){
			if(response[i].length>3){

				String [] pathIdsStr = response[i][3].split(",");
				int[] pathIds = new int[pathIdsStr.length];
				
				for(int j=0;j<pathIdsStr.length;j++){
					pathIds[j] = Integer.parseInt(pathIdsStr[j]);
				}			
				Cell cell = new Cell(pathIds,response[i][2],(response[i][0].equals("1")?CellType.CELL_NUMERIC:CellType.CELL_STRING), cubeDims,null);
				cellsMap.put(cell.getPathIdsAsString(),cell);
			}else{
				if(Float.parseFloat(response[i][0])/Float.parseFloat(response[i][1]) >= 1.0){
					return cellsMap;
				}else{
					cellsMap.putAll(extractCellsMapFromPath(request,response[i-1][3],cubeDims));
					return cellsMap;
				}
			}
		}

		throw new PaloJException("The format of the result in the export is not correct.");
	}

	protected void clear(Database database, String name, int id, IElement[][] area) throws PaloException{

		String toAppend = "&complete=1";
		if(area!= null){
			toAppend = "&area=" + fillAreaBuffer(area);
		}

		StringBuilder CLEAR_CUBE_REQUEST_BUFFER = new StringBuilder(CLEAR_CUBE_REQUEST);
		StringBuilder currentRequest = CLEAR_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId()).append(toAppend);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	protected StringBuilder fillAreaBuffer(IElement[][] area) {
		StringBuilder stringareabuffer=new StringBuilder() ;
		for(int i=0;i<area.length;i++){
			IElement[] row = area[i];
			if(row != null){
			for(int j=0;j<row.length;j++){
				stringareabuffer.append(((Element)row[j]).getId()).append(":");
			}
			stringareabuffer.deleteCharAt(stringareabuffer.length()-1);
			}else{
				stringareabuffer.append("*");
			}
			stringareabuffer.append(',');
		}

     return  stringareabuffer.deleteCharAt(stringareabuffer.length()-1);
	}

	protected IRule[] getRules(Database database, int id) throws PaloException{

		StringBuilder GET_CUBE_RULES_REQUEST_BUFFER = new StringBuilder(GET_CUBE_RULES_REQUEST);
		StringBuilder currentRequest = GET_CUBE_RULES_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId());
		String[][] responses = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		if(responses.length == 0)
			return new Rule[]{};
		else{
			ArrayList<IRule> rules= new ArrayList<IRule>();

			for(int i=0;i<responses.length;i++){
				rules.add((IRule) new Rule(Integer.parseInt(responses[i][0]),responses[i][1],responses[i][2],responses[i][3],Long.parseLong(responses[i][4]),(responses[i][5].equals("1")?true:false)));
			}
			return rules.toArray(new Rule[rules.size()]);
		}
	}

	protected void convert(Database database, String name,CubeType type, int id) throws PaloException{

		StringBuilder CONVERT_CUBE_REQUEST_BUFFER = new StringBuilder(CONVERT_CUBE_REQUEST);
		StringBuilder currentRequest = CONVERT_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId()).append("&type=").append(type.ordinal());
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	protected void save(Database database, String name, int id) throws PaloException{

		StringBuilder SAVE_CUBE_REQUEST_BUFFER = new StringBuilder(SAVE_CUBE_REQUEST);
		StringBuilder currentRequest = SAVE_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId());
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	protected void loadCells(int databaseId, int id, IElement[][] paths,Object[] values, int blockSize, boolean isAdd, SplashMode mode,boolean eventprocessor, IElement[][] lockedPaths) throws PaloJException, PaloException {

		int isAddInt = (isAdd?1:0);
		
		int splashModeInt = 1;
		switch(mode){
		case SPLASH_NOSPLASHING:splashModeInt = 0;break;
		case SPLASH_ADD:splashModeInt = 2;break;
		case SPLASH_SET:splashModeInt = 3;break;
		default:
		}

		int number_of_cells = paths.length;
		if(number_of_cells!= values.length)
			throw new PaloJException("Number of paths should be equal to the number of values");
		
		StringBuilder lockedpathsbuffer = new StringBuilder();
		if(lockedPaths!=null && lockedPaths.length>0){
			for(int j=0;j<lockedPaths.length;j++){
				IElement[] lockedCell = lockedPaths[j];
				lockedpathsbuffer.append(fillPathBuffer(lockedCell)).append(":");	
			}
			lockedpathsbuffer.deleteCharAt(lockedpathsbuffer.length()-1);
		}
		
		int i=0;
		for(;i<number_of_cells;){
			loadCellsFromIndex(databaseId, id, paths, values, blockSize, isAddInt, splashModeInt, i, eventprocessor, lockedpathsbuffer);
			i+=blockSize;
		}
		if(i<(number_of_cells-1)){
			loadCellsFromIndex(databaseId, id, paths, values, blockSize, isAddInt, splashModeInt, i, eventprocessor, lockedpathsbuffer);
		}

	}

	private void loadCellsFromIndex(int databaseId, int id, IElement[][] paths,Object[] values, int blockSize, int isAddInt, int splashMode,  int Startindex, boolean eventprocessor, StringBuilder lockedpathsbuffer) throws PaloException {

		StringBuilder CELLS_REPLACE_BULK_REQUEST_BUFFER = new StringBuilder(CELLS_REPLACE_BULK_REQUEST);
		StringBuilder pathsbuffer = new StringBuilder();
		StringBuilder valuebuffer = new StringBuilder();

		int max = Math.min(Startindex+blockSize,paths.length);
		for(int i=Startindex;i<max;i++){
			IElement[] cell = paths[i];
			String value = Helpers.urlEncode(Helpers.addDoubleQuotes(values[i].toString()));
			pathsbuffer.append(fillPathBuffer(cell)).append(":");		
			valuebuffer.append(value).append(":");
		}
		pathsbuffer.deleteCharAt(pathsbuffer.length()-1);
		valuebuffer.deleteCharAt(valuebuffer.length()-1);


		StringBuilder currentRequest = CELLS_REPLACE_BULK_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(databaseId).append("&paths=").append(pathsbuffer).append("&values=").append(valuebuffer).append("&add=").append(isAddInt).append("&splash=").append(splashMode).append("&event_processor=").append((eventprocessor?"1":"0")).append(lockedpathsbuffer.length()>0?"&locked_paths="+lockedpathsbuffer.toString():"");
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	private StringBuilder fillPathBuffer(IElement[] cell) {
		StringBuilder pathsbuffer = new StringBuilder();
		for(IElement e:cell){
			if(e==null)
				throw new PaloJException("Cell path can not contain null values.");
			pathsbuffer.append(((Element)e).getId()).append(",");
		}
		pathsbuffer.deleteCharAt(pathsbuffer.length()-1);
		return pathsbuffer;
	}

	public void addRule(Database database, Cube cube, String definition, boolean activate,String externalIdentifier,String comment) throws PaloException {
		StringBuilder ADD_CUBE_RULE_REQUEST_BUFFER = new StringBuilder(ADD_CUBE_RULE_REQUEST);
		StringBuilder currentRequest = ADD_CUBE_RULE_REQUEST_BUFFER.append("cube=").append(cube.getId()).append("&database=").append(database.getId()).append("&definition=").append(Helpers.urlEncode(definition)).append("&activate=").append((activate==true?"1":"0")).append("&externalIdentifier=").append(externalIdentifier).append("&comment=").append(Helpers.urlEncode(comment));
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	public void updateRule(Database database, Cube cube, int id,String definition, boolean activate, String externalIdentifier,String comment) throws PaloException {

		StringBuilder MODIFY_CUBE_RULE_REQUEST_BUFFER = new StringBuilder(MODIFY_CUBE_RULE_REQUEST);
		StringBuilder currentRequest = MODIFY_CUBE_RULE_REQUEST_BUFFER.append("rule=").append(id).append("&cube=").append(cube.getId()).append("&database=").append(database.getId()).append("&definition=").append(Helpers.urlEncode(definition)).append("&activate=").append((activate==true?"1":"0")).append("&externalIdentifier=").append(externalIdentifier).append("&comment=").append(Helpers.urlEncode(comment));
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);

	}

	public void removeRules(Database database, Cube cube,IRule[] rules) throws PaloException {

		if (useBulkRulesAPI() || (rules!=null && rules.length==1)) {
			StringBuilder ids = getIdsList(rules);				
			StringBuilder DELETE_CUBE_RULE_REQUEST_BUFFER = new StringBuilder(DELETE_CUBE_RULE_REQUEST);
			StringBuilder currentRequest = DELETE_CUBE_RULE_REQUEST_BUFFER.append("cube=").append(cube.getId()).append("&database=").append(database.getId()).append(ids.length()==0?"":"&rule="+ids);
			HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);			
		} else {
			if (rules==null) {
				rules = getRules(database, cube.getId());				
			}
			for (IRule rule : rules) {
				removeRules(database, cube, new IRule[]{rule});
			}			
		}	
	}
	
	public String parseRule(Database database, Cube cube, String definition) throws PaloException {
		StringBuilder PARSE_CUBE_RULE_REQUEST_BUFFER = new StringBuilder(PARSE_CUBE_RULE_REQUEST);
		StringBuilder currentRequest = PARSE_CUBE_RULE_REQUEST_BUFFER.append("&cube=").append(cube.getId()).append("&database=").append(database.getId()).append("&definition=").append(Helpers.urlEncode(definition));
		String[][] response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		return response[0][0];
	}

	private StringBuilder getIdsList(IRule[] rules) {
		StringBuilder ids = new StringBuilder();
		if(rules!=null){
			for(int i=0;i<rules.length;i++){
				ids.append(rules[i].getIdentifier()).append(",");		
			}
			ids.deleteCharAt(ids.length()-1);
		}
		return ids;
	}

	public Cell getCell(Database database, Cube cube, IElement[] path) throws PaloException, PaloJException {
		StringBuilder CELL_VALUE_REQUEST_BUFFER = new StringBuilder(CELL_VALUE_REQUEST);
		StringBuilder pathBuffer =  fillPathBuffer(path);
		int[] pathIds = new int[path.length];
		String[] pathNames = new String[path.length];
		IDimension[] cubeDims = cube.getDimensions();
		Dimension[] dimensions = new Dimension[cubeDims.length];
		for(int j=0;j<cubeDims.length;j++){
			pathIds[j]=((Element)path[j]).getId();
			dimensions[j] = ((Dimension)cubeDims[j]);
			pathNames[j] = path[j].getName();
		}
		StringBuilder currentRequest = CELL_VALUE_REQUEST_BUFFER.append("cube=").append(cube.getId()).append("&database=").append(database.getId()).append("&path=").append(pathBuffer);
		String[][] responses = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
		return new Cell(pathIds,responses[0][2],(responses[0][0].equals("1")?CellType.CELL_NUMERIC:CellType.CELL_STRING), dimensions,pathNames);			
	}

	public void rename(Database database, int id, String name) throws PaloException {
		StringBuilder RENAME_CUBE_REQUEST_BUFFER = new StringBuilder(RENAME_CUBE_REQUEST);
		StringBuilder currentRequest = RENAME_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId()).append("&new_name=").append(name);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);		
	}

	public int lock(Database database, int id) throws PaloException {
		StringBuilder LOCK_CUBE_REQUEST_BUFFER = new StringBuilder(LOCK_CUBE_REQUEST);
		StringBuilder currentRequest = LOCK_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId()).append("&complete=1");
		String[][] response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		return Integer.parseInt(response[0][0]);
	}
	
	public int lock(Database database, int  id, IElement[][] area) throws PaloException {
		StringBuilder LOCK_CUBE_REQUEST_BUFFER = new StringBuilder(LOCK_CUBE_REQUEST);
		StringBuilder currentRequest = LOCK_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId()).append("&area=").append(fillAreaBuffer(area));
		String[][] response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		return Integer.parseInt(response[0][0]);
	}
	
	public void commitLock(Database database, int id,int lockId) throws PaloException {
		StringBuilder COMMIT_CUBE_REQUEST_BUFFER = new StringBuilder(COMMIT_CUBE_REQUEST);
		StringBuilder currentRequest = COMMIT_CUBE_REQUEST_BUFFER.append("cube=").append(id).append("&database=").append(database.getId()).append("&lock=").append(lockId);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
	}

	public void changeRuleActivation(Database database, Cube cube,
			IRule[] rules, boolean activate) {
		
		if(useBulkRulesAPI()){
			StringBuilder ids = getIdsList(rules);
			StringBuilder MODIFY_CUBE_RULE_REQUEST_BUFFER = new StringBuilder(MODIFY_CUBE_RULE_REQUEST);
			StringBuilder currentRequest = MODIFY_CUBE_RULE_REQUEST_BUFFER.append("rule=").append(ids).append("&cube=").append(cube.getId()).append("&database=").append(database.getId()).append("&activate=").append((activate==true?"1":"0"));
			HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);	
		}else{
			for(IRule r:rules){
				if(activate && !r.isActive()) {
					updateRule(database, cube ,r.getIdentifier(), r.getDefinition(), true, r.getExternalIdentifier(),r.getComment());	
				}else if(!activate && r.isActive()){
					updateRule(database, cube ,r.getIdentifier(), r.getDefinition(), false, r.getExternalIdentifier(),r.getComment());	
				}
			}
		}
	}
	
	private boolean useBulkRulesAPI(){
		int major = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).majorVersion;
		int minor = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).minorVersion;
		int buildNumber = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).buildNumber;
		return ( (major>5) ||
				 (( major==5) && (minor>0)) ||
				 (( major==5) && (minor==0) && (buildNumber>4098)) );
		
	}


}
