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

import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class NormalizerProcessor extends CoordinateProcessor {
	
	private int valueSelect = 0;
	private Column value;
	private Row values;
	
	private boolean ignoreEmpty = false;
	
	public NormalizerProcessor(IProcessor input, ColumnManager columns, boolean includeInfos) {
		super(input,columns,includeInfos);
		ignoreEmpty=includeInfos;
		this.setSourceProcessor(input);
		values = columns.getColumnsOfType(IColumn.ColumnTypes.value);
		if (values.size() > 0) {
			//determine name of value column
			value = new Column(values.getName());
			//determine data type of value column. Is of type double only, if all values are numeric
			String datatype = Double.class.getCanonicalName();
			for (IColumn avalue : values.getColumns()) {
				if (!((ValueNode)avalue).getElementType().equals(ElementType.ELEMENT_NUMERIC)) {
					datatype = String.class.getCanonicalName();
					value.setElementType(ElementType.ELEMENT_STRING.toString());
					break;
				}
			}
			value.setValueType(datatype);
			value.setColumnType(IColumn.ColumnTypes.value);
			getRow().addColumn(value);
		}
	}
	
	protected int getLength() {
		return super.getLength() + 1;
	}
	
	protected boolean fillValues(Row inputRow) {
		if (inputRow != null) {
			ValueNode vn = (ValueNode) values.getColumn(valueSelect);
			//get value from input
			Object v = getInputProcessor().current().getColumn(super.getLength()+valueSelect).getValue();
			value.setValue(v);
			//write the value's name to target to provide the index.
			if (vn.getTarget() != null) {
				IColumn target = getRow().getColumn(vn.getTarget());
				target.setValue(vn.getName());
				target.setElementType(vn.getElementType().toString());
			}
			valueSelect = (valueSelect + 1) % values.size();
			boolean x = !(value.isEmpty() || value.getValueAsString().equals("0"));
			return x;
		}
		return true;
	}
	
	protected boolean fillRow(Row row) throws Exception {
		if (values.size() > 0) {
			boolean hasData = true;
			if (valueSelect == 0) {
				hasData = super.fillRow(row);
			}
			if (!hasData)
				return false;
			//fill value
			
			if (ignoreEmpty) {
				do  {
					boolean foundNonEmpty=false;
					do {
						foundNonEmpty = fillValues(row);
					} while (valueSelect != 0 && !foundNonEmpty);
					if (foundNonEmpty)
						return true;
					hasData = super.fillRow(row);				
				} while (hasData);
				return false;
			}
			else {
				fillValues(row);
				return true;
			}
		
		}
		else 
			return super.fillRow(row);
	}

}
