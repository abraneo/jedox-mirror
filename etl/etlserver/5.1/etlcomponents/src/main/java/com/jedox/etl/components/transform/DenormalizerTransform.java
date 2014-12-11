package com.jedox.etl.components.transform;

import java.util.HashSet;
import java.util.List;

import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.CoordinateProcessor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;


public class DenormalizerTransform extends AggregationTransform implements ITransform {
	
	private class DenormalizerProcessor extends CoordinateProcessor {

		private List<ValueNode> values;
		
		private HashSet<String> foundColumns = new HashSet<String>();
		
		public DenormalizerProcessor(IProcessor input, Row columns) {
			super(input, columns);
			if (input!=null)
				input.setFacet(Facets.HIDDEN);
			values = columns.getColumns(ValueNode.class);
		}
		
		protected void init() throws RuntimeException {
			super.init();
			for (ValueNode avalue : values) {
				Column c = new Column();
				c.mimic(avalue);
				getRow().addColumn(c);
				/*CSCHW TODO check this
				//remove denormalization coordinate and input coordinate
				removeInfo(NamingUtil.internal(v.getTarget()));
				removeInfo(NamingUtil.internal(v.getInputName()));
				*/
			}
		}
		
		protected int getLength() {
			return super.getLength() + values.size();
		}
		
		protected void fillValues(Row inputRow) throws RuntimeException {
			if (inputRow != null) {
				for (int i=0; i<values.size(); i++) {
					ValueNode v = values.get(i);
					//note: target and input are only in inputProcessor, not in current processor row, since we created them as AttributeNodes, not as CoordinateNodes
					IColumn target = getSourceProcessor().current().getColumn(NamingUtil.internal(v.getTarget()));
					IColumn value = getRow().getColumn(v.getName());
					if (target.getValue().equals(v.getName())) {
						IColumn input = getSourceProcessor().current().getColumn(NamingUtil.internal(v.getInputName()));
						value.setValue(input.getValue());
						foundColumns.add(v.getName());
					} 
					else {
						if (v.getValueType().equals(Double.class))
							value.setValue(0);
						else if (v.getValueType().equals(String.class)) {
							value.setValue("");
//							value.setDefaultValue("");
						}	
						else
							value.setValue(null);
					}	
				}
			}
		}
		
		protected boolean fillRow(Row row) throws Exception {
			if (values.size() > 0) {
				boolean hasData = super.fillRow(row);
				//fill value
				if (hasData) {
					fillValues(row);
				}
				return hasData;
			}
			else 
				return super.fillRow(row);
		}
		
		public void close() {
			super.close();
			// Check if all 
			for (int i=0; i<values.size(); i++) {
				ValueNode v = values.get(i);;
				if (!foundColumns.contains(v.getName())) {
					log.warn("In transform "+getName()+" the measure "+v.getName()+" has not been found in source column "+v.getTarget()+".");
				}		
			}	
		}	
		
		protected String getFinishedText() {
			return "Lines denormalized from "+getLogDisplayType()+" "+getName()+": "+getRowsAccepted();
		}

	}
	
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor in = super.getSourceProcessor(size);
		IProcessor out = initProcessor(new DenormalizerProcessor(in,getRow()),Facets.HIDDEN);
		return out;
	}
	
	public IProcessor getOutputProcessor(IProcessor input, int size) throws RuntimeException {
		if(!isCached()){
			IProcessor out = initProcessor(new DenormalizerProcessor(input, getRow()),Facets.OUTPUT);
			out.setLastRow(size);
			return out;
		}else
			return super.getOutputProcessor(input, size);
	
	}
	
	
	public Row getOutputDescription() throws RuntimeException {
		return initProcessor(new DenormalizerProcessor(null, getRow()),Facets.OUTPUT).getOutputDescription();
	}
	
	public void init() throws InitializationException {
		super.init();
		//setCaching(false); //force caching, since denormalization will return wrong results else.
	}

}
