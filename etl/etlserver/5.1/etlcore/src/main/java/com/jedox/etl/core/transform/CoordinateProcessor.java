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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.transform;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import java.util.List;

import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.component.RuntimeException;

public abstract class CoordinateProcessor extends Processor {

	private List<CoordinateNode> coordinates;
	private Row row = new Row();
	
	//private static final Log log = LogFactory.getLog(PipelineOutputProcessor.class);
	
	public CoordinateProcessor(IProcessor input, Row columns) {
		//NOTE: We expect the input to be ordered as coordinates, infos, values!!
		coordinates = columns.getColumns(CoordinateNode.class);
		this.setSourceProcessor(input);
	}
	
	protected boolean fillCoordinates(Row inputRow) {
		if (inputRow != null) {
			for (int i=0; i<coordinates.size(); i++) {
				CoordinateNode c = (CoordinateNode)getRow().getColumn(i);
				c.setInput(inputRow.getColumn(i));
			}
		}
		return inputRow != null;
	}
	
	
	protected boolean fillRow(Row row) throws Exception {
		return fillCoordinates(getSourceProcessor().next());
	}
	
	protected int getLength() {
		return coordinates.size();
	}

	protected Row getRow() {
		return row;
	}
	
	protected void init() throws RuntimeException {
		for (IColumn source : coordinates) {
			CoordinateNode column = ColumnNodeFactory.getInstance().createCoordinateNode(source.getName(),source);
			row.addColumn(column);
		}
	}

}
