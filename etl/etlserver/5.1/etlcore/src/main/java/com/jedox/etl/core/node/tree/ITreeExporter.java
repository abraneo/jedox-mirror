package com.jedox.etl.core.node.tree;


import com.jedox.etl.core.component.RuntimeException;
import com.jedox.palojlib.interfaces.IElement;


public interface ITreeExporter {

	public IElement[] getNextBulk() throws RuntimeException;
	public boolean hasNext() throws RuntimeException;
	public void reset() throws RuntimeException;
	public void setWithAttributes(boolean withAttributes);
	public void setBulkSize(int bulkSize);	
	public Attribute[] getAttributes();
}
