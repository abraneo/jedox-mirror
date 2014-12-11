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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.transform;

import java.util.HashSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class DenormalizerProcessor extends CoordinateProcessor {

	private Row values;
	
	private HashSet<String> foundColumns = new HashSet<String>();
	private static final Log log = LogFactory.getLog(DenormalizerProcessor.class);
	
	public DenormalizerProcessor(IProcessor input, ColumnManager columns, boolean includeInfos) {
		super(input, columns, includeInfos);
		this.setSourceProcessor(input);
		values = columns.getColumnsOfType(IColumn.ColumnTypes.value);
		for (IColumn avalue : values.getColumns()) {
			Column c = new Column();
			c.mimic(avalue);
			getRow().addColumn(c);
			ValueNode v = (ValueNode)avalue;
			//remove denormalization coordinate and input coordinate
			removeInfo(NamingUtil.internal(v.getTarget()));
			removeInfo(NamingUtil.internal(v.getInputName()));
		}
	}
	
	protected int getLength() {
		return super.getLength() + values.size();
	}
	
	protected void fillValues(Row inputRow) {
		if (inputRow != null) {
			for (int i=0; i<values.size(); i++) {
				ValueNode v = (ValueNode)values.getColumn(i);
				//note: target and input are only in inputProcessor, not in current processor row, since we removed them an initialization
				IColumn target = getInputProcessor().current().getColumn(NamingUtil.internal(v.getTarget()));
				IColumn value = getRow().getColumn(v.getName());
				if (target.getValue().equals(v.getName())) {
					IColumn input = getInputProcessor().current().getColumn(NamingUtil.internal(v.getInputName()));
					value.setValue(input.getValue());
					foundColumns.add(v.getName());
				} 
				else {
					if (v.getElementType().equals(ElementType.ELEMENT_NUMERIC))
						value.setValue(0);
					else if (v.getElementType().equals(ElementType.ELEMENT_STRING)) {
						value.setValue("");
//						value.setDefaultValue("");
					}	
					else
						value.setValue(null);
				}	
			}
		}
	}
	
	protected boolean fillRow(Row row) throws Exception {
		if (values.size() > 0) {
			boolean hasData = super.fillRow(row);
			//fill value
			if (hasData) {
				fillValues(row);
			}
			return hasData;
		}
		else 
			return super.fillRow(row);
	}
	
	public void close() {
		super.close();
		// Check if all 
		for (int i=0; i<values.size(); i++) {
			ValueNode v = (ValueNode)values.getColumn(i);;
			if (!foundColumns.contains(v.getName())) {
				log.warn("In transform "+getName()+" the measure "+v.getName()+" has not been found in source column "+v.getTarget()+".");
			}		
		}	
	}	

}
