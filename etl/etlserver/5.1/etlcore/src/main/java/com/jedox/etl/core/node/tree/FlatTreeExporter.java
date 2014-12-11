package com.jedox.etl.core.node.tree;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class FlatTreeExporter implements ITreeExporter {
	
	private IProcessor rows;
	private int bulkSize;
	private boolean withAttributes = false;
	private List<Attribute> attributes;
	private Row row = null;
	
	public FlatTreeExporter(IProcessor sourceProcessor) throws RuntimeException {			
		this.rows = sourceProcessor;
		row=rows.next();		
	}
		
	public IElement[] getNextBulk() throws RuntimeException{
		List<FlatElement> elements = new ArrayList<FlatElement>();
		int count=0;
		while (row!=null && count<bulkSize) {
			FlatElement elem = new FlatElement(row.getColumn(0).getValueAsString(), ElementType.ELEMENT_NUMERIC);			
			elements.add(elem); // don't check if element is existing due to performance
			
			if (withAttributes && row.size()>1) {
				HashMap<String,Object> attributes = new HashMap<String,Object>();								
				for (int i=1; i<row.size(); i++) {
					attributes.put(row.getColumn(i).getName(), row.getColumn(i).getValueAsString());
				}
				elem.setAttributes(attributes);				
			}			
			count++;
			row=rows.next();
		};		
		return elements.toArray(new FlatElement[elements.size()]);
	}
		
	public boolean hasNext() throws RuntimeException{
		return row!=null && row.size()>0;
	}
	
	public void reset() throws RuntimeException{
		rows.close();
		rows.initialize();
	}
		
	public void setWithAttributes(boolean withAttributes) {
		this.withAttributes = withAttributes;		
		if (withAttributes) {
			attributes = new ArrayList<Attribute>();								
			if (row!=null) {
				for (int i=1; i<row.size(); i++) {
					Attribute a = new Attribute(row.getColumn(i).getName(),ElementType.ELEMENT_STRING);
					attributes.add(a);				
				}
			}
		}	
	}
	
	public void setBulkSize(int bulkSize) {
		this.bulkSize=bulkSize;
	}
	
	public Attribute[] getAttributes() {
		if (withAttributes)
			return attributes.toArray(new Attribute[attributes.size()]);
		else 
			return null;
	}	
	
}
