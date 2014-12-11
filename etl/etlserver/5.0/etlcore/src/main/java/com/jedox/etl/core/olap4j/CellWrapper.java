package com.jedox.etl.core.olap4j;

import com.jedox.palojlib.interfaces.ICell;

public class CellWrapper implements ICell {
	
	private String[] pathnames;
	private Object value;
	
	//private IDimension[] dimensions;
	
	public CellWrapper(Object value) {
		this.value = value;
	}

	@Override
	public String[] getPathNames() {
	    return pathnames;
	}

	@Override
	public Object getValue() {
		return value;
	}
	
	public void setPathNames(String[] pathnames) {
		this.pathnames = pathnames;
	}

	@Override
	public CellType getType() {
		return CellType.CELL_NUMERIC;
	}

}
