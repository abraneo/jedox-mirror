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

import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.views.ElementAttributesView;
import com.jedox.etl.core.source.processor.views.FullHierarchyView;
import com.jedox.etl.core.source.processor.views.FullHierarchyWeightedView;
import com.jedox.etl.core.source.processor.views.IModelView;
import com.jedox.etl.core.source.processor.views.LevelElementWeightAttributesView;
import com.jedox.etl.core.source.processor.views.LevelElementWeightView;
import com.jedox.etl.core.source.processor.views.NCAttributesView;
import com.jedox.etl.core.source.processor.views.NCView;
import com.jedox.etl.core.source.processor.views.ParentChildView;
import com.jedox.etl.core.source.processor.views.ParentChildWeightAttributesTypeView;
import com.jedox.etl.core.source.processor.views.ParentChildWeightAttributesView;
import com.jedox.etl.core.source.processor.views.ParentChildWeightView;
import com.jedox.etl.core.component.RuntimeException;

public class TreeViewProcessor extends Processor implements ITreeProcessor {
	
	private int current = 0;
	private Row row;
	private IModelView modelView;
	private Views view;
	private ITreeSource source;
	private ITreeProcessor processor;
	
	public TreeViewProcessor(ITreeSource source, Views view) {
		this.view = view;
		this.source = source;
	}
	
	public TreeViewProcessor(ITreeProcessor processor, Views view) {
		this.view = view;
		this.processor = processor;
	}
	
	public TreeViewProcessor(ITreeSource source, Views view, boolean description) throws RuntimeException {
		this.view = view;
		this.source = source;
		if (description) initRow();
	}
	
	protected Row getRow() {
		return row;
	}
	
	public void close() {
		modelView = null;
	}

	@Override
	protected boolean fillRow(Row row) throws RuntimeException {
		if (modelView==null) {
			// Tree Processor has already been closed
			return false;
		}
		if (current < modelView.size()) {
			row = modelView.fillRow(current);
			current++;
			return true;
		}
		//reset the processor for new use
		//reset();
		return false;
	}

	public Views getFormat() {
		return view;
	}

	protected boolean initRow() throws RuntimeException {
		boolean generated = false;
		switch (view) {
			case FH: {
				setSourceProcessor(getTreeProcessor()); //we need to generate to init rows
				modelView = new FullHierarchyView(getTreeProcessor().getManager()); 
				generated = true;
				break;
			}
			case FHW: {	//we need to generate to init rows
				setSourceProcessor(getTreeProcessor());
				modelView = new FullHierarchyWeightedView(getTreeProcessor().getManager());
				generated = true;
				break;
			}
			case EA: modelView = new ElementAttributesView(); break;
			case PC: modelView = new ParentChildView(); break;
			case PCW: modelView = new ParentChildWeightView(); break;
			case PCWA: modelView = new ParentChildWeightAttributesView(); break;
			case NCW: modelView = new NCView(); break;
			case NCWA: modelView = new NCAttributesView(); break;
			case PCWAT: modelView = new ParentChildWeightAttributesTypeView(); break;
			case LEW: modelView = new LevelElementWeightView(); break;
			case LEWTA: modelView = new LevelElementWeightAttributesView(); break;
			default: throw new RuntimeException("Tree View of format "+view.toString()+" not supported yet.");
		}
		row = modelView.setupRow(getAttributeRow());
		return generated;
	}
	
	protected ITreeProcessor getTreeProcessor() throws RuntimeException {
		if (processor == null && source != null) {
			processor = source.generate();
		}
		if (processor == null) throw new RuntimeException("Cannot build view on null tree.");
		return processor;
	}
	
	protected Row getAttributeRow() throws RuntimeException {
		if (source == null) {
			Attribute[] attributes = getTreeProcessor().getManager().getAttributes();
			Row result = new Row();
			for (Attribute a : attributes) {
				result.addColumn(ColumnNodeFactory.getInstance().createAttributeNode(a, null));
			}
			return result;
		} else {
			return source.getAttributes();
		}
	}
	
	@Override
	protected void init() throws RuntimeException {
		if (!initRow()) {
			//init treeprocessor here!
			setSourceProcessor(getTreeProcessor());
		}
		modelView.init(getTreeProcessor().getManager());
	}

	@Override
	public ITreeManager getManager() throws RuntimeException {
		return getTreeProcessor().getManager();
	}
}
