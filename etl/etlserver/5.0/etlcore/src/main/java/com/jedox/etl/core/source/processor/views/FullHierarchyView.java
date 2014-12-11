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

import java.util.ArrayList;
import java.util.List;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.INamedValue;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class FullHierarchyView implements IModelView {
	
	private List<List<ITreeElement>> rows;
	private Row row;

	protected Row initRow(IAliasMap map, int length) {
		Row row = new Row();
		for (int i=0; i<length; i++) {
			//CoordinateNode coordinate = new CoordinateNode(map.getAlias(i+1, "level"+i));
			CoordinateNode coordinate = new CoordinateNode("level"+(i+1));
			// coordinate.setFallbackDefault(null);
			row.addColumn(coordinate);
		}
		return row;
	}
	
	protected List<List<ITreeElement>> getRows(ITreeManager manager, ITreeElement n, List<ITreeElement> path, List<List<ITreeElement>> rows) {
		ITreeElement[] children = manager.getElement(n.getName()).getChildren();
		for (ITreeElement c : children) {
			path.add(n);
			getRows(manager,c,path,rows);
			path.remove(n);
		}
		if (children.length == 0) {
			path.add(n);
			List<ITreeElement> arow = new ArrayList<ITreeElement>(path.size());
			arow.addAll(path);
			rows.add(arow);
			path.remove(n);
		}
		return rows;
	}
	
	protected void fillColumn(CoordinateNode target, INamedValue<ElementType> source) {
		target.setValue(source.getName());
		if (source.getValue() != null) target.setElementType(source.getValue().toString());
	}
	
	public Row fillRow(int current) {
		List<ITreeElement> arow = rows.get(current);
		for (int i=0; i<arow.size(); i++) {
			fillColumn((CoordinateNode) row.getColumn(i), arow.get(i));
		}
		for (int i=arow.size(); i<row.size(); i++) {
			CoordinateNode c = (CoordinateNode) row.getColumn(i);
			c.setValue("");
		}
		return row;
	}

	public Row setupRow(ITreeSource source) throws RuntimeException {
		ITreeManager manager = source.generate();
		rows = new ArrayList<List<ITreeElement>>();
		for (ITreeElement c : manager.getRootElements(false)) {
			rows = getRows(manager, c, new ArrayList<ITreeElement>(),rows);
		}
		//rows may have different length due to non-symmetric trees - thus determine row with max length
		int maxLength = 1;
		for (List<ITreeElement> arow : rows) {
			if (arow.size() > maxLength) {
				maxLength = arow.size();
			}
		}
		//create internal row based on row with max length
		row = initRow(source.getAliasMap(),maxLength);
		return row;
	}
	
	public void init() {
		//init has to be done in setupRow since we cannot determine output description otherwise due to asymmetric trees.
	}

	public int size() {
		return rows.size();
	}
	
	protected List<List<ITreeElement>> getSourceRows() {
		return rows;
	}
	
	protected Row getTargetRow() {
		return row;
	}

}
