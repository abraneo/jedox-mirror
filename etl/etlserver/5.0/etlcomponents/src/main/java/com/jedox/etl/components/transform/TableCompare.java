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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.transform;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformInputProcessor;

/**
 * Compares an arbitrary set of sources (>0) for differences in their columns and values.
 * @author Christian Schwarzinger
 *
 */
public class TableCompare extends TableTransform implements ITransform {
	
	private static enum Modes {
		line, key
	}
	
	private static enum References {
		source, origin
	}
	
	private abstract class CompareProcessor extends Processor {
		
		protected class ProcessorTuple {
			public IProcessor source;
			public IProcessor target;
		}
		
		private Row row = new Row();
		private List<ProcessorTuple> tuples = new ArrayList<ProcessorTuple>();
		protected ProcessorTuple currentTuple;
		protected Column sourceTable;
		protected Column targetTable;
		protected Column sourceColumn;
		protected Column targetColumn;
		protected Column rowNumber;
		protected Column sourceValue;
		protected Column targetValue;
		protected Column timestamp;
		protected List<Row> buffer = new ArrayList<Row>();
		private long sourceRows = 0;
		
		
		public CompareProcessor(List<IProcessor> processors) throws RuntimeException {
			init(processors);
		}
		
		private Row getTargetRow() {
			Row row = new Row();
			timestamp = new Column("timestamp");
			timestamp.setValueType(Date.class.getCanonicalName());
			timestamp.setValue(new Date());
			sourceTable = new Column("sourceTable");
			targetTable = new Column("targetTable");
			sourceColumn = new Column("sourceColumn"); 
			targetColumn = new Column("targetColumn");
			rowNumber = new Column("rowNumber");
			sourceValue = new Column("sourceValue");
			targetValue = new Column ("targetValue");
			row.addColumn(rowNumber);
			row.addColumn(timestamp);
			row.addColumn(sourceTable);
			row.addColumn(targetTable);
			row.addColumn(sourceColumn);
			row.addColumn(targetColumn);
			row.addColumn(sourceValue);
			row.addColumn(targetValue);
			return row;
		}
		
		protected void init(List<IProcessor> processors) throws RuntimeException {
			if (processors.size() < 2) {
				throw new RuntimeException("At least two sources are requried for comparing.");
			}
			row = getTargetRow();
			for (int i=0; i<processors.size(); i++) {
				for (int j=i+1; j<processors.size(); j++) {
					ProcessorTuple tuple = new ProcessorTuple();
					tuple.source = processors.get(i);
					tuple.target = processors.get(j);
					tuples.add(tuple);
				}
			}
			setName(TableCompare.this.getName());
			currentTuple = initTuple();
		}
		
		protected String getTableName(IProcessor processor) {
			switch (reference) {
			case origin: return processor.getOrigin(); 
			default: return processor.getName();
			}
		}
		
		protected IProcessor getProcessor(IProcessor processor) {
			switch (reference) {
			case origin: return processor.getProcessorChain().get(0); 
			default: return processor;
			}
		}
		
		/**
		 * gets and removes a source / target tuple from the list of possible tuples.
		 * @return the tuple
		 */
		protected ProcessorTuple initTuple() throws RuntimeException {
			if (tuples.size() > 0) {
				ProcessorTuple tuple = tuples.remove(0);
				getProcessor(tuple.source).getRowCountColumn();
				getProcessor(tuple.target).getRowCountColumn();
				return tuple;
			}
			return null;
		}
		
		/**
		 * clears all columns which might get a null value.
		 */
		protected void clearColumns() {
			sourceColumn.setValue(null);
			targetColumn.setValue(null);
			sourceValue.setValue(null);
			targetValue.setValue(null);
		}
		
		/**
		 * adds a new processor row to the buffer. We need the buffer, since each line of data may have multiple differences.
		 */
		protected Row addBufferRow() {
			Row bufferRow = row.clone();
			IColumn sourceTableClone = bufferRow.getColumn(sourceTable.getName());
			IColumn targetTableClone = bufferRow.getColumn(targetTable.getName());
			String source = getTableName(currentTuple.source);
			String target = getTableName(currentTuple.target);
			if (subheaders && (!source.equals(sourceTable.getValue()) || !target.equals(targetTable.getValue()))) { //source / origin has changed. Add new header line if needed.
				Row headerRow = row.clone();
				IColumn sourceTableHeader = headerRow.getColumn(sourceTable.getName());
				IColumn targetTableHeader = headerRow.getColumn(targetTable.getName());
				sourceTableHeader.setValue(source);
				targetTableHeader.setValue(target);
				buffer.add(headerRow);
			}
			sourceTable.setValue(source);
			targetTable.setValue(target);
			sourceTableClone.setValue(source);
			targetTableClone.setValue(target);
			IColumn cloneSourceColumn = bufferRow.getColumn(sourceColumn.getName());
			cloneSourceColumn.setValue(sourceColumn.getValue());
			IColumn cloneTargetColumn = bufferRow.getColumn(targetColumn.getName());
			cloneTargetColumn.setValue(targetColumn.getValue());
			IColumn cloneSourceValue = bufferRow.getColumn(sourceValue.getName());
			cloneSourceValue.setValue(sourceValue.getValue());
			IColumn cloneTargetValue = bufferRow.getColumn(targetValue.getName());
			cloneTargetValue.setValue(targetValue.getValue());
			IColumn rowNumberClone = bufferRow.getColumn(rowNumber.getName());
			rowNumberClone.setValue(rowNumber.getValue());
			buffer.add(bufferRow);
			return bufferRow;
		}
		
		/**
		 * tries to fill the row from the internal buffer and removes one line from the buffer if successful
		 * @param row the processor row to be filled
		 * @return true if successful, false if buffer is empty.
		 */
		protected boolean fillFromBuffer(Row row) {
			if (buffer.size() > 0) {
				Row bufferRow = buffer.remove(0);
				for (IColumn c : row.getColumns()) {
					IColumn bc = bufferRow.getColumn(c.getName());
					if (bc != null)
						c.setValue(bc.getValue());
					else
						c.setValue(null);
				}
				return true;
			}
			return false;
		}

		/**
		 * Writes into the buffer, when only processor has a valid row while the other one is finished.
		 * @param row the row of the processor having still data
		 * @param nameColumn the name column to hold the name of this processor
		 * @param valueColumn the value column where the values should go.
		 * @param rowCount the rowcount column of the processor
		 */
		protected void checkSingleRow(Row row, IColumn nameColumn, IColumn valueColumn, IColumn rowCount) throws RuntimeException {
			for (IColumn c : row.getColumns()) {
				Object value = c.getValue();
				if (value != null) {
					clearColumns();
					nameColumn.setValue(c.getName());
					valueColumn.setValue(c.getValue());
					rowNumber.setValue(rowCount.getValue());
				}
				addBufferRow();
			}
		}
		
		protected boolean equals(Object o1, Object o2) {
			if (o1 == null) o1 = "";
			if (o2 == null) o2 = "";
			//if (o1 == null && o2 == null) return true;
			//if (o1 == null && o2 != null) return false;
			return o1.equals(o2);
		}
		
		/**
		 * Checks for differences between the rows of the two processors.
		 * @param sourceRow the row of the source processor
		 * @param targetRow the row of the target processor
		 * @param rowCount the rowcount column of the source processor (same as target processor, since both still have lines
		 * @throws Exception
		 */
		protected void checkRows(Row sourceRow, Row targetRow, IColumn rowCount) {
			//check all source->target
			for (IColumn sc : sourceRow.getColumns()) {
				IColumn tc = targetRow.getColumn(sc.getName());
				Object sv = sc.getValue();
				if (!ignoreMissingColumns && tc == null && sv != null) { //value in source but no column in target, add row
					sourceColumn.setValue(sc.getName());
					sourceValue.setValue(sv);
					targetColumn.setValue("#missing");
					targetValue.setValue(null);
					rowNumber.setValue(rowCount.getValue());
					addBufferRow();
				}
				if (tc == null && sv != null) { //value in source but no column in target, do log
					log.warn("Sources "+getTableName(currentTuple.source)+" and "+getTableName(currentTuple.target)+" are structurally different: Column "+sc.getName()+" is only present in "+getTableName(currentTuple.source)+".");
				}
				if (tc != null) { //column present in both source and target
					Object tv = tc.getValue();
					if (!equals(sv,tv)) {
						sourceColumn.setValue(sc.getName());
						sourceValue.setValue(sv);
						targetColumn.setValue(tc.getName());
						targetValue.setValue(tv);
						rowNumber.setValue(rowCount.getValue());
						addBufferRow();
					}
				}
			}
			//check for target column without source presence
			for (IColumn tc : targetRow.getColumns()) {
				IColumn sc = sourceRow.getColumn(tc.getName());
				Object tv = tc.getValue();
				if (!ignoreMissingColumns && sc == null && tv != null) { //value in target but not in source, add row
					sourceColumn.setValue("#missing");
					sourceValue.setValue(null);
					targetColumn.setValue(tc.getName());
					targetValue.setValue(tv);
					rowNumber.setValue(rowCount.getValue());
					addBufferRow();
				}
				if (sc == null && tv != null) { //value in target but not in source, do log
					log.warn("Sources "+getTableName(currentTuple.source)+" and "+getTableName(currentTuple.target)+" are structurally different: Column "+tc.getName()+" is only present in "+getTableName(currentTuple.target)+".");
				}
			}
		}
		
		protected abstract Row getSourceRow(ProcessorTuple tuple) throws RuntimeException;
		
		protected abstract Row getTargetRow(ProcessorTuple tuple) throws RuntimeException;
		
		
		@Override
		protected boolean fillRow(Row row) throws Exception {
			while (true) {
				if (fillFromBuffer(row)) {
					return true;
				}
				else {
					if (currentTuple != null) {
						Row sourceRow = getSourceRow(currentTuple);
						Row targetRow = getTargetRow(currentTuple);
						IColumn rowCountColumn = getProcessor(currentTuple.source).getRowCountColumn();
						if (sourceRow != null && targetRow != null) 
							checkRows(sourceRow,targetRow,rowCountColumn);
						if (sourceRow == null && targetRow != null) {
							rowCountColumn.setValue((Integer)rowCountColumn.getValue()+1);
							checkSingleRow(targetRow,targetColumn,targetValue,rowCountColumn);
						}
						if (sourceRow != null && targetRow == null)
							checkSingleRow(sourceRow,sourceColumn,sourceValue,rowCountColumn);
						if (sourceRow == null && targetRow == null) {//we are finished with this tuple
							currentTuple = initTuple();
						} else {
							sourceRows++;
						}
					}
					else return false;
				}		
			}
		}

		@Override
		protected Row getRow() {
			return row;
		}
		
		@SuppressWarnings("unused")
		public long getSourceRowCount() {
			return sourceRows;
		}
		
	}
	
	private class LineCompareProcessor extends CompareProcessor {

		public LineCompareProcessor(List<IProcessor> processors) throws RuntimeException {
			super(processors);
		}

		@Override
		protected Row getTargetRow(ProcessorTuple tuple) throws RuntimeException {
			return tuple.target.next();
		}

		@Override
		protected Row getSourceRow(ProcessorTuple tuple) throws RuntimeException {
			return tuple.source.next();
		}
		
	}
	
	private class RowComparator implements Comparator<Row> {

		@Override
		public int compare(Row o1, Row o2) {
			return Integer.valueOf(o1.getColumn("rowNumber").getValueAsString()).compareTo(Integer.valueOf(o2.getColumn("rowNumber").getValueAsString()));
		}
	}
	
	private class KeyCompareProcessor extends CompareProcessor {
		
		private class ValueTuple {
			public String name;
			public Object value;
		}
		
		private Map<String,Map<Object,List<ValueTuple>>> lookup;
		private String currentSourceName;
		private String currentTargetName;
		private int sourceNumber = 0;
		private int targetNumber = 0;
		private IColumn sourceKey;
		private IColumn targetKey;
		private Map<String,String> sources = new HashMap<String,String>();
		private Map<String,String> targets = new HashMap<String,String>();
		private Map<String,Set<Object>> duplicateKeys = new HashMap<String,Set<Object>>();
		private String rowCountColumnName;

		public KeyCompareProcessor(List<IProcessor> processors) throws RuntimeException {
			super(processors);
			initLookup(currentTuple);
			sourceKey = new Column("sourceKey");
			targetKey = new Column("targetKey");
			getRow().addColumn(sourceKey);
			getRow().addColumn(targetKey);
		}
		
		private String getSourceNumber(IProcessor source) {
			String tableName = super.getTableName(source);
			if (!tableName.equals(currentSourceName)) {
				currentSourceName = tableName;
				sourceNumber++;
				sources.put(String.valueOf(sourceNumber), tableName);
			}
			return String.valueOf(sourceNumber);
		}
		
		private String getTargetNumber(IProcessor target) {
			String tableName = super.getTableName(target);
			if (!tableName.equals(currentTargetName)) {
				currentTargetName = tableName;
				targetNumber++;
				targets.put(String.valueOf(targetNumber), tableName);
			}
			return String.valueOf(targetNumber);
		}
		
		protected String getTableName(IProcessor processor) {
			if (processor.equals(currentTuple.source))
				return sources.get(getSourceNumber(processor));//super.getTableName(processor);
			return targets.get(getSourceNumber(currentTuple.source));
		}
		
		private List<ValueTuple> getTuplesFromRow(Row row) {
			List<ValueTuple> result = new ArrayList<ValueTuple>();
			for (IColumn c : row.getColumns()) {
				ValueTuple tuple = new ValueTuple();
				tuple.name = c.getName();
				tuple.value = c.getValue();
				result.add(tuple);
			}
			return result;
		}
		
		private Row getRowFromTuples(List<ValueTuple> tuples) {
			if (tuples == null) return null;
			Row result = new Row();
			for (ValueTuple t: tuples) {
				if (!t.name.equals(rowCountColumnName)) {
					Column c = new Column(t.name);
					c.setValue(t.value);
					result.addColumn(c);
				}
				else {
					result.setName(String.valueOf(t.value));
				}
			}
			return result;
		}
		
		protected void initLookup(ProcessorTuple current) throws RuntimeException {
			if (current != null) {
				lookup = new HashMap<String,Map<Object,List<ValueTuple>>>();
				IColumn rowCountColumn = getProcessor(current.target).getRowCountColumn();
				rowCountColumnName = rowCountColumn.getName();
				Row target = current.target.next();
				while (target != null) {
					rowCountColumn = getProcessor(current.target).getRowCountColumn();
					Object keyValue = getKeyValue(current.target);
					if (keyValue != null) {
						String position = getTargetNumber(current.target);
						Map<Object,List<ValueTuple>> mapPart = lookup.get(position);
						if (mapPart == null) {
							mapPart = new HashMap<Object,List<ValueTuple>>();
							lookup.put(position, mapPart);
						}
						Set<Object> duplicateKeysPart = duplicateKeys.get(position);
						if (duplicateKeysPart == null) {
							duplicateKeysPart = new HashSet<Object>();
							duplicateKeys.put(position, duplicateKeysPart);
						}
						if (mapPart.containsKey(keyValue) || duplicateKeysPart.contains(keyValue)) {
							duplicateKeysPart.add(keyValue);
							IColumn keyColumn = getKeyRow(current.target).getColumn(keyColumnPos);
							log.warn("Key value "+keyValue+" of column "+keyColumn.getName()+" at row "+rowCountColumn.getValue()+" is duplicate. Key will be ignored.");
							mapPart.remove(keyValue);
						}
						else {
							Row keyRow = getKeyRow(current.target);
							/*
							//clone row and set values
							Row cloneRow = keyRow.clone();
							//set table origin into row.
							cloneRow.setName(rowCountColumn.getValueAsString());
							for (int i=0; i<keyRow.size(); i++) {
								cloneRow.getColumn(i).setValue(keyRow.getColumn(i).getValue());
							}
							*/
							List<ValueTuple> tuples = getTuplesFromRow(keyRow);
							ValueTuple rowNumber = new ValueTuple();
							rowNumber.name = rowCountColumn.getName();
							rowNumber.value = rowCountColumn.getValue();
							tuples.add(rowNumber);
							mapPart.put(keyValue,tuples);
						}
					}
					target = current.target.next();
				}
			}
		}
		
		private Row getKeyRow(IProcessor processor) throws RuntimeException {
			Row keyRow = null;
			switch (reference) {
			case origin: keyRow = processor.getProcessorChain().get(0).current(); break;
			default: keyRow = processor.current();
			}
			if (keyRow.size() <= keyColumnPos) {
				throw new RuntimeException("Key position "+keyColumnPos+" is greater then number of available columns in source "+super.getTableName(processor)+": "+keyRow.size());
			}
			return keyRow;
		}
		
		private Object getKeyValue(IProcessor processor) throws RuntimeException {
			Row keyRow = getKeyRow(processor);
			if (keyRow != null) {
				IColumn keyColumn = keyRow.getColumn(keyColumnPos);
				Object keyValue = keyColumn.getValue();
				if (keyValue == null) {
					log.warn("Key value "+keyValue+" of column "+keyColumn.getName()+" at row "+processor.getRowCountColumn().getValue()+" is null. Row from source "+super.getTableName(processor)+" will we ignored.");
				}
				return keyValue;
			}
			return null;
		}

		@Override
		protected Row getTargetRow(ProcessorTuple tuple) throws RuntimeException {
			if (!tuple.source.isClosed()) {
				Object keyValue = getKeyValue(tuple.source);
				targetKey.setValue(keyValue);
				Map<Object,List<ValueTuple>> mapPart = lookup.get(getSourceNumber(tuple.source));
				Row row = null;
				if (mapPart != null) {
					List<ValueTuple> t = mapPart.remove(keyValue);
					row = getRowFromTuples(t);
				}
				return row;
			}
			else {
				return null;
			}
				
		}

		@Override
		protected Row getSourceRow(ProcessorTuple tuple) throws RuntimeException {
			String lastSourceNumber = getSourceNumber(tuple.source);
			Row sourceRow = tuple.source.next();
			if (!getSourceNumber(tuple.source).equals(lastSourceNumber)) { //flush all non matching targets of current tuple to buffer
				Map<Object,List<ValueTuple>> mapPart = lookup.get(lastSourceNumber);
				if (mapPart != null) {
					Iterator<Object> i = mapPart.keySet().iterator();
					List<Row> tempBuffer = new ArrayList<Row>();
					while (i.hasNext()) {
						Object keyValue = i.next();
						targetKey.setValue(keyValue);
						Row row = getRowFromTuples(mapPart.get(keyValue));
						Row bufferRow = getRow().clone();
						IColumn sourceTableClone = bufferRow.getColumn(sourceTable.getName());
						IColumn targetTableClone = bufferRow.getColumn(targetTable.getName());
						sourceTableClone.setValue(sources.get(lastSourceNumber));
						targetTableClone.setValue(targets.get(lastSourceNumber));
						IColumn cloneTargetColumn = bufferRow.getColumn(targetColumn.getName());
						cloneTargetColumn.setValue(row.getColumn(keyColumnPos).getName());
						IColumn cloneTargetValue = bufferRow.getColumn(targetValue.getName());
						cloneTargetValue.setValue(row.getColumn(keyColumnPos).getValue());
						IColumn rowNumberClone = bufferRow.getColumn(rowNumber.getName());
						rowNumberClone.setValue(Integer.parseInt(row.getName()));
						bufferRow.getColumn(targetKey.getName()).setValue(targetKey.getValue());
						tempBuffer.add(bufferRow);
					}
					Collections.sort(tempBuffer, new RowComparator());
					buffer.addAll(tempBuffer);
				}
			}
			Object keyValue = getKeyValue(tuple.source);
			while (keyValue == null && sourceRow != null) {
				sourceRow = tuple.source.next();
				keyValue = getKeyValue(tuple.source);
			}
			sourceKey.setValue(keyValue);
			return sourceRow;
		}
		
		protected void checkSingleRow(Row row, IColumn nameColumn, IColumn valueColumn, IColumn rowCount) throws RuntimeException  {
			Row keyOnly = new Row();
			//only log key in single row mode.
			if (row.equals(currentTuple.source.current())) {
				keyOnly.addColumn(getKeyRow(currentTuple.source).getColumn(keyColumnPos));
			} else { //target is always already keyrow.
				keyOnly.addColumn(row.getColumn(keyColumnPos));
			}
			super.checkSingleRow(keyOnly, nameColumn, valueColumn, rowCount);
			//check for special case duplicate keys
			Set<Object> duplicateKeysPart = duplicateKeys.get(getSourceNumber(currentTuple.source));
			if (duplicateKeysPart != null && duplicateKeysPart.contains(keyOnly.getColumn(0).getValue())) {
				Row bufferRow = buffer.get(buffer.size()-1);
				bufferRow.getColumn(targetColumn.getName()).setValue("#duplicateKey");
			}
			
		}
		
		protected Row addBufferRow() {
			//add key column values;
			Row bufferRow = super.addBufferRow();
			bufferRow.getColumn(sourceKey.getName()).setValue(sourceKey.getValue());
			bufferRow.getColumn(targetKey.getName()).setValue(targetKey.getValue());
			return bufferRow;
		}
		
	}
	
	private boolean ignoreMissingColumns;
	private int keyColumnPos;
	private Modes compareMode;
	private References reference;
	private boolean subheaders;
	private static final Log log = new MessageHandler(LogFactory.getLog(TableCompare.class));
	
	public TableCompare() {
		super();
	}
	
	protected IProcessor getInputProcessor(int size, boolean isCached) throws RuntimeException {
		if (!isCached) {
			IProcessor sourceProcessor = null;
			switch (compareMode) {
			case key : sourceProcessor = new KeyCompareProcessor(getSourceManager().getProcessors()); break;
			default: sourceProcessor = new LineCompareProcessor(getSourceManager().getProcessors()); 
			}
			if (getColumnManager() == null) //calculate columns in runtime
				setColumnManager(sourceProcessor);
			IProcessor processor = new TransformInputProcessor(sourceProcessor,getFunctionManager(), getColumnManager(), getName());
			// processor.setName(getName());
			processor.setState(getState());
			processor.addFilter(getFilter());
			processor.setLastRow(size);
			return processor;
		}
		else return super.getProcessor(size);
	}

	public void init() throws InitializationException {
		try {
			super.init();
			ignoreMissingColumns = getParameter("ignoreMissingColumns","false").equalsIgnoreCase("true");
			subheaders = getParameter("subheaders","false").equalsIgnoreCase("true");
			keyColumnPos = Integer.parseInt(getParameter("keyColumn","1"))-1;
			compareMode = Modes.valueOf(getParameter("compareMode",Modes.line.toString()));
			reference = References.valueOf(getParameter("reference",References.source.toString()));
		}
		catch (Exception e) {
			throw new InitializationException("In transform " + getConfigurator().getName() + ": " + e.getMessage());
		}
	}
	
	public Row getOutputDescription() throws RuntimeException {
		switch (compareMode) {
		case key : return new KeyCompareProcessor(new ArrayList<IProcessor>()).getOutputDescription(); 
		default: return new  LineCompareProcessor(new ArrayList<IProcessor>()).getOutputDescription();
		}
	}

}
