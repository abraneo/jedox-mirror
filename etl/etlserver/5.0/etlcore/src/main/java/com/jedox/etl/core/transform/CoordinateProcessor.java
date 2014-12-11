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
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right 
*   (commercial copyright) has Jedox AG, Freiburg.
*  
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.transform;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.AnnexNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;

public abstract class CoordinateProcessor extends Processor {
	
	private IProcessor input;
	private Row coordinates;
	private Row infos;
	private Row row = new Row();
	
	//private static final Log log = LogFactory.getLog(PipelineOutputProcessor.class);
	
	public CoordinateProcessor(IProcessor input, ColumnManager columns, boolean includeInfos) {
		//NOTE: We expect the input to be ordered as coordinates, infos, values!!
		this.input = input;
		coordinates = columns.getColumnsOfType(IColumn.ColumnTypes.coordinate);
		infos = new Row();
		for (IColumn source : coordinates.getColumns()) {
			CoordinateNode column = new CoordinateNode(source.getName());
			column.setInput(source);
			row.addColumn(column);
		}
		if (includeInfos) {
			infos = columns.getColumnsOfType(IColumn.ColumnTypes.annex);
			for (IColumn source : infos.getColumns()) {
				CoordinateNode column = new AnnexNode(source.getName());
				column.setInput(source);
				row.addColumn(column);
			}
		}
		if (input != null) this.setName(input.getName());
		this.setSourceProcessor(input);
	}
	
	protected IProcessor getInputProcessor() {
		return input;
	}
	
	protected void removeInfo(String name) {
		infos.removeColumn(name);
		getRow().removeColumn(name);
	}
	
	protected boolean fillCoordinates(Row inputRow) {
		if (inputRow != null) {
			for (int i=0; i<coordinates.size(); i++) {
				CoordinateNode c = (CoordinateNode)getRow().getColumn(i);
				c.setInput(inputRow.getColumn(i));
			}
			for (int i=0; i<infos.size(); i++) {
				//info follow the coordinates, if enabled
				CoordinateNode c = (CoordinateNode)getRow().getColumn(coordinates.size()+i);
				c.setInput(inputRow.getColumn(coordinates.size()+i));
			}
		}
		return inputRow != null;
	}
	
	
	protected boolean fillRow(Row row) throws Exception {
		return fillCoordinates(getInputProcessor().next());
	}
	
	protected int getLength() {
		return coordinates.size() + infos.size();
	}

	protected Row getRow() {
		return row;
	}

}
