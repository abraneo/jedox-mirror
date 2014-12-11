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
package com.jedox.etl.components.transform;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.components.config.transform.TableConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.UnionProcessor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformInputProcessor;
import com.jedox.etl.core.transform.NormalizerProcessor;
import com.jedox.etl.core.transform.DenormalizerProcessor;
import com.jedox.etl.core.util.SQLUtil;

public class TableTransform extends TableSource implements ITransform {
	private ColumnManager columns;
	private RowFilter filter;
	private static Log log = LogFactory.getLog(TableTransform.class);
	private boolean ignoreEmpty;

	public TableTransform() {
		setConfigurator(new TableConfigurator());
		notifyRetrieval = false;
	}

	public TableConfigurator getConfigurator() {
		return (TableConfigurator)super.getConfigurator();
	}

	public ColumnManager getColumnManager() {
		return columns;
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}

	protected FunctionManager getFunctionManager() {
		return (FunctionManager)getManager(ITypes.Functions);
	}

	protected void setColumnManager(IProcessor processor) {
		columns = new ColumnManager();
		columns.addCoordinates(processor.current());
		for (IColumn f : getFunctionManager().getAll()) {
			//remove possibly existing input node with name name in favor of function.
			columns.removeColumn(f.getName());
			columns.addCoordinate(f.getName(), f.getName()).setColumnType(IColumn.ColumnTypes.function);
		}
	}

	protected IProcessor getInputProcessor(int size, boolean isCached) throws RuntimeException {
		if (!isCached) {
			IProcessor sourceProcessor = UnionProcessor.getInstance(getSourceManager().getProcessors());
			if (getColumnManager() == null) //calculate columns in runtime
				setColumnManager(sourceProcessor);
			IProcessor processor = new TransformInputProcessor(sourceProcessor,getFunctionManager(), getColumnManager(), getName());
			// processor.setName(getName());
			processor.setState(getState());
			processor.addFilter(filter);
			processor.setLastRow(size);
			return processor;
		}
		else return super.getProcessor(size);
	}

	protected ValueNode.Operations getOperation() {
		Row values = getColumnManager().getColumnsOfType(IColumn.ColumnTypes.value);
		if (values.size() == 0)
			return ValueNode.Operations.NONE;
		ValueNode v = (ValueNode)values.getColumn(0);
		return v.getOperation();
	}

	protected IProcessor getOutputProcessor(IProcessor input, int size, boolean isCached) throws RuntimeException {
		IProcessor out = input;
		ValueNode.Operations operation = getOperation();
		if (!isCached) {
			switch (operation) {
			// Option includeInfos of Processors is obsolete with 4.0. Used for ignorEmpty option to skip 0-values in 4.0.
			// Has to be redesigned			
			case NORMALIZE: out = new NormalizerProcessor(input, getColumnManager(), ignoreEmpty); break;
			case DENORMALIZE: out =  new DenormalizerProcessor(input,getColumnManager(), false); break;
			default: out = input; //none: no action needed
			}
		} else {
			switch (operation) {
			case NORMALIZE: out = new NormalizerProcessor(input, getColumnManager(), ignoreEmpty); break;
			case DENORMALIZE: out = input; //denormalization is already done via caching
			default: out = input; //none: no action needed
			}
		}
		out.setName(getName());
		out.setLastRow(size);
		return out;
	}

	public IProcessor getProcessor(int size) throws RuntimeException {
		if (isExecutable()) {
			log.info("Data retrieval from transform "+getName());
			IProcessor in = getInputProcessor(size,isCached());
			IProcessor out = getOutputProcessor(in,size,isCached());
			log.debug("Finishing data retrieval from transform "+getName());
			return out;
		}
		return null;
	}

	protected Row getCoordinates() {
		return getColumnManager().getColumnsOfType(IColumn.ColumnTypes.coordinate);
	}

	protected Row getValues() {
		return getColumnManager().getColumnsOfType(IColumn.ColumnTypes.value);
	}
	
	protected RowFilter getFilter() {
		return filter;
	}

	private boolean initColumns() {
		columns = getConfigurator().getColumnManager();
		if (columns != null) {
			//needs persistence if there is an aggregate function
			for (IColumn column : columns.getColumns()) {
				if (((CoordinateNode)column).hasAggregateFunction())
					return true;
			}
		}
		return false;
	}

	public String getAggregateName(CoordinateNode coordinate) {
		if (coordinate.getAggregateFunction().equalsIgnoreCase("none")) return escapeName(coordinate.getName());
		return coordinate.getAggregateFunction()+"("+escapeName(coordinate.getName())+") as "+escapeName(coordinate.getName());
	}

	private String getFields() {
		List<String> fields = new ArrayList<String>();
		for (IColumn c : getCoordinates().getColumns()) {
			fields.add(getAggregateName((CoordinateNode)c));
		}
		for (IColumn c : getValues().getColumns()) {
			fields.add(getAggregateName((CoordinateNode)c));
		}
		return SQLUtil.enumNames(fields);
	}

	private String getGroupBy(String fields) {
		List<String> group = new ArrayList<String>();
		for (IColumn c : getCoordinates().getColumns()) {
			CoordinateNode coordinate = (CoordinateNode) c;
			if (!coordinate.hasAggregateFunction())
				group.add(escapeName(coordinate.getName()));
		}
		for (IColumn c : getValues().getColumns()) {
			CoordinateNode coordinate = (CoordinateNode) c;
			if (!coordinate.hasAggregateFunction())
				group.add(escapeName(coordinate.getName()));
		}
		String result = SQLUtil.enumNames(group);
		if (result.equals(fields))
			return "";
		else
			return result;
	}


	/**
	 * needed to comply to ISource interface
	 */
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor in = getInputProcessor(size,false);
		IProcessor out = in;
		//Denormalize if necessary
		if (getOperation().equals(ValueNode.Operations.DENORMALIZE))
			out = new DenormalizerProcessor(in,getColumnManager(), false);
		String fields = getFields();
		String groupBy = getGroupBy(fields);
		String query = SQLUtil.buildQuery(getLocator().getPersistentName(),fields,"",groupBy,"");
		//manipulate internal query
		setQueryInternal(query);
		return out;
	}

	public Row getOutputDescription() throws RuntimeException {
		if (getColumnManager() != null) {
			ValueNode.Operations op = getOperation();
			IProcessor processor = null;
			switch (op) {
			case NONE: processor =  new TransformInputProcessor(null,getFunctionManager(), getColumnManager(), getName()); break;
			case NORMALIZE: processor = new NormalizerProcessor(null, getColumnManager(), ignoreEmpty); break;
			case DENORMALIZE: processor = new DenormalizerProcessor(null, getColumnManager(), false); break;
			default: processor = getOutputProcessor(getInputProcessor(1,isCached()),1,isCached()); //fallback should never be used, since it is more expensive
			}
			Row od = processor.getOutputDescription();
			if (isCached()) { //remove annexes
				for (IColumn c : getColumnManager().getColumnsOfType(IColumn.ColumnTypes.annex).getColumns()) {
					od.removeColumn(c.getName());
				}
			}
			return od;
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
	
	public Element getComponentDescription() throws ConfigurationException {
		Element transform = super.getComponentDescription();
		return transform;
	}

	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			addManager(new FunctionManager());
			getSourceManager().addAll(getConfigurator().getSources());
			getFunctionManager().addAll(getConfigurator().getFunctions());
			filter = getConfigurator().getFilter();
			if (initColumns()) { //does need persistence
				setCaching(true);
			}
			ignoreEmpty = Boolean.valueOf(getParameter("ignoreEmpty","false"));
			if (ignoreEmpty)
				log.info("Ignore empty option set in transform "+getName());
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
