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

import java.util.Arrays;
import java.util.Iterator;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeColumn;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;

public class TreeManagerProcessor extends Processor implements ITreeProcessor {
	
	private Row row = new Row();
	private int currentElement = 0;
	private ITreeManager manager;
	private IDimension dimension;
	private Iterator<? extends IElement> elementsIterator;

	
	public TreeManagerProcessor(ITreeProcessor sourceProcessor) throws RuntimeException {
		this.manager = sourceProcessor.getManager();
		setSourceProcessor(sourceProcessor);
	}
	
	public TreeManagerProcessor(ITreeManager manager) {
		this.manager = manager;
	}
	
	public TreeManagerProcessor(IDimension dimension) {
		this.dimension = dimension;
	}

	@Override
	protected boolean fillRow(Row row) throws Exception {
		if (elementsIterator.hasNext()) {
			currentElement++;
			TreeColumn c = row.getColumn(0, TreeColumn.class);
			c.setElement(elementsIterator.next());
			return true;
		}
		return false;
	}

	@Override
	protected Row getRow() throws RuntimeException {
		return row;
	}

	@Override
	protected void init() throws RuntimeException {
		TreeColumn treeColumn = new TreeColumn();
		row.addColumn(treeColumn);
		
		if (manager!=null)
			elementsIterator = manager.getElementsSet().iterator();	
		else if (dimension!=null)
			elementsIterator = Arrays.asList(dimension.getElements(false)).iterator();
	}

	@Override
	public ITreeManager getManager() {
		return manager;
	}
	
	public int getRowsAccepted() {
		return currentElement;
	}
	
	

}
