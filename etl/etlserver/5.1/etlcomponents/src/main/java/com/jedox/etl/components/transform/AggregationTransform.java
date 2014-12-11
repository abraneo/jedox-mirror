package com.jedox.etl.components.transform;

import java.util.ArrayList;
import java.util.List;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformInputProcessor;
import com.jedox.etl.core.util.SQLUtil;

public class AggregationTransform extends TableTransform implements ITransform {
	
	public String getAggregateName(ValueNode coordinate) {
		if (coordinate.getAggregateFunction().equalsIgnoreCase("none")) return escapeName(coordinate.getName());
		return coordinate.getAggregateFunction()+"("+(coordinate.isDistinctFunction()?"DISTINCT ":"")+escapeName(coordinate.getName())+") as "+escapeName(coordinate.getName());
	}

	protected String getFields() {
		List<String> fields = new ArrayList<String>();
		for (CoordinateNode c : getCoordinates()) {
			fields.add(escapeName(c.getName()));
		}
		for (ValueNode c : getValues()) {
			fields.add(getAggregateName(c));
		}
		return SQLUtil.enumNames(fields);
	}

	protected String getGroupBy(String fields) {
		List<String> group = new ArrayList<String>();
		for (CoordinateNode coordinate : getCoordinates()) {
			group.add(escapeName(coordinate.getName()));
		}
		for (ValueNode coordinate : getValues()) {
			if (!coordinate.hasAggregateFunction())
				group.add(escapeName(coordinate.getName()));
		}
		String result = SQLUtil.enumNames(group);
		if (result.equals(fields))
			return "";
		else
			return result;
	}
	
	/**
	 * needed to comply to ISource interface
	 */
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor in = super.getSourceProcessor(0); //we need all input rows for aggregation. size has to be applied on output
		String fields = getFields();
		String groupBy = getGroupBy(fields);
		String query = SQLUtil.buildQuery(getLocator().getPersistentName(),fields,"",groupBy,"");
		//manipulate internal query
		setQueryInternal(query);
		return in;
	}
	

	@Override
	protected IProcessor getInputProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(new TransformInputProcessor(getInputUnion(),getFunctionManager(), getRow()), Facets.INPUT);
		return processor;
	}

	@Override
	protected IProcessor getOutputProcessor(IProcessor input, int size) throws RuntimeException {
		//aggregation is done internally by table source calling getSourceProcessor
		return input;
	}

}
