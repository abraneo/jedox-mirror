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
package com.jedox.etl.core.source.processor;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.views.ElementAttributesView;
import com.jedox.etl.core.source.processor.views.FullHierarchyView;
import com.jedox.etl.core.source.processor.views.FullHierarchyWeightedView;
import com.jedox.etl.core.source.processor.views.IModelView;
import com.jedox.etl.core.source.processor.views.NCAttributesView;
import com.jedox.etl.core.source.processor.views.NCView;
import com.jedox.etl.core.source.processor.views.ParentChildView;
import com.jedox.etl.core.source.processor.views.ParentChildWeightAttributesTypeView;
import com.jedox.etl.core.source.processor.views.ParentChildWeightAttributesView;
import com.jedox.etl.core.source.processor.views.ParentChildWeightView;

public class TreeProcessor extends Processor {
	
	private int current = 0;
	private Row row;
	private IModelView modelView;
	private Views view;
	private boolean initialized = false;
	private static final Log log = LogFactory.getLog(TreeProcessor.class);
	private ITreeSource source;
	
	public TreeProcessor(ITreeSource source, Views view) throws RuntimeException {
		this.view = view;
		this.source = source;
		switch (view) {
			case FH: modelView = new FullHierarchyView(); break;
			case FHW: modelView = new FullHierarchyWeightedView(); break;
			case EA: modelView = new ElementAttributesView(); break;
			case PC: modelView = new ParentChildView(); break;
			case PCW: modelView = new ParentChildWeightView(); break;
			case PCWA: modelView = new ParentChildWeightAttributesView(); break;
			case NCW: modelView = new NCView(); break;
			case NCWA: modelView = new NCAttributesView(); break;
			case PCWAT: modelView = new ParentChildWeightAttributesTypeView(); break;
			default: log.error("Tree View of format "+view.toString()+" not supported yet.");
		}
		row = modelView.setupRow(source);
		setName(source.getName());
		setState(source.getState());
	}
	
	protected Row getRow() {
		return row;
	}
	
	public void close() {
		modelView = null;
	}

	@Override
	protected boolean fillRow(Row row) throws Exception {
		if (!initialized) {
			modelView.init();
			initialized = true;
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
}
