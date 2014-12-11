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
import java.util.HashMap;
import java.util.List;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.util.NamingUtil;

public class LevelElementWeightView implements IModelView {

	protected List<ITreeElement> elements;
	protected List<Integer> levels;
	protected Row row = new Row();
	private int currentLevel;
	protected HashMap<Integer,Double> weightsMap;
	private double defaultWeight = 1;
	private int index = 0;


	protected Row getTargetRow() {
		return row;
	}
	

	public Row fillRow(int current) {
		CoordinateNode level = row.getColumn(0,CoordinateNode.class);
		level.setValue(levels.get(current));
		CoordinateNode element = row.getColumn(1,CoordinateNode.class);
		element.setValue(elements.get(current).getName());
		CoordinateNode weight = row.getColumn(2,CoordinateNode.class);
		Double weightValue = weightsMap.get(current);
		weight.setValue(weightValue!=null?weightValue:defaultWeight);
		return row;
	}
	
	private void addRows(ITreeManager manager) throws RuntimeException {
		elements = new ArrayList<ITreeElement>();
		levels = new ArrayList<Integer>();
		weightsMap = new HashMap<Integer, Double>();
		currentLevel = 1;
		for (ITreeElement c : manager.getRootElements(false)) {
			addRows(manager, c, null);
		}
	}

	private void addRows(ITreeManager manager, ITreeElement child, ITreeElement parent) {

		elements.add(child);

		if(parent!=null){
			double weightValue = child.getWeight(parent);
			if(weightValue!=1)weightsMap.put(index, weightValue);
		}
		
		index++;

		levels.add(currentLevel++);
	
		ITreeElement[] children = manager.getElement(child.getName()).getChildren();
		for (ITreeElement c : children) {
			addRows(manager,c,child);
		}
		currentLevel--;
	}

	public Row setupRow(Row attributeRow) throws RuntimeException {
		currentLevel = 1;
		CoordinateNode level = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameLevel(),null);
		CoordinateNode element = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameElement(),null);
		CoordinateNode weight = ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameWeight(),null);		
		//now build the row
		row.addColumn(level);
		row.addColumn(element);
		row.addColumn(weight);
		//get root nodes
		return row;
	}

	public void init(ITreeManager manager) throws RuntimeException {
		addRows(manager);
		currentLevel = 1;
	}

	@Override
	public int size() {
		return elements.size();
	}

	

}
