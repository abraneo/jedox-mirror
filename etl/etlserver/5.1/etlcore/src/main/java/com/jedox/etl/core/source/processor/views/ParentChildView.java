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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.INamedValue;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class ParentChildView implements IModelView {

	private ITreeManager manager;
	private List<List<ITreeElement>> rows = new ArrayList<List<ITreeElement>>();
	private Row row;
	private static final Log log = LogFactory.getLog(ParentChildView.class);
	
	protected void fillColumn(CoordinateNode target, INamedValue<ElementType> source) {
		if (source != null) {
			target.setValue(source.getName());
		}
		else { 
			target.setValue("");
		}
	}
	
	protected List<List<ITreeElement>> getSourceRows() {
		return rows;
	}
	
	protected Row getTargetRow() {
		return row;
	}
	
	protected ITreeManager getTree() {
		return manager;
	}
	
	public Row fillRow(int current) {
		List<ITreeElement> arow = rows.get(current);
		for (int i=0; i<arow.size(); i++) {
			fillColumn((CoordinateNode) row.getColumn(i), arow.get(i));
		}
		return row;
	}

	public Row setupRow(Row attributeRow) throws RuntimeException {
		row = new Row();
		row.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameParent(),null));
		row.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(NamingUtil.getElementnameChild(),null));
		return row;
	}
	
	public void init(ITreeManager manager) throws RuntimeException {
		this.manager = manager;
		Set<ITreeElement> visited = new HashSet<ITreeElement>();
		for (ITreeElement child : manager.getDescendants(null,false)) {
			if (!visited.contains(child)) {
				visited.add(child);
				if (child.getParentCount() == 0) {
					List<ITreeElement> arow = new ArrayList<ITreeElement>();
					arow.add(null);
					arow.add(child);
					rows.add(arow);
				}
				else for (ITreeElement parent : child.getParents()) {
					if (!parent.equals(child)) {
						List<ITreeElement> arow = new ArrayList<ITreeElement>();
						arow.add(parent);
						arow.add(child);
						rows.add(arow);
					} 
					else 
						log.warn("Found identical parent and child: "+parent.getName()+". Ignoring it..");
				}
			}
		}
	}

	public int size() {
		return rows.size();
	}
}
