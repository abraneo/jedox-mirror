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
package com.jedox.etl.core.source.processor.views;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.INamedValue;
import com.jedox.etl.core.node.NamedValue;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class LevelElementWeightAttributesView extends LevelElementWeightView {

	private static final Log log = LogFactory.getLog(LevelElementWeightAttributesView.class);

	private AttributesUtil attributes;
	

	protected Row getTargetRow() {
		return row;
	}
	
	private String reduceTypeString(ElementType type){
		switch(type){
		case ELEMENT_CONSOLIDATED: return "C";
		case ELEMENT_NUMERIC: return "N";
		case ELEMENT_STRING: return "S";
		default: log.error("Element type " + String.valueOf(type)  + " can not be mapped.");
		}
		return null;
	}
	
	protected INamedValue<ElementType> getTypeColumn(ITreeElement node, ElementType type) {
		NamedValue<ElementType> column = new NamedValue<ElementType>(reduceTypeString(type));
		return column;
	}
		
	
	public Row fillRow(int current) {
		super.fillRow(current);
		CoordinateNode type = row.getColumn(3,CoordinateNode.class);
		type.setValue(reduceTypeString(elements.get(current).getType()));		
		for(int i=4;i<row.size();i++){
			row.getColumn(i).setValue(elements.get(current).getAttributeValue(row.getColumn(i).getName()));
		}

		return row;
	}

	public Row setupRow(Row attributeRow) throws RuntimeException {
		super.setupRow(attributeRow);
		CoordinateNode type = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameNodeType(),null);
		row.addColumn(type);				
		attributes = new AttributesUtil(attributeRow);
		row.addColumns(attributes.getAttributes());
		return row;
	}

	@Override
	public int size() {
		return elements.size();
	}

	

}
