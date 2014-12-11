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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source.processor;

import java.util.List;
import java.util.HashMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

public class UnionProcessor extends Processor {

	private List<IProcessor> processors;
	private IProcessor processor = null;
	private int index = 0;
	private Row row;
	private HashMap<Integer,String> nameMap = new HashMap<Integer,String>();
	private static Log log = LogFactory.getLog(UnionProcessor.class);
	
	public static IProcessor getInstance(List<IProcessor> processors) throws RuntimeException {
		if (processors.size() == 0)
			throw new RuntimeException("Nothing to be processed.");
		if (processors.size() == 1)
			return processors.get(0);
		return new UnionProcessor(processors);
	}
	
	protected UnionProcessor (List<IProcessor> processors) throws RuntimeException {
		this.processors = processors;
	}

	protected boolean fillRow(Row targetRow) throws Exception {
		Row sourceRow = processor.next();
		while (sourceRow == null) {
			index++;
			if (index < processors.size()) {
				processor = processors.get(index);
				//setSourceProcessor(processor);
				sourceRow = processor.next();
			}
			else
				return false;
		}
		for (int i=0; i<targetRow.size(); i++) {
			CoordinateNode targetColumn = (CoordinateNode) targetRow.getColumn(i);
			IColumn sourceColumn = sourceRow.getColumn(nameMap.get(i));
			targetColumn.setInput(sourceColumn);
		}
		return true;
	}

	protected Row getRow() {
		return row;
	}

	@Override
	protected void init() throws RuntimeException {
		processor = processors.get(0);
		//init the internal row with the uniun of all column
		row = new Row();
		//setSourceProcessor(processor);
		for (int i=0; i<processors.size(); i++) {
			IProcessor p = processors.get(i);
			for (IColumn source : p.current().getColumns()) {
				IColumn target = row.getColumn(source.getName());
				if (target == null) { //column is not yet present in internal row
					CoordinateNode cn = ColumnNodeFactory.getInstance().createCoordinateNode(source.getName(),source);
					row.addColumn(cn);
					nameMap.put(row.indexOf(cn), cn.getName());
				}
				else {//Column is present in internal row.
					if (!target.getValueType().equals(source.getValueType()))
						log.warn("Column "+source.getName()+" of source "+p.getName()+" is of data type "+source.getValueType()+". Type "+target.getValueType()+" was set for this union by a previous source.");
				}
			}
		}
		//fix initialization time calculation for multiple multiple processors 
		for(int i = 0; i < processors.size(); i++)
		{
			this.addProcessingTime(processors.get(i).getOverallProcessingTime());
		}
	}

	
	public long getOwnProcessingTime() {
		long sourceProcessing = 0;
		for (IProcessor p : processors) {
			sourceProcessing += p.getOverallProcessingTime();
		}
		return super.getOverallProcessingTime() - sourceProcessing;
	}
	
	public void close() {
		super.close();
		for (IProcessor p : processors) {
			if (!p.isClosed()) p.close();
		}
	}

}
