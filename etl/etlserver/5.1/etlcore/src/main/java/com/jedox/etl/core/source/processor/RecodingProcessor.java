package com.jedox.etl.core.source.processor;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.Recoder;

public class RecodingProcessor extends Processor {
	
	private Row row = new Row();
	private Recoder recoder;
	
	public RecodingProcessor(IProcessor sourceProcessor, String sourceEncoding, String targetEncoding) {
		recoder = new Recoder();
		recoder.setRecoding(sourceEncoding, targetEncoding);
		setSourceProcessor(sourceProcessor); 
	}
	
	/*
	public String getLogDisplayType() {
		return "recoder";
	}
	*/

	@Override
	protected boolean fillRow(Row row) throws Exception {
		if (getSourceProcessor().next() != null) {
			if (recoder.isRecoding()) { //fill in in clone row. else no action necessary, since we pass through sourceprocessor row.
				for (int i=0; i<getSourceProcessor().current().size(); i++) {
					IColumn s = getSourceProcessor().current().getColumn(i);
					IColumn t = row.getColumn(i);
					t.setValue(recoder.recode(s.getValueAsString()));
				}
			}
			return true;
		}
		return false;
	}

	@Override
	protected Row getRow() {
		return row;
	}
	
	public void setFacet(Facets facet) {
		super.setFacet(facet);
		setLogInfo(false);
	}	

	@Override
	protected void init() throws RuntimeException {
		if (recoder.isRecoding()) {
			row = getSourceProcessor().current().clone();
		}
		else {
			row = getSourceProcessor().current();
		}
	}

}
