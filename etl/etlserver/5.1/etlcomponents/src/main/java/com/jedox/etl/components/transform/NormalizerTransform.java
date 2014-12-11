package com.jedox.etl.components.transform;

import java.util.List;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.CoordinateProcessor;
import com.jedox.etl.core.transform.ITransform;

public class NormalizerTransform extends AggregationTransform implements ITransform {
	
	private class NormalizerProcessor extends CoordinateProcessor {
		
		private int valueSelect = 0;
		private Column value;
		private List<ValueNode> values;
		
		public NormalizerProcessor(IProcessor input, Row columns) {
			super(input,columns);
			this.setSourceProcessor(input);
			values = columns.getColumns(ValueNode.class);
		}
		
		protected void init() throws RuntimeException {
			super.init();
			if (values.size() > 0) {
				//determine name of value column
				value = new Column(values.get(0).getValueTarget());
				//determine data type of value column. Is of type double only, if all values are numeric
				Class<?> datatype = Double.class;
				for (ValueNode avalue : values) {
					if (!avalue.getValueType().equals(datatype)) {
						datatype = String.class;
						break;
					}
				}
				value.setValueType(datatype);
				getRow().addColumn(value);
			}
		}
		
		protected int getLength() {
			return super.getLength() + 1;
		}
		
		protected boolean fillValues(Row inputRow) throws RuntimeException {
			if (inputRow != null) {
				ValueNode vn = (ValueNode) values.get(valueSelect);
				//get value from input
				Object v = getSourceProcessor().current().getColumn(super.getLength()+valueSelect).getValue();
				value.setValue(v);
				//write the value's name to target to provide the index.
				if (vn.getTarget() != null) {
					IColumn target = getRow().getColumn(vn.getTarget());
					if (target == null) throw new RuntimeException("Normalization column "+vn.getTarget()+" does not exists in input.");
					target.setValue(vn.getName());
				}
				valueSelect = (valueSelect + 1) % values.size();
				boolean x = !(value.isEmpty() || value.getValueAsString().equals("0"));
				return x;
			}
			return true;
		}
		
		protected boolean fillRow(Row row) throws Exception {
			if (values.size() > 0) {
				boolean hasData = true;
				if (valueSelect == 0) {
					hasData = super.fillRow(row);
				}
				if (!hasData)
					return false;
				//fill value
				
				if (ignoreEmpty) {
					do  {
						boolean foundNonEmpty=false;
						do {
							foundNonEmpty = fillValues(row);
						} while (valueSelect != 0 && !foundNonEmpty);
						if (foundNonEmpty)
							return true;
						hasData = super.fillRow(row);				
					} while (hasData);
					return false;
				}
				else {
					fillValues(row);
					return true;
				}
			
			}
			else 
				return super.fillRow(row);
		}
		
		protected String getFinishedText() {
			return "Lines normalized from "+getLogDisplayType()+" "+getName()+": "+getRowsAccepted();
		}

	}
	
	private boolean ignoreEmpty;
	
	protected IProcessor getOutputProcessor(IProcessor input, int size) throws RuntimeException {
		IProcessor out = initProcessor(new NormalizerProcessor(input, getRow()),Facets.OUTPUT);
		out.setLastRow(size);
		return out;
	}
	
	public Row getOutputDescription() throws RuntimeException {
		return initProcessor(new NormalizerProcessor(null, getRow()),Facets.OUTPUT).getOutputDescription();
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			ignoreEmpty = Boolean.valueOf(getParameter("ignoreEmpty","false"));
		} catch (ConfigurationException e) {
			throw new InitializationException(e);
		}
		if (ignoreEmpty)
			log.info("Ignore empty option set in transform "+getName());
	}

}
