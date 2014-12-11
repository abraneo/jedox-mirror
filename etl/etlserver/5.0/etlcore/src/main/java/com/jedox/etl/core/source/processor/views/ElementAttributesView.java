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

import java.util.List;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.INamedValue;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class ElementAttributesView implements IModelView {

	private List<ITreeElement> elements;
	private Row row = new Row();
	private ITreeSource source;
	//private static final Log log = LogFactory.getLog(ElementAttributesView.class);
	private AttributesUtil attributes;

/*
	protected ColumnNodeManager getColumns() {
		return columns;
	}
*/

	protected List<ITreeElement> getElements() {
		return elements;
	}

	protected Row getTargetRow() {
		return row;
	}

	protected CoordinateNode fillElement(int currentRow) {
		INamedValue<ElementType> node = elements.get(currentRow);
		//get element node as the first column
		CoordinateNode element = (CoordinateNode) row.getColumn(0);
		element.setValue(node.getName());
		if (node.getValue() != null) element.setElementType(node.getValue().toString());
		return element;
	}

	public Row fillRow(int current) {
		fillElement(current);
		for (int i=0; i<attributes.getAttributes().size(); i++) {
			attributes.fillAttribute(elements.get(current),i,source.getTreeManager());
		}
		return row;
	}

	public Row setupRow(ITreeSource source) throws RuntimeException {
		this.source = source;
		attributes = new AttributesUtil(source.getAttributes());
		//add a coordinate node for the dimension element
		CoordinateNode element = new CoordinateNode(NamingUtil.getElementnameElement());
		//now build the row
		row.addColumn(element);
		row.addColumns(attributes.getAttributes());
		return row;
	}

	public void init() throws RuntimeException {
		ITreeManager manager = source.generate();
		//get all nodes
		elements = manager.getDescendants(null,false);
	}

	public int size() {
		return elements.size();
	}

}
