package com.jedox.etl.core.olap4j;

import java.util.ArrayList;
import java.util.List;

import com.jedox.palojlib.interfaces.ICellExportContext;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.main.CellExportContext;

public class ExtendedCellExportContext extends CellExportContext implements ICellExportContext {
	
	private List<String> slicerDimensions = new ArrayList<String>();
	
	public ExtendedCellExportContext(CellsExportType type, int blockSize,
			boolean useRules, boolean onlyBases, boolean skipEmpty) {
		super(type, blockSize, useRules, onlyBases, skipEmpty);
	}

	public void setSlicerDimensions(List<String> slicerDimensions) {
		this.slicerDimensions = slicerDimensions;
	}

	public List<String> getSlicerDimensions() {
		return slicerDimensions;
	}
	
	public void addSlicerDimension(String dimension) {
		slicerDimensions.add(dimension);
	}

	
}
