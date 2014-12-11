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
package com.jedox.etl.core.transform;

import java.util.HashMap;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.function.IFunction;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.AnnexNode;
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
	private ColumnManager columns;
	private FunctionManager functions;
	private IProcessor sourceProcessor;
	private HashMap<ColumnNode,IColumn> binding;
	
	public TransformInputProcessor(IProcessor sourceProcessor, FunctionManager functions, ColumnManager columns, String name) throws RuntimeException {
		this.sourceProcessor = sourceProcessor;
		this.functions = functions.cloneManager();
		this.setSourceProcessor(sourceProcessor);
		this.setName(name);
		//this.columns = columns;
	
		this.columns = new ColumnManager();
		for (IColumn c : columns.getColumns()) {//CSCHW: clone columnmanager
			ColumnNode processorColumn;
			switch (c.getColumnType()) {
			case coordinate: processorColumn = new CoordinateNode(c.getName()); break;
			case level: processorColumn = new LevelNode(c.getName()); break;
			case annex: processorColumn = new AnnexNode(c.getName()); break;
			case value: processorColumn = new ValueNode(c.getName()); break;
			default: continue;
			}
			processorColumn.mimic(c);
			if (!c.isEmpty()) processorColumn.setConstantValue(c.getValue());
			if (processorColumn instanceof LevelNode) { //CSCHW also clone attributes
				LevelNode level = (LevelNode) processorColumn;
				Row attributes = new Row();
				for (IColumn a : level.getAttributes().getColumns()) {
					ColumnNode ac = new ColumnNode(a.getName());
					ac.mimic(a);
					if (!a.isEmpty()) ac.setConstantValue(a.getValue());
					attributes.addColumn(ac);
				}
				level.setAttributes(attributes);
			}
			this.columns.addColumn(processorColumn);
		}
		
		row.addColumns(this.columns.getColumnsOfType(IColumn.ColumnTypes.coordinate));
		row.addColumns(this.columns.getColumnsOfType(IColumn.ColumnTypes.level));
		row.addColumns(this.columns.getColumnsOfType(IColumn.ColumnTypes.annex));
		row.addColumns(this.columns.getColumnsOfType(IColumn.ColumnTypes.value));
		if (sourceProcessor != null)
			binding = bindToInput(sourceProcessor.current());
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
			Row values = columns.getColumnsOfType(IColumn.ColumnTypes.value);
			for (int i=0; i<values.size(); i++) {
				ValueNode v = (ValueNode) values.getColumn(i);
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
						if (input instanceof IFunction) {
							Row r = new Row();
							r.addColumn(t);
							detectCyclicInputs((IFunction) input, r, row);
						}	 
						node.setInput(input);
						binding.put(node, input);
					}
				}
			}
			t.setRowCountColumn(sourceProcessor.getRowCountColumn());
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindLevels(Row row) throws RuntimeException {
		//levels
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		Row levels = columns.getColumnsOfType(IColumn.ColumnTypes.level);
		for (int i=0;i<levels.size();i++) {
			LevelNode n = (LevelNode) levels.getColumn(i);
			IColumn input = findInput(n.getInputName(),row, !n.isEmpty());
			n.setInput(input);
			binding.put(n, input);
			Row attributes = n.getAttributes();
			for (IColumn c : attributes.getColumns()) {
				input = findInput(((ColumnNode)c).getInputName(),row, !c.isEmpty());
				//if not constant
				if (input != null) {
					n.setAttribute(c.getName(), input);
					binding.put((ColumnNode)c, input);
				}
			}
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindCoordinates(Row row) throws RuntimeException {
		//coordinates
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		Row coordinates = columns.getColumnsOfType(IColumn.ColumnTypes.coordinate);
		for (int i=0;i<coordinates.size();i++) {
			CoordinateNode n = (CoordinateNode) coordinates.getColumn(i);
			IColumn input = findInput(n.getInputName(),row, !n.isEmpty());
			n.setInput(input);
			binding.put(n, input);
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindValues(Row row) throws RuntimeException {
		//values
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		Row values = columns.getColumnsOfType(IColumn.ColumnTypes.value);
		for (int i=0;i<values.size();i++) {
			ValueNode n = (ValueNode) values.getColumn(i);
			IColumn input = findInput(n.getInputName(),row,!n.isEmpty());
			n.setInput(input);
			binding.put(n,input);
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindInfos(Row row) throws RuntimeException {
		//infos 
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		Row infos = columns.getColumnsOfType(IColumn.ColumnTypes.annex);
		for (int i=0;i<infos.size();i++) {
			AnnexNode n = (AnnexNode) infos.getColumn(i);
			IColumn input = findInput(n.getInputName(),row, !n.isEmpty());
			n.setInput(input);
			binding.put(n, input);
		}
		return binding;
	}
	
	private HashMap<ColumnNode, IColumn> bindToInput(Row row) throws RuntimeException {
		HashMap<ColumnNode, IColumn> binding = new HashMap<ColumnNode, IColumn>();
		if (row != null) {
			binding.putAll(bindInfos(row));
			binding.putAll(bindLevels(row));
			binding.putAll(bindCoordinates(row));
			binding.putAll(bindValues(row));
			binding.putAll(bindFunctions(row));
		}
		return binding;
	}
	
	@Override
	protected boolean fillRow(Row row) throws RuntimeException{
		Row current = sourceProcessor.current();
		Row next = sourceProcessor.next();
		//see if there was a change in the row structure of the underlying processor, which may occur on special processors. 
		if (current != next)
			binding = bindToInput(next);
		//ensure binding from input to row.
		for (ColumnNode n : binding.keySet()) {
			IColumn input = binding.get(n);
			if (input instanceof IFunction) {
				IFunction f = (IFunction) input;
				if (f.getParameter().getProperty("forceEval", "false").equalsIgnoreCase("true")) {
					IColumn calculatedInput = new Column();
					calculatedInput.mimic(input);
					calculatedInput.setValue(input.getValue());
					n.setInput(calculatedInput);
				} else {
					n.setInput(input);
				}
			} else {
				n.setInput(input);
			}
			
		}
		
		/* debug row content...
		for (int i=0; i<row.getColumnCount()-1;i++) {
			System.err.print("\""+row.getColumn(i).getValueAsString()+"\",");
		}
		System.err.println("\""+row.getColumn(row.getColumnCount()-1).getValueAsString()+"\"");
		*/
		return next != null;
	}

	protected Row getRow() {
		return row;
	}
	
	public void close() {
		super.close();
		for (IFunction f : getFunctionManager().getAll()) 
			f.close();
	}

}
