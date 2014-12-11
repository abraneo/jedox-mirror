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
package com.jedox.etl.components.transform;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.TableTransformConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.UnionProcessor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformInputProcessor;

public abstract class TableTransform extends TableSource implements ITransform {
	private Row columns;
	protected static Log log = LogFactory.getLog(TableTransform.class);

	public TableTransform() {
		setConfigurator(new TableTransformConfigurator());
		notifyRetrieval = false;
	}

	public TableTransformConfigurator getConfigurator() {
		return (TableTransformConfigurator)super.getConfigurator();
	}

	public Row getRow() {
		return columns;
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}

	protected FunctionManager getFunctionManager() {
		return (FunctionManager)getManager(ITypes.Functions);
	}

	protected void setRow(IProcessor processor) throws RuntimeException {
		columns = new Row();
		for (IColumn c : processor.current().getColumns()) {
			CoordinateNode coordinate = ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(), c);
			columns.addColumn(coordinate);
		}
		for (IColumn f : getFunctionManager().getAll()) {
			columns.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(f.getName(), f));
		}
	}

	protected abstract IProcessor getInputProcessor(int size) throws RuntimeException; 

	protected abstract IProcessor getOutputProcessor(IProcessor input, int size) throws RuntimeException; 

	
	protected ValueNode.Operations getOperation() {
		List<ValueNode> values = getRow().getColumns(ValueNode.class);
		if (values.size() == 0)
			return ValueNode.Operations.NONE;
		ValueNode v = (ValueNode)values.get(0);
		return v.getOperation();
	}
	
	protected IProcessor getInputUnion() throws RuntimeException {
		IProcessor sourceProcessor = initProcessor(UnionProcessor.getInstance(getSourceManager().getProcessors()),Facets.INPUT);
		if (getRow() == null) //calculate columns in runtime
			setRow(sourceProcessor);
		return sourceProcessor;
	}

	public IProcessor getProcessor(int size) throws RuntimeException {
		if (isExecutable()) {
			log.info("Data retrieval from transform "+getName());
			IProcessor in = (isCached() ? super.getProcessor(size) : getInputProcessor(size));
			IProcessor out = getOutputProcessor(in,size);
			log.debug("Finishing data retrieval from transform "+getName());
			return out;
		}
		return null;
	}

	protected List<CoordinateNode> getCoordinates() {
		return getRow().getColumns(CoordinateNode.class);
	}

	protected List<ValueNode> getValues() {
		return getRow().getColumns(ValueNode.class);
	}

	private boolean initColumns() {
		columns = getConfigurator().getRow();
		if (columns != null) {
			//needs persistence if there is an aggregate function
			for (ValueNode column : columns.getColumns(ValueNode.class)) {
				if (column.hasAggregateFunction())
					return true;
			}
		}
		return false;
	}

	/**
	 * needed to comply to ISource interface
	 */
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		return getInputProcessor(size);
	}

	public Row getOutputDescription() throws RuntimeException {
		if (getRow() != null) {
			return initProcessor(new TransformInputProcessor(null,getFunctionManager(), getRow()),Facets.OUTPUT).getOutputDescription();
		}
		else {
			Row row = new Row();
			for (ISource s : getSourceManager().getSources()) {
				row.addColumns(s.getOutputDescription());
			}
			for (IFunction f : getFunctionManager().getAll()) {
				row.removeColumn(f.getName()); //remove input named identically to function, since function replaces that input.
				row.addColumn(f);
			}
			return row;
		}
	}
	
	public void invalidate() {
		super.invalidate();
		for (IFunction f : getFunctionManager().getAll()) 
			f.close();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			addManager(new FunctionManager());
			getSourceManager().addAll(getConfigurator().getSources());
			getFunctionManager().addAll(getConfigurator().getFunctions());
			if (initColumns() && !isCached()) { //does need persistence
				setCaching(getDefaultCacheType());
			}
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
