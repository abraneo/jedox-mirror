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
package com.jedox.etl.core.transform;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.node.AttributeNode;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.LevelNode;
import com.jedox.etl.core.node.ColumnNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.node.ValueNode.Operations;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;

public class TransformInputProcessor extends Processor {

	//private static final Log log = LogFactory.getLog(PipelineInputProcessor.class);
	private final Row row = new Row();
	private Row columns;
	private FunctionManager functions;
	private HashMap<ColumnNode,IColumn> binding;
	private long functionProcessorTime = 0;
	//private static final Log log = LogFactory.getLog(TransformInputProcessor.class);
	
	public TransformInputProcessor(IProcessor sourceProcessor, FunctionManager functions, Row columns) throws RuntimeException {
		this.functions = functions.cloneManager();
		this.columns = columns.clone();
		this.setSourceProcessor(sourceProcessor);
	}

	
	private FunctionManager getFunctionManager() {
		return functions;
	}
	
	/**
	 * Finds the input Colum for a pipeline object
	 * Starts with the datasource, then searches the transformations 
	 * @param name the name of the input Column
	 * @param datasource the datasource to search
	 * @return the input column
	 */
	private IColumn findInput(String name, Row row, boolean isConstant) throws RuntimeException {
		if (isConstant) return null;
		//search in functions
		IColumn c = getFunctionManager().get(name);
		//use datasource columns if not found 
		if (c == null)
			c = row.getColumn(name);
		if (c == null) {
			//look if input is a normalization measure, which is built in normalization process 
			List<ValueNode> values = columns.getColumns(ValueNode.class);
			for (int i=0; i<values.size(); i++) {
				ValueNode v = (ValueNode) values.get(i);
				if (v.getOperation().equals(Operations.NORMALIZE) && name.equals(v.getTarget())) return null;
			}
			//else throw exception
			throw new RuntimeException("Cannot bind to input "+name+" in Transform "+getName());
		}
		return c;
	}
	
	private void detectCyclicInputs(IFunction t, Row denied, Row allowed) throws RuntimeException {
		for (IColumn c : t.getInputs().getColumns()) {
			//cyclic input if column is not a constant and name matches tagged input node.
			if (c.isEmpty() && denied.containsColumn(c.getName()) && !allowed.containsColumn(c.getName())) {
				throw new RuntimeException("Cyclic dependency in input of function "+t.getName()+" detected: "+c.getName());
			}
			denied.addColumn(c);
			CoordinateNode cn = (CoordinateNode) c;
			if (cn.getInput() instanceof IFunction)  
				detectCyclicInputs((IFunction)cn.getInput(),denied,allowed);
			denied.removeColumn(c);
			}
	}
	
	private HashMap<ColumnNode, IColumn> bindFunctions(Row row) throws RuntimeException {
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		//functions
		IFunction[] ts = getFunctionManager().getAll();
		for (IFunction t : ts) {
			if (t.getInputs().size() > 0) { //function has defined inputs
				//bind input columns
				for (IColumn column : t.getInputs().getColumns()) {
					if (column.isEmpty()) { //not a constant
						CoordinateNode node = (CoordinateNode) column;
						IColumn input = null;
						//try first to take column from input, if function name is equal to its input to avoid cycle.
						if (t.getName().equals(column.getName()))
							input = row.getColumn(column.getName());
						if (input == null)
							input = findInput(column.getName(), row, false);
						node.setInput(input);
						binding.put(node, input);
					}
				}
			}
			t.setRowCountColumn(getSourceProcessor().getRowCountColumn());
		}
		//2nd pass cycle dectection after all inputs have been assigned.
		for (IFunction t : ts) {
			if (t.getInputs().size() > 0) { //function has defined inputs
				for (IColumn column : t.getInputs().getColumns()) {
					if (column.isEmpty()) { //not a constant
						CoordinateNode node = (CoordinateNode) column;
						IColumn input = node.getInput();
						if (input instanceof IFunction) {
							Row r = new Row();
							r.addColumn(t);
							detectCyclicInputs((IFunction) input, r, row);
						}	 
					}
				}
			}
			t.setRowCountColumn(getSourceProcessor().getRowCountColumn());
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindLevels(Row row) throws RuntimeException {
		//levels
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		List<LevelNode> levels = columns.getColumns(LevelNode.class);
		for (int i=0;i<levels.size();i++) {
			LevelNode n = levels.get(i);
			IColumn input = findInput(n.getInputName(),row, !n.isEmpty());
			n.setInput(input);
			binding.put(n, input);
			Row attributes = n.getAttributes();
			for (AttributeNode c : attributes.getColumns(AttributeNode.class)) {
				input = findInput(c.getInputName(),row, !c.isEmpty());
				//if not constant
				if (input != null) {
					c.setInput(input);
					binding.put(c, input);
				}
			}
			//finally check weight node, which is not an attribute any more
			ColumnNode w = n.getWeightNode();
			if (w != null && w.isEmpty()) { //weight node with reference
				input = findInput(w.getInputName(),row, false);
				w.setInput(input);
				binding.put(w, input);
			}
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindCoordinates(Row row) throws RuntimeException {
		//coordinates
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		List<CoordinateNode> coordinates = columns.getColumns(CoordinateNode.class);
		for (int i=0;i<coordinates.size();i++) {
			CoordinateNode n = coordinates.get(i);
			IColumn input = findInput(n.getInputName(),row, !n.isEmpty());
			n.setInput(input);
			binding.put(n, input);
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindAttributes(Row row) throws RuntimeException {
		//coordinates
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		List<AttributeNode> attributes = columns.getColumns(AttributeNode.class);
		for (int i=0;i<attributes.size();i++) {
			AttributeNode n = attributes.get(i);
			IColumn input = findInput(n.getInputName(),row, !n.isEmpty());
			n.setInput(input);
			binding.put(n, input);
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindValues(Row row) throws RuntimeException {
		//values
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		List<ValueNode> values = columns.getColumns(ValueNode.class);
		for (int i=0;i<values.size();i++) {
			ValueNode n = values.get(i);
			IColumn input = findInput(n.getInputName(),row,!n.isEmpty());
			n.setInput(input);
			binding.put(n,input);
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindToInput(Row row) throws RuntimeException {
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		if (row != null) {
			binding.putAll(bindLevels(row));
			binding.putAll(bindCoordinates(row));
			binding.putAll(bindAttributes(row));
			binding.putAll(bindValues(row));
			binding.putAll(bindFunctions(row));
		}
		return binding;
	}
	
	@Override
	protected boolean fillRow(Row row) throws RuntimeException{
		Row current = getSourceProcessor().current();
		Row next = getSourceProcessor().next();
		//see if there was a change in the row structure of the underlying processor, which may occur on special processors. 
		if (current != next)
			binding = bindToInput(next);
		//ensure binding from input to row.
		Map<ColumnNode, IFunction> evalFunctions = new HashMap<ColumnNode,IFunction>();
		for (ColumnNode n : binding.keySet()) {
			IColumn input = binding.get(n);
			if (input instanceof IFunction) {
				IFunction f = (IFunction) input;
				if (f.isForceEval()) {
					evalFunctions.put(n,f);
				} 
			} 
			n.setInput(input);
		}
		for (ColumnNode n : evalFunctions.keySet()) {
			IFunction f = evalFunctions.get(n);
			n.setValue(f.getValue());
		}
		return next != null;
	}

	protected Row getRow() {
		return row;
	}
	
	public void close() {
		super.close();
		for (IFunction f : getFunctionManager().getAll()){ 
			//log.info("Function " + f.getName() + " time: " + f.getOwnProcessingTime());
			for (IProcessor p : f.getUsedProcessorList()) {
				functionProcessorTime += p.getOverallProcessingTime();
			}
			f.close();
		}
	}
	
	public long getOwnProcessingTime() {
		return super.getOwnProcessingTime() - functionProcessorTime; //functionProcessorTime is already measured separately as processor time
	}


	@Override
	protected void init() throws RuntimeException {
		row.addColumns(this.columns.getColumns(CoordinateNode.class));
		row.addColumns(this.columns.getColumns(ValueNode.class));
		row.addColumns(this.columns.getColumns(LevelNode.class));
		row.addColumns(this.columns.getColumns(AttributeNode.class));
		if (getSourceProcessor() != null)
			binding = bindToInput(getSourceProcessor().current());	
	}

}
