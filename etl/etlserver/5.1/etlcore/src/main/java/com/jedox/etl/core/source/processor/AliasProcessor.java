package com.jedox.etl.core.source.processor;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

public class AliasProcessor extends Processor {
	
	private Row row;
	private IAliasMap map;
	
	public AliasProcessor(IProcessor source, IAliasMap map) {
		setSourceProcessor(source);
		this.map = map;
	}

	@Override
	protected boolean fillRow(Row row) throws Exception {
		return getSourceProcessor().next() != null;
	}

	@Override
	protected Row getRow() throws RuntimeException {
		return row;
	}

	@Override
	protected void init() throws RuntimeException {
		row = new Row();
		for (IColumn c : getSourceProcessor().current().getColumns()) {
			row.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(), c));
		}
		row.setAliases(map);
	}

}
