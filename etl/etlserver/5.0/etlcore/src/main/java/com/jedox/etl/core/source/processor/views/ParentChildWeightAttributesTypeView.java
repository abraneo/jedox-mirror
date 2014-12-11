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
package com.jedox.etl.core.source.processor.views;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.INamedValue;
import com.jedox.etl.core.node.NamedValue;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class ParentChildWeightAttributesTypeView extends ParentChildWeightAttributesView {
	
	private ITreeSource source;
	private static final Log log = LogFactory.getLog(ParentChildWeightAttributesTypeView.class);
	
	protected CoordinateNode getTypeCoordinate() {
		CoordinateNode c = new CoordinateNode(NamingUtil.getElementnameNodeType());
		return c;
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
	
	public Row setupRow(ITreeSource source) throws RuntimeException {
		this.source = source;
		//get a parent child row
		Row row = super.setupRow(source);
		//add the type coordinate
		row.addColumn(getTypeCoordinate());
		return row;
	}
	
	public Row fillRow(int current){
		Row row = super.fillRow(current);
		ITreeElement node = source.getTreeManager().getElement(getTargetRow().getColumn(1).getValueAsString());
		fillColumn((CoordinateNode) getTargetRow().getColumn(NamingUtil.getElementnameNodeType()), getTypeColumn(node, node.getType()));
		return row;
	}

}
