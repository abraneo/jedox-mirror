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
 *   @author Christian Schwarzinger, proclos OG, Wien, Austria
 *   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.etl.core.source.processor;

import java.util.Iterator;
import java.util.List;
import org.jdom.Element;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;

public class ConstantTableProcessor extends Processor {

	private Element data;
	private Row row;
	private Iterator<Element> iterator = null;
	private int count = 0;
	private int numberOfColumns = 0;
	private String[] columnsNames = null;

	public ConstantTableProcessor(Element data, int size) throws RuntimeException {
		this.setLastRow(size);
		this.data=data;
	}


	@SuppressWarnings("unchecked")
	protected void init() throws RuntimeException{
		if (data==null)
			throw new RuntimeException("No data found");
		// Set Column Headers
		Element header=data.getChild("header");
		if (header==null)
			throw new RuntimeException("No header found");			
		List<Element> columnValues = header.getChildren("value");
		numberOfColumns = columnValues.size();
		columnsNames = new String[numberOfColumns];	
		for (int i=0; i<numberOfColumns; i++) {
			columnsNames[i] = columnValues.get(i).getTextTrim();
		}
		row = PersistenceUtil.getColumnDefinition(columnsNames);
		//Initialize Rows Iterator
		iterator  =  data.getChildren("row").iterator();
	}
	
	

	@SuppressWarnings("unchecked")
	protected boolean fillRow(Row row) throws Exception {
		if (iterator.hasNext()) {
			count++;
			Element dataRow = iterator.next();
			if (dataRow != null) {
				try {
					List<Element> columnValues = dataRow.getChildren("value");
					for (int i=0; i<numberOfColumns; i++) {
						if (i<columnValues.size())
							row.getColumn(i).setValue(columnValues.get(i).getTextTrim());
						else
							row.getColumn(i).setValue("");
					}
				}
				catch (Exception e) {
					throw new RuntimeException ("Error in reading row: "+count+" "+e.getMessage());
				}
			}
			return true;
		}
		else { //finished ... do some cleanup
			iterator = null;
			count = 0;
			return false;
		}
	}

	protected Row getRow() {
		return row;
	}

}
