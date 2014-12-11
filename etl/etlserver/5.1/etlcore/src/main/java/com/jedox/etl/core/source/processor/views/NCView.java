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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class NCView implements IModelView {
	private List<String> types = new ArrayList<String>();
	private List<ITreeElement> elements = new ArrayList<ITreeElement>();
	private ITreeElement lastParent = null;
	private Row row = new Row();
	private ITreeManager manager;
	
	protected CoordinateNode fillType(int currentRow) {
		CoordinateNode type = (CoordinateNode) getTargetRow().getColumn(0);
		//set constant input depending on type
		type.setValue(types.get(currentRow));
		return type;
	}
	
	protected List<ITreeElement> getElements() {
		return elements;
	}
	
	protected Row getTargetRow() {
		return row;
	}
	
	protected ITreeManager getTree() {
		return manager;
	}
	
	protected CoordinateNode fillElement(int currentRow) {
		ITreeElement node = getElements().get(currentRow);
		//get element node as the first column
		CoordinateNode element = (CoordinateNode) getTargetRow().getColumn(1);
		element.setValue(node.getName());
		return element;
	}
	
	protected CoordinateNode fillWeight(int currentRow, String type) {
		ITreeElement node = getElements().get(currentRow);
		CoordinateNode weight = (CoordinateNode) getTargetRow().getColumn(2);
		if (!(type.length() == 0)) { //N Rows have no weight.
			weight.setValue("");
			lastParent = node;
		}
		else {//set weight column as input
			weight.setValue(node.getWeight(lastParent));
		}
		return weight;
	}
	
	public Row fillRow(int current) {
		fillType(current);
		fillElement(current);
		fillWeight(current, types.get(current));
		return getTargetRow();
	}

	public Row setupRow(Row attributeRow) throws RuntimeException {
		CoordinateNode type = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameType(),null);
		CoordinateNode element = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameElement(),null);
		CoordinateNode weight = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameWeight(),null);
		getTargetRow().addColumn(type);
		getTargetRow().addColumn(element);
		getTargetRow().addColumn(weight);
		return getTargetRow();
	}
	
	public void init(ITreeManager manager) throws RuntimeException {
		this.manager = manager;
		for (ITreeElement node : manager.getElements(false)) {
			if (node.getChildCount() == 0) elements.add(node);
		}
		//Add N / S types for the leaf nodes
		for (ITreeElement element : elements) {
			ElementType e = element.getType();
			switch (e) {
			case ELEMENT_NUMERIC: types.add("N"); break;
			case ELEMENT_STRING: types.add("S"); break;
			default: types.add("N"); break;
			}
		}
		//process unique branch nodes in reverse order (so that root comes last)
		Set<ITreeElement> visited = new HashSet<ITreeElement>();
		List<ITreeElement> branches = manager.getDescendants(null,false);
		for (int i = branches.size()-1; i >= 0; i--) {
			ITreeElement t = branches.get(i);
			if (t.getChildCount() > 0 && !visited.contains(t)) {
				visited.add(t);
				elements.add(t);
				//Add a C type for the consolidation definition
				types.add("C");
				//get the consolidated children
				ITreeElement[] children = t.getChildren();
				//Add empty types of children enumeration
				for (int j=0; j<children.length; j++) {
					elements.add(children[j]);
					types.add("");
				}
			}
		}
	}
	
	public int size() {
		return getElements().size();
	}

}
