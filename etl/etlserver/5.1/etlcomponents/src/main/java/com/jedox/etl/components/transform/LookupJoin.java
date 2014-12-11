package com.jedox.etl.components.transform;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.TableJoinConfigurator.JoinDefinition;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.Key;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.Match;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;

public class LookupJoin extends Processor {
	
	private JoinDefinition joinDefinition;
	private SourceManager sourceManager;
	
	private IComponent owner;
	private static final Log log = LogFactory.getLog(LookupJoin.class);
		
	private Row row;
	private List<List<Object>> buffer = new ArrayList<List<Object>>();
	HashMap<String, List<List<Object>>> lookup = new HashMap<String,List<List<Object>>>();
	private List<Key> leftKeyList;
	private List<Key> rightKeyList;
	private Row rightDataRow;
	private IProcessor leftProcessor;
	private IProcessor rightProcessor;
	
	public LookupJoin(IComponent owner, JoinDefinition joinDefinition, SourceManager sourceManager) throws RuntimeException {
		this.owner = owner;
		this.joinDefinition = joinDefinition;
		this.sourceManager = sourceManager;
		ISource leftSource = sourceManager.get(joinDefinition.getLeftSource());
		leftProcessor = owner.initProcessor(leftSource.getProcessor(),Facets.INPUT);
		setSourceProcessor(leftProcessor);
	}

	@Override
	protected boolean fillRow(Row row) throws Exception {
		while (buffer.isEmpty()) {
			Row leftRow = getSourceProcessor().next();
			if (leftRow == null)
				return false;
			List<List<Object>> result = lookup.get(concatKeyValues(getKeyRow(leftKeyList,leftRow,joinDefinition.getLeftSource())));
			if (result != null) {
				buffer.addAll(result);
			}
			else if (!joinDefinition.getType().equalsIgnoreCase("inner")) { //no lookup available but outer join. set lookup values to null
				for (IColumn c : rightDataRow.getColumns()) {
					row.getColumn(c.getName()).setValue(null);
				}
				return true;
			}
		}
		
			List<Object> rightValues = buffer.remove(0);
			for (int i = 0; i < rightDataRow.size(); i++) {
				IColumn c = rightDataRow.getColumn(i);
				IColumn target = row.getColumn(c.getName());
				target.setValue(rightValues.get(i));
			}
			return true;

	}

	@Override
	protected Row getRow() {
		return row;
	}
	
	public void close() {
		super.close();
		lookup.clear();
	}
	
	@Override
	protected void init() throws RuntimeException {
		List<Match> matches = joinDefinition.getMatches();
		rightKeyList = new ArrayList<Key>();
		for (Match m : matches) {
			rightKeyList.add(m.getRightKey());
		}
		leftKeyList = new ArrayList<Key>();
		for (Match m : joinDefinition.getMatches()) {
			leftKeyList.add(m.getLeftKey());
		}
		ISource rightSource = sourceManager.get(joinDefinition.getRightSource());
//		rightProcessor = owner.initProcessor(new IdProcessor(rightSource.getProcessor()),Facets.INPUT);
		rightProcessor = owner.initProcessor(rightSource.getProcessor(),Facets.INPUT);
		
		rightDataRow = new Row();
		rightDataRow.addColumns(rightProcessor.current());
		//detect conflicting columns
		Row conflictingColumns = new Row();
		for (IColumn c : rightDataRow.getColumns()) {
			if (leftProcessor.current().containsColumn(c.getName())) { 
				conflictingColumns.addColumn(c);
			}
		}
		//remove conflicting columns from right row
		for (IColumn c : conflictingColumns.getColumns()) {
			rightDataRow.removeColumn(c.getName());
			boolean isKey = false;
			for (Key k : rightKeyList) {
				if (k.getName().equals(c.getName())) {
					isKey = true;
					break;
				}
			}
			if (!isKey)
				log.warn("Removing column "+c.getName()+" of source "+rightProcessor.getName()+" from joint result, because source "+leftProcessor.getName()+" already contains column with identical name.");
		}
		//build lookup
		Row rightRow = rightProcessor.next();
		Row rightKeyRow = getKeyRow(rightKeyList,rightProcessor.current(),joinDefinition.getRightSource());
		while (rightRow != null) {		
			String key = concatKeyValues(rightKeyRow);
			List<List<Object>> data = lookup.get(key);
			if (data == null) {
				data = new ArrayList<List<Object>>();
				lookup.put(key, data);
			}
			//get Column values
			List<Object> values = new ArrayList<Object>();
			for (IColumn c : rightDataRow.getColumns()) {
				values.add(c.getValue());
			}
			data.add(values);
			rightRow = rightProcessor.next();
		}
		row = new Row();
		for (IColumn c : leftProcessor.current().getColumns()) {
			CoordinateNode cn = ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(),c);
			row.addColumn(cn);
		}
		for (IColumn c : rightDataRow.getColumns()) {
			CoordinateNode cn = ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(),c);
			row.addColumn(cn);
		}
	}
	
	public long getOwnProcessingTime() {
		return super.getOwnProcessingTime() - (rightProcessor==null ? 0 : rightProcessor.getOverallProcessingTime());
	}
	
	private String concatKeyValues(Row row) {
		StringBuffer buf = new StringBuffer();
		for (IColumn c : row.getColumns()) {
			buf.append("@");
			buf.append(c.getValueAsString());
		}
		return buf.toString();
	}
	
	private Row getKeyRow(List<Key> keys, Row allColumns, String sourceName) throws RuntimeException {
		Row result = new Row();
		for (Key key : keys) {
			if (!key.isConstant()) {
				IColumn keyColumn = allColumns.getColumn(key.getName());
				if (keyColumn == null)
					throw new RuntimeException("Key "+key.getName()+" is not part of source "+sourceName);
				result.addColumn(keyColumn);
			} else {
				Column c = new Column(key.getName());
				c.setValue(key.getName());
				result.addColumn(c);
			}
		}
		return result;
	}

}
