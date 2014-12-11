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
import java.util.Iterator;

import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICell.CellType;

public class CellsExporter implements ICellsExporter {

	private IDimension[] dimensions;
	private StringBuilder initialRequest;
	private ArrayList<ICell> cells;
	private Iterator<ICell> iter;
	private ICell  lastCell;
	private boolean lastBulkRecieved;
	private final String contextId;
	private static final String CELLS_EXPORT_REQUEST  = "/cell/export?";

	protected CellsExporter(IElement[][] area,CellsExportType type,int blockSize,boolean useRules, boolean onlyBases, boolean skipEmpty,CubeHandler cubehandler,IDimension[] dimensions,int databaseId,int cubeId,String contextId) throws PaloException, PaloJException{

		this.dimensions = dimensions;
		this.contextId = contextId;
		String stringtype = cubehandler.parseTypeToString(type);
		StringBuilder stringareabuffer= cubehandler.fillAreaBuffer(area);

		StringBuilder GET_CELLS_REQUEST_BUFFER = new StringBuilder(CELLS_EXPORT_REQUEST);
		initialRequest = GET_CELLS_REQUEST_BUFFER.append("database=").append(databaseId).append("&cube=").append(cubeId).append("&blocksize=").append(blockSize).append("&type=").append(stringtype).append("&use_rules=").append((useRules?"1":"0")).append("&base_only=").append((onlyBases?"1":"0")).append("&skip_empty=").append((skipEmpty?"1":"0")).append("&area=").append(stringareabuffer.toString());
	}

	public ICell next() throws PaloException{
		
		//only the first time
		if(cells== null){
			cells = extractCellsFromPath(initialRequest, null);
			iter = cells.iterator();
		}

		//if the iterator still delivers a cell
		if(iter.hasNext()){
			lastCell = iter.next();
			return lastCell;
		}
		else if(lastBulkRecieved){
			return null;
		}
		else
		{
			String lastPath =((Cell)lastCell).getPathIdsAsString();
			cells.clear();
			cells = extractCellsFromPath(initialRequest, lastPath);
			if(cells.isEmpty())
				return null;
			iter = cells.iterator();
			return next();
		}
	}

	public void reset(){
		cells = null;
	}

	public boolean hasNext() throws PaloException{
		if(cells== null){
			cells = extractCellsFromPath(initialRequest, null);
			iter = cells.iterator();
		}

		if(!iter.hasNext() && lastBulkRecieved)
			return false;

		return true;
	}

	protected ArrayList<ICell> extractCellsFromPath(StringBuilder request, String path) throws PaloException {

		path = ((path == null)?"":"&path="+path);
		StringBuilder originalRequest = new StringBuilder(request);
		StringBuilder currentRequest = originalRequest.append(path);

		String [][]response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		ArrayList<ICell> cells = new ArrayList<ICell>();

		for(int i=0;i<response.length;i++){
			if(response[i].length>3 && !response[i][3].isEmpty()){

				String [] pathIdsStr = response[i][3].split(",");
				int[] pathIds = new int[pathIdsStr.length];
				
				for(int j=0;j<pathIdsStr.length;j++){
					pathIds[j] = Integer.parseInt(pathIdsStr[j]);
				}
				Cell cell = new Cell(pathIds,response[i][2],(response[i][0].equals("1")?CellType.CELL_NUMERIC:CellType.CELL_STRING), this.dimensions,null);
				cells.add((ICell)cell);
			
			}else{
				if(Float.parseFloat(response[i][0])/Float.parseFloat(response[i][1]) >= 1.0){
					lastBulkRecieved = true;
				}
			}
		}
		return cells;
	}

	public IDimension[] getDimensions() {
		return this.dimensions;
	}


}
