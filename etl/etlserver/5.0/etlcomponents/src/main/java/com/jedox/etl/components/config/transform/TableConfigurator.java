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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.transform;

import java.util.ArrayList;
import java.util.List;
import org.jdom.Element;
import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.source.FilterConfigurator;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.config.transform.ColumnConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.AnnexNode;
import com.jedox.etl.core.node.ValueNode;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.node.IColumn.ColumnTypes;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TableConfigurator extends TableSourceConfigurator implements ITransformConfigurator {

	private class OperationDefinition {
		public ValueNode.Operations operation = ValueNode.Operations.NONE;
		public String field;
	}

	private ColumnManager manager;
	//private static final Log log = LogFactory.getLog(TableConfigurator.class);
	private TransformConfigUtil util;

	public ColumnManager getColumnManager() {
		return manager;
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		return util.getFunctions();
	}

	public RowFilter getFilter() throws ConfigurationException {
		FilterConfigurator fc = new FilterConfigurator(getContext(), getParameter());
		return fc.getFilter(getXML().getChild("filter"));
	}

	private Element getTarget() {
		return getXML().getChild("target");
	}

	private void setCoordinate(ColumnManager manager, Element column, String defaultAggregate) throws ConfigurationException {
		ColumnConfigurator conf = new ColumnConfigurator(getName());
		String inputName = conf.getInputName(column);
		String name = conf.getColumnName(column,inputName);
		String value = conf.getInputValue(column);
		// String cdefault = column.getAttributeValue("default");
		// String caggregate = column.getAttributeValue("aggregate", defaultAggregate);
		if(manager.getColumn(name)!= null)
			throw new ConfigurationException("Coordinate \"" + name + "\" exists more than once.");
		CoordinateNode c = manager.addCoordinate(name,inputName);
		// c.setAggregateFunction(caggregate);
		c.setConstantValue(value);
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

	private void setMeasures(ColumnManager manager, Element measures) throws ConfigurationException {
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
			String vtype = measure.getAttributeValue("type",data);
			String vaggregate = measure.getAttributeValue("aggregate", aggregate);
			if(!vtype.equals("numeric") && !(vaggregate.equals("none") || vaggregate.equals("count") || vaggregate.equals("max") || vaggregate.equals("min") || vaggregate.equals("first") || vaggregate.equals("last"))){
				throw new ConfigurationException("Only aggregate of type \"count\", \"first\", \"last\", \"min\" or \"max\"  can be applied on measures of type \"text\".");
			}
			if(!vaggregate.equals("none")?isAggregated.add(true):isAggregated.add(false));

			// default value 0 for numeric and "" for text measures
			// no longer necessary, done by Column class based on type (String, Double)  
			// String vdefault = measure.getAttributeValue("default",(vtype.equals("text")) ? "" : "0");
			if(manager.getColumn(name)!= null)
				throw new ConfigurationException("Measure \"" + name + "\" exists more than once.");

			ValueNode v = manager.addValue(op.field, name, inputName);
			v.setConstantValue(value);
			v.setAggregateFunction(vaggregate);
			v.setElementType(vtype.equalsIgnoreCase("numeric") ? ElementType.ELEMENT_NUMERIC.toString() : ElementType.ELEMENT_STRING.toString());
			// v.setFallbackDefault(vdefault);
			v.setOperation(op.operation);
		}
		if(isAggregated.contains(true) && isAggregated.contains(false))
			throw new ConfigurationException("If at least one measure is aggregated, all measures should be aggregated.");
		//check if there is aggregation and annexes at the same time, which we do not allow.
		if (isAggregated.contains(true) && manager.getColumnsOfType(ColumnTypes.annex).size() > 0)
			throw new ConfigurationException("Usage of annexes and aggregation is mutually exclusive. Please choose either one or the other.");
		if (op.operation.equals(ValueNode.Operations.NORMALIZE)) {
			//see if normalizer column is already defined. Consider this an error.
			IColumn m = manager.getColumn(op.field);
			if (m == null) {
				manager.addCoordinate(op.field,op.field);
			}
			else {
				throw new ConfigurationException("Column '"+op.field+"' is already defined otherwise. Please choose a different name for the normalization column.");
			}
			//set normalized value name
			String valueName = measures.getAttributeValue("valuename",NamingUtil.internal("Value"));
			manager.getColumnsOfType(IColumn.ColumnTypes.value).setName(valueName);
		}
		//add denormalization input(s) as coordinate(s)
		if (op.operation.equals(ValueNode.Operations.DENORMALIZE)) {
			manager.addAnnex(NamingUtil.internal(op.field),op.field);
			for (IColumn v : manager.getColumnsOfType(ColumnTypes.value).getColumns()) {
				String inputName = ((ValueNode)v).getInputName();
				manager.addAnnex(NamingUtil.internal(inputName), inputName);
			}
		}
	}

	protected ColumnManager setColumns() throws ConfigurationException {
		ColumnManager manager = new ColumnManager();
		//process coordinate columns
		List<Element> coordinate = getChildren(getTarget(),"coordinate");
		for (Element column : coordinate)
			setCoordinate(manager,column,"none");
		Element annexes = getTarget().getChild("annexes");
		if(annexes != null){
			throw new ConfigurationException("Drillthrough configuration is no longer supportted in \"TableTransform\", the configuration is now completely in \"CubeLoad\"");
		}
		//for (Element column : annexes)
		//	setInfo(manager,column);
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
				manager = setColumns();
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
