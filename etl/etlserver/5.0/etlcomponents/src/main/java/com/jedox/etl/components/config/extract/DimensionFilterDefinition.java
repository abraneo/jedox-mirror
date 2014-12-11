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

	public class ConditionExtended extends Condition{
		private FilterModes mode;
		private String searchAttribute;
		private LogicalOperators logicalOperator;
		
		public FilterModes getFilterMode() {
			return mode;
		}
		public String getSearchAttribute() {
			return searchAttribute;
		}
		/**
		 * @return the logicalOperator
		 */
		public LogicalOperators getLogicalOperator() {
			return logicalOperator;
		}
	}

	public static enum FilterModes {
		rootToBases, rootToNodes, rootToConsolidates, nodesToBases, onlyBases, onlyNodes, onlyRoots
	}
	
	public static enum LogicalOperators {
		AND,OR
	}

	private String name = "default";
	private final String alias = "olap";
	

	public DimensionFilterDefinition(IContext context, Element config, Element scripts) throws ConfigurationException {
		super();
		init(context, config, scripts);
	}

	public DimensionFilterDefinition() throws ConfigurationException {
		super();
	}

	public List<Condition> getConditions() {
		ArrayList<Condition> result = new ArrayList<Condition>();
		for (Condition c : getCondition(alias)) {
			result.add(c);
		}
		return result;
	}

	public List<ConditionExtended> getConditionsExtended() {
		ArrayList<ConditionExtended> result = new ArrayList<ConditionExtended>();
		for (Condition c : getCondition(alias)) {
			result.add((ConditionExtended)c);
		}
		return result;
	}

	public String getName() {
		return name;
	}

	protected ConditionExtended createCondition() {
		return new ConditionExtended();
	}

	public ConditionExtended addCondition(String mode, IEvaluator evaluator ,FilterModes m,LogicalOperators op) {
		return addCondition(mode,evaluator,m,null,op);
	}
	
	public ConditionExtended addCondition(String mode, IEvaluator evaluator ,FilterModes m, String searchAttribute, LogicalOperators op) {
		ConditionExtended fad = (ConditionExtended) super.addCondition(mode, evaluator ,alias);
		fad.mode = m;
		fad.searchAttribute = searchAttribute;
		fad.logicalOperator = op;
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
			List<?> filters = dimension.getChildren();
			for (int j=0; j<filters.size();j++) {
				Element op = (Element) filters.get(j);
				Operators operator = Operators.valueOf(op.getAttributeValue("operator"));
				String value = op.getAttributeValue("value");
				LogicalOperators logicalOperator=null;
				if(op.getAttributeValue("logicaloperator")!=null){
					if(j==0){
						throw new ConfigurationException("Logical operators can not be used in the first filter.");
					}
					/*if(op.getName().equalsIgnoreCase("deny")){
						throw new ConfigurationException("Logical operators can only be used with accept filters.");
					}*/
					logicalOperator = LogicalOperators.valueOf(op.getAttributeValue("logicaloperator"));
				}
					
				String searchAttribute = op.getAttributeValue("attribute");
				if (searchAttribute!=null && searchAttribute.equals(NamingUtil.getElementnameElement()))
					searchAttribute=null;
				
				FilterModes mode = FilterModes.valueOf(op.getAttributeValue("mode",FilterModes.onlyNodes.toString()));
				
				IEvaluator evaluator = null;
				switch(operator){
					case equal:
						evaluator = new EqualityEvaluator(value);
						break;
					case like:
						evaluator = new RegexEvaluator(value);
						break;
/*						
					case between:  //obsolete
						evaluator = new RangeEvaluator(value);
						break;
					case alpha_range:  //obsolete
						evaluator = new AlphaRangeEvaluator(value);
						break;
*/						
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
/*							
						case subset:
							evaluator = new SubsetEvaluator(value);
							break;
*/						
					default:
						throw new ConfigurationException("Filter operator " + operator + " is not defined.");
				}
				addCondition(op.getName(), evaluator,mode,searchAttribute,logicalOperator);
			}
		}
	}
}
