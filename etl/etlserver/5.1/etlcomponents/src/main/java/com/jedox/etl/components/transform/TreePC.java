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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.transform;


import java.util.List;
import com.jedox.etl.components.config.transform.TreePCConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.node.AttributeNode;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.TreeBuildProcessor;
import com.jedox.etl.core.source.processor.UnionProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformInputProcessor;

public class TreePC extends TreeSource implements ITransform {

	private Row columns;
	protected String defaultElement;
	protected String defaultParentElement;
	

	public TreePC() {
		setConfigurator(new TreePCConfigurator());
	}

	public TreePCConfigurator getConfigurator() {
		return (TreePCConfigurator)super.getConfigurator();
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}

	public Row getRow() {
		return columns;
	}

	protected List<CoordinateNode> getCoordinates() {
		return getRow().getColumns(CoordinateNode.class);
	}

	protected FunctionManager getFunctionManager() {
		return (FunctionManager)getManager(ITypes.Functions);
	}

	public Row getAttributes() {
		Row row = new Row();
		row.addColumns(getRow().getColumns(AttributeNode.class));
		return row;
	}

	public IProcessor getProcessor(Views view) throws RuntimeException {
		//check if format is not given, which causes no generation but a pass-through of the input source.
		if (view.equals(Views.NONE))
			return getSourceProcessor(getSampleSize());
		//else generate and Render Tree
		return super.getProcessor(view);
	}

	public IProcessor getSourceProcessor(int size) throws RuntimeException {
		if (isExecutable()) {
			//return native format
			IProcessor sourceProcessor = initProcessor(UnionProcessor.getInstance(getSourceManager().getProcessors()),Facets.INPUT);
			IProcessor processor = initProcessor(new TransformInputProcessor(sourceProcessor,getFunctionManager(), getRow()),Facets.HIDDEN);
			processor.setLastRow(size);
			return processor;
		}
		return null;
	}

	public ITreeProcessor buildTree() throws RuntimeException {
		TreeBuildProcessor treeBuilder = new TreeBuildProcessor(getSourceProcessor(getSampleSize()),Views.PCWA);
		treeBuilder.setAttributes(getAttributes().getColumns(AttributeNode.class));
		treeBuilder.setDefaultElement(defaultElement);
		treeBuilder.setDefaultParentElement(defaultParentElement);
		initProcessor(treeBuilder,Facets.HIDDEN).run();
		setTreeManager(treeBuilder.getManager());		
		return treeBuilder;
	}

	public Row getOutputDescription() throws RuntimeException {
		Row row = new Row();
		row.addColumns(getRow().getColumns(CoordinateNode.class));
		return row;
	}

	public void init() throws InitializationException {
		super.init();
		try {
			columns = getConfigurator().getRow();
			addManager(new SourceManager());
			addManager(new FunctionManager());
			getSourceManager().addAll(getConfigurator().getSources());
			getFunctionManager().addAll(getConfigurator().getFunctions());
			defaultElement = getConfigurator().getDefaultElement();
			defaultParentElement = getConfigurator().getDefaultParentElement();
			if (defaultParentElement!=null && defaultElement==null)
				throw new InitializationException("No default parent element can be defined without a default element");			
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
