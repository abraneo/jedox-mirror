/**
*   @brief <Description of Class>
*
*   @file
*
*   Copyright (C) 2008-2013 Jedox AG
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License (Version 2) as published
*   by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
*
*   This program is distributed in the hope that it will be useful, but WITHOUT
*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*   more details.
*
*   You should have received a copy of the GNU General Public License along with
*   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
*   Place, Suite 330, Boston, MA 02111-1307 USA
*
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible
*   OEM Commercial License.
*
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right
*   (commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.transform;

import java.util.ArrayList;
import java.util.List;
import org.jdom.Element;
import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.config.transform.ColumnConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.util.NamingUtil;

public class TableTransformConfigurator extends TableSourceConfigurator implements ITransformConfigurator {

	private class OperationDefinition {
		public ValueNode.Operations operation = ValueNode.Operations.NONE;
		public String field;
	}

	private Row row;
	//private static final Log log = LogFactory.getLog(TableConfigurator.class);
	private TransformConfigUtil util;

	public Row getRow() {
		return row;
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		return util.getFunctions();
	}


	private Element getTarget() {
		return getXML().getChild("target");
	}

	private void setCoordinate(Row manager, Element column, String defaultAggregate) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(column);
		String name = conf.getColumnName(column,inputName);
		String value = conf.getInputValue(column);
		// Skip coordinates with name #skipColumn 
		if (name.equals(NamingUtil.skipColumn()))			
			return;		
		if(manager.getColumn(name)!= null)
			throw new ConfigurationException("Coordinate \"" + name + "\" exists more than once.");
		CoordinateNode c = ColumnNodeFactory.getInstance().createCoordinateNode(name,new Column(inputName));
		// c.setAggregateFunction(caggregate);
		if (value != null) c.setValue(value);
		manager.addColumn(c);
		// c.setFallbackDefault(cdefault);
	}

	private OperationDefinition getOperationDefinition(Element element) throws ConfigurationException {
		OperationDefinition op = new OperationDefinition();
		String fieldNorm = element.getAttributeValue("normalize");
		String fieldDenorm = element.getAttributeValue("denormalize");
		if (fieldNorm != null && fieldDenorm != null)
			throw new ConfigurationException("Not allowed to normalize and denormalize at the same time");
		if (fieldNorm != null) {
			if(fieldNorm.isEmpty()){throw new ConfigurationException("Normalization field can not be empty");}
			op.operation = ValueNode.Operations.NORMALIZE;
			op.field = fieldNorm;
		} else if (fieldDenorm != null) {
			if(fieldDenorm.isEmpty()){throw new ConfigurationException("Denormalization field can not be empty");}
			op.operation = ValueNode.Operations.DENORMALIZE;
			op.field = fieldDenorm;
		}
		return op;
	}

	private void setMeasures(Row manager, Element measures) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		OperationDefinition op = getOperationDefinition(measures);
		//String measureName = conf.getColumnName(measures,null);
		List<Element> measureList = getChildren(measures,"measure");
		if (op!=null && measureList.isEmpty())
			throw new ConfigurationException("At least one measure has to be defined for Normalisation");
		String defaultAggregate = getParameter("aggregate","none");
		/*
		if (op.operation.equals(ValueNode.Operations.DENORMALIZE))
			defaultAggregate = "sum";
		*/
		String aggregate = measures.getAttributeValue("aggregate",defaultAggregate);
		//get measure type
		String data = measures.getAttributeValue("type","numeric");
		ArrayList<Boolean> isAggregated = new ArrayList<Boolean>();
		for (Element measure : measureList) {
			String inputName = conf.getInputName(measure);
			String name = conf.getColumnName(measure,inputName);
			String value = conf.getInputValue(measure);
			//type and aggregation may be overruled per measure
			String vaggregate = measure.getAttributeValue("aggregate", aggregate);
			//if (vaggregate.equalsIgnoreCase("var") || vaggregate.equalsIgnoreCase("stddev")) vaggregate += "_samp";
			String vtype = measure.getAttributeValue("type",vaggregate.equals("group_concat") ? "text" : data);
			if(!vtype.equals("numeric") && !(vaggregate.equals("none") || vaggregate.equals("count") || vaggregate.equals("max") || vaggregate.equals("min") || vaggregate.equals("first") || vaggregate.equals("last") || vaggregate.equals("group_concat") || vaggregate.equals("selectivity"))){
				throw new ConfigurationException("Only aggregate of type \"count\", \"first\", \"last\", \"min\", \"max\", \"group_concat\" or \"selectivity\" can be applied on measures of type \"text\".");
			}
			if(!vaggregate.equals("none")?isAggregated.add(true):isAggregated.add(false));

			// default value 0 for numeric and "" for text measures
			// no longer necessary, done by Column class based on type (String, Double)  
			// Skip measures with name #skipColumn
			if (name.equals(NamingUtil.skipColumn()))			
				continue;		
			if(manager.getColumn(name)!= null)
				throw new ConfigurationException("Measure \"" + name + "\" exists more than once.");

			ValueNode v = ColumnNodeFactory.getInstance().createValueNode(op.field, name, new Column(inputName));
			if (value != null) v.setValue(value);
			v.setAggregateFunction(vaggregate);
			v.setValueType(vtype.equalsIgnoreCase("numeric") ? Double.class : String.class);
			// v.setFallbackDefault(vdefault);
			v.setOperation(op.operation);
			manager.addColumn(v);
		}
		if(isAggregated.contains(true) && isAggregated.contains(false))
			throw new ConfigurationException("If at least one measure is aggregated, all measures should be aggregated.");
		if (op.operation.equals(ValueNode.Operations.NORMALIZE)) {
			//see if normalizer column is already defined. Consider this an error.
			IColumn m = manager.getColumn(op.field);
			if (m == null) {
				manager.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(op.field, new Column(op.field)));
			}
			else {
				throw new ConfigurationException("Column '"+op.field+"' is already defined otherwise. Please choose a different name for the normalization column.");
			}
			//set normalized value name
			String valueName = measures.getAttributeValue("valuename",NamingUtil.internal("Value"));
			for (ValueNode v : manager.getColumns(ValueNode.class)) {
				v.setValueTarget(valueName);
			}
		}
		//add denormalization input(s) as attributes(s)
		if (op.operation.equals(ValueNode.Operations.DENORMALIZE)) {
			manager.addColumn(ColumnNodeFactory.getInstance().createAttributeNode(new Attribute(NamingUtil.internal(op.field)), new Column(op.field)));
			for (ValueNode v : manager.getColumns(ValueNode.class)) {
				String inputName = v.getInputName();
				manager.addColumn(ColumnNodeFactory.getInstance().createAttributeNode(new Attribute(NamingUtil.internal(inputName)), new Column(inputName)));
			}
		}
	}

	protected Row setColumns() throws ConfigurationException {
		Row manager = new Row();
		//process coordinate columns
		List<Element> coordinate = getChildren(getTarget(),"coordinate");
		for (Element column : coordinate)
			setCoordinate(manager,column,"none");
		Element measures = getTarget().getChild("measures");
		if (measures != null)
			setMeasures(manager,measures);
		return manager;
	}

	public void configure() throws ConfigurationException {
		try {
			setName(getXML().getAttributeValue("name"));
			util = new TransformConfigUtil(getXML(),getLocator(),getContext());
			if (getTarget() != null) { //set column according to target
				row = setColumns();
			}
			/*
			else { //no target defined. enable passthrough
				manager = util.getColumns();
			}
			//only build aliases for coordinates and values. ignore infos, since they may be removed and never should be addressed by design
			Row aliases = new Row();
			aliases.addColumns(manager.getColumnsOfType(ColumnTypes.coordinate));
			aliases.addColumns(manager.getColumnsOfType(ColumnTypes.value));
			setAliasMap(AliasMap.build(aliases.getOutputDescription()));
			*/
			setAliasMap(new AliasMap());
		}
		catch (Exception e) {
			throw new ConfigurationException("Failed to configure transform "+getName()+": "+e.getMessage());
		}
	}
}
