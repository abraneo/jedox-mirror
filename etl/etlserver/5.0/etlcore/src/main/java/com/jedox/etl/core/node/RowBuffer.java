package com.jedox.etl.core.node;

import java.util.ArrayList;
import java.util.List;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;

/**
 * DRAFT FOR FUTURE EXTENSION - NOT IN USE NOW
 * @author chris
 *
 */
public class RowBuffer extends Row {
	
	public enum Aggregates {
		sum, min, max, avg, count, countdistinct, first, last
	}
	
	private class BufferProcessor extends Processor {
		
		private int processorIndex = 0;

		@Override
		protected boolean fillRow(Row row) throws Exception {
			if (processorIndex < RowBuffer.this.buffer.size()) {
				RowBuffer.this.moveToIndex(processorIndex);
				processorIndex++;
				return true;
			}
			else return false;
		}

		@Override
		protected Row getRow() {
			return RowBuffer.this;
		}
		
		public void close() {
			super.close();
			RowBuffer.this.buffer.clear();
			RowBuffer.this.currentIndex = -1;
		}	
	}
	
	private List<List<Object>> buffer = new ArrayList<List<Object>>();
	private int currentIndex = -1;
	
	private void addToBuffer(Row row) {
		List<Object> values = new ArrayList<Object>();
		for (IColumn c : this.getColumns()) {
			IColumn newColumn = row.getColumn(c.getName());
			if (newColumn != null) {
				values.add(newColumn.getValue());
			}
			else {
				values.add(null);
			}
		}
		buffer.add(values);
	}
	
	private void getValuesFromBuffer(int index) throws RuntimeException {
		if (index < buffer.size() && index >= 0) {
			currentIndex = index;
			List<Object> values = buffer.get(index);
			for (int i=0; i<values.size(); i++) {
				this.getColumn(i).setValue(values.get(i));
			}
		}
		else throw new RuntimeException("Index out of bounds: "+index);
	}
	
	private void removeValuesFromBuffer(int index) throws RuntimeException {
		if (index < buffer.size() && index >= 0) {
			buffer.remove(index);
		}
		else throw new RuntimeException("Index out of bounds: "+index);
	}
	
	public void addRow(Row row) {
		addToBuffer(row);
	}
	
	public void addValues(List<List<Object>> values) {
		buffer.addAll(values);
	}
	
	public void removeFirst() throws RuntimeException {
		removeValuesFromBuffer(0);
	}
	
	public void removeLast() throws RuntimeException {
		removeValuesFromBuffer(buffer.size()-1);
	}
	
	public void removeCurrent() throws RuntimeException {
		removeValuesFromBuffer(currentIndex);
	}
	
	public void moveToFirst() throws RuntimeException {
		getValuesFromBuffer(0);
	}
	
	public void moveToLast() throws RuntimeException {
		getValuesFromBuffer(buffer.size()-1);
	}
	
	public void moveToNext() throws RuntimeException {
		getValuesFromBuffer(currentIndex+1);
	}
	
	public void moveToPrev() throws RuntimeException {
		getValuesFromBuffer(currentIndex-1);
	}

	public void moveToIndex(int index) throws RuntimeException {
		getValuesFromBuffer(index);
	}
	
	public int getBufferSize() {
		return buffer.size();
	}
	
	public boolean hasNext() {
		return currentIndex < buffer.size();
	}
	
	public boolean hasPrev() {
		return currentIndex >= 0;
	}
	
	public void aggregate(Aggregates aggregate) {
		
	}
	
	public IProcessor getProcessor(int size) {
		IProcessor result = new BufferProcessor();
		result.setLastRow(size);
		return result;
	}
	

}
