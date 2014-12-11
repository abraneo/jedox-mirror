package com.jedox.etl.components.config.extract;

import java.util.List;
import java.util.ArrayList;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.source.filter.AlphaRangeEvaluator;
import com.jedox.etl.core.source.filter.Conditions;
import com.jedox.etl.core.source.filter.EmptyEvaluator;
import com.jedox.etl.core.source.filter.EqualityEvaluator;
import com.jedox.etl.core.source.filter.IEvaluator;
import com.jedox.etl.core.source.filter.RangeEvaluator;
import com.jedox.etl.core.source.filter.RegexEvaluator;
import com.jedox.etl.core.source.filter.ScriptEvaluator;
import com.jedox.etl.core.source.filter.RowFilter.Operators;
import com.jedox.etl.core.util.NamingUtil;

public class DimensionFilterDefinition extends Conditions {
	
	//default is AND
	private LogicalOperators logicalOperator = LogicalOperators.AND;

	public class TreeCondition extends Condition{
		private FilterModes mode = FilterModes.onlyNodes;
		
		public FilterModes getFilterMode() {
			return mode;
		}
	}
	
	/**
	 * @return the logicalOperator
	 */
	public LogicalOperators getLogicalOperator() {
		return logicalOperator;
	}

	public static enum FilterModes {
		rootToBases, rootToNodes, rootToConsolidates, nodesToBases, onlyBases, onlyNodes, onlyRoots
	}
	
	public static enum LogicalOperators {
		AND,OR
	}

	private String name = "default";
	

	public DimensionFilterDefinition(IContext context, Element config, Element scripts) throws ConfigurationException {
		super();
		init(context, config, scripts);
	}

	public DimensionFilterDefinition() throws ConfigurationException {
		super();
	}


	public List<TreeCondition> getTreeConditions() {
		ArrayList<TreeCondition> result = new ArrayList<TreeCondition>();
		for (String field : getFieldNames()) {
			for(Condition condition:getCondition(field))
				result.add((TreeCondition)condition);
		}
		return result;
	}
	
	public List<TreeCondition> getTreeConditions(String field) {
		ArrayList<TreeCondition> result = new ArrayList<TreeCondition>();
		for(Condition condition:getCondition(field))
			result.add((TreeCondition)condition);
		return result;
	}

	public String getName() {
		return name;
	}

	protected TreeCondition createCondition() {
		return new TreeCondition();
	}


	
	public TreeCondition addTreeCondition(String mode, IEvaluator evaluator ,FilterModes m, String fieldName) {
		TreeCondition fad = (TreeCondition) super.addCondition(mode, evaluator ,fieldName);
		fad.mode = m;
		return fad;
	}
	
	protected String findScript(String name, Element scripts) throws ConfigurationException {
		if (scripts != null) {
			List<?> scriptElements = scripts.getChildren("script");
			for (int i=0; i<scriptElements.size(); i++) {
				Element scriptElement = (Element)scriptElements.get(i);
				if (scriptElement.getAttributeValue("name").equalsIgnoreCase(name)) {
					return scriptElement.getText();
				}
			}
		}
		throw new ConfigurationException("Script with name "+name+" is not found.");
	}

	protected void init(IContext context, Element dimension, Element scripts) throws ConfigurationException {
		if (dimension != null) {
			name = dimension.getAttributeValue("name");
			String searchColumn = NamingUtil.getElementnameElement();
			String logicaloperatorStr = dimension.getAttributeValue("type");
			if(logicaloperatorStr!=null)
				logicalOperator = LogicalOperators.valueOf(logicaloperatorStr);
			List<?> filters = dimension.getChildren();
			for (int j=0; j<filters.size();j++) {
				Element op = (Element) filters.get(j);
				Operators operator = Operators.valueOf(op.getAttributeValue("operator"));
				String value = op.getAttributeValue("value");
				
				String searchAttribute = op.getAttributeValue("attribute");
				if (searchAttribute!=null)
					searchColumn = searchAttribute;
				
				FilterModes mode = FilterModes.valueOf(op.getAttributeValue("mode",FilterModes.onlyNodes.toString()));
				
				IEvaluator evaluator = null;
				switch(operator){
					case equal:
						evaluator = new EqualityEvaluator(value);
						break;
					case like:
						evaluator = new RegexEvaluator(value);
						break;					
					case inRange:
						evaluator = new RangeEvaluator(value);
						break;
					case inAlphaRange:
						evaluator = new AlphaRangeEvaluator(value);
						break;
					case isEmpty:
						evaluator = new EmptyEvaluator(value);
						break;
					case script: 
						evaluator = new ScriptEvaluator(context,findScript(value,scripts),"groovy");
						break;					
					default:
						throw new ConfigurationException("Filter operator " + operator + " is not defined.");
				}
				addTreeCondition(op.getAttributeValue("type", op.getName()), evaluator,mode,searchColumn);
			}
		}
	}
}
