package com.jedox.etl.components.transform;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.TableJoinConfigurator.Conditions;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.JoinDefinition;
import com.jedox.etl.components.config.transform.TableJoinConfigurator.Match;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.SQLUtil;

public class PersistentJoin {
	
	private JoinDefinition joinDefinition; 
	private SourceManager sourceManager;
	private String escapeCharacter;
	private static final Log log = LogFactory.getLog(PersistentJoin.class);
	
	public PersistentJoin(JoinDefinition joinDefinition, SourceManager sourceManager, String escapeCharacter) {
		this.joinDefinition = joinDefinition;
		this.sourceManager = sourceManager;
		this.escapeCharacter = escapeCharacter;
	}

	/**
	 * Sets up an underlying datasource. Ensures that it is cached.
	 * @param datasource
	 * @return the SQL-name of the join alias
	 */
	private Row setupSource(ISource datasource) throws RuntimeException {
		try {
			//ensure that datasources are cached and accessible
			datasource.setCaching(true);
			IProcessor processor = datasource.getProcessor();
			Row current = processor.current();
			if (datasource instanceof ITreeSource) {
				Row next = processor.next();
				return (next == null) ? current : next;
			}
			else return current;
		} catch (Exception sqle) {
			throw new RuntimeException("Failed to setup internal join table for source "+datasource.getName()+": "+sqle.getMessage());
		}
	}

	private String resolveKey(String sourcename, String keyname, boolean constant) {
		if (!constant) {
			return getAlias(sourcename)+"."+SQLUtil.quoteName(keyname,escapeCharacter);
		} else {
			return "'"+keyname+"'";
		}
	}
	
	private String getCast(String name, boolean constant) {
		return (!constant) ? "TRIM(CAST(CAST("+name+ " AS CHAR(100)) AS VARCHAR(100))) " : name;
	}
	
	private String getJoinConditionExpression(JoinDefinition def) {
		StringBuffer result = new StringBuffer();
		int keys = 0;
		for (Match m: def.getMatches()) {
			if (keys > 0) result.append(" AND ");
			if (m.getCondition().equals(Conditions.EQ) || m.getCondition().equals(Conditions.NE)) {
				result.append(getCast(resolveKey(m.getLeftSource(), m.getLeftKey().getName(), m.getLeftKey().isConstant()),m.getLeftKey().isConstant()) +m.getConditionOperator());
				result.append(getCast(resolveKey(m.getRightSource(), m.getRightKey().getName(), m.getRightKey().isConstant()),m.getRightKey().isConstant()));
			} else {
				result.append(resolveKey(m.getLeftSource(), m.getLeftKey().getName(), m.getLeftKey().isConstant()) +" "+m.getConditionOperator());
				result.append(resolveKey(m.getRightSource(), m.getRightKey().getName(), m.getRightKey().isConstant()));
			}
			keys++;
		}
		return result.toString();
	}

	private String getAlias(String name) {
		String side;
		if (name.equals(joinDefinition.getLeftSource()))
			side = "l";
		else
			side = "r";
		return "s"+side;
	}

	private String getJoinExpression() {
		StringBuffer result = new StringBuffer();
		//append initial source
		ISource baseSource = sourceManager.get(joinDefinition.getLeftSource());
		result.append(baseSource.getLocator().getPersistentName()+" "+getAlias(joinDefinition.getLeftSource()));
		result.append(" "+joinDefinition.getType()+" join ");
		result.append(sourceManager.get(joinDefinition.getRightSource()).getLocator().getPersistentName()+" "+getAlias(joinDefinition.getRightSource()));
		if (joinDefinition.getMatches().size() > 0) {
			result.append(" on ");
			result.append(getJoinConditionExpression(joinDefinition));
		}
		return result.toString();
	}

	private String getAliasForOutput(HashMap<String, Row> lookup, String name) {
		if (joinDefinition.getType().contains("inner") || joinDefinition.getType().contains("left outer")) {
			if (lookup.get(joinDefinition.getLeftSource()).containsColumn(name))
				return getAlias(joinDefinition.getLeftSource());
			else 
				return getAlias(joinDefinition.getRightSource());
		}
		else {//right outer join
			if (lookup.get(joinDefinition.getRightSource()).containsColumn(name))
				return getAlias(joinDefinition.getRightSource());
			else 
				return getAlias(joinDefinition.getLeftSource());
		}
	}
	
	private boolean isIdenticalMatch(String name) {
		for (Match m : joinDefinition.getMatches()) {
			if (name.equalsIgnoreCase(m.getLeftKey().getName()) && name.equalsIgnoreCase(m.getRightKey().getName()) && !m.getLeftKey().isConstant() && !m.getRightKey().isConstant()) {
				return true;
			}
		}
		return false;
	}
	
	public String getJoinQuery() throws RuntimeException {
		HashMap<String, Row> lookup = new HashMap<String, Row>();
		ColumnManager columns = new ColumnManager();
		//get sources and get join fields
		ISource left = sourceManager.get(joinDefinition.getLeftSource());
		Row leftRow = setupSource(left);
		lookup.put(left.getName(),leftRow);
		columns.addCoordinates(leftRow);
		ISource right = sourceManager.get(joinDefinition.getRightSource());
		Row rightRow = setupSource(right);
		lookup.put(right.getName(),rightRow);
		for (IColumn c : rightRow.getColumns()) {
			//removal is implicit, because duplicate columns are not added below
			if (columns.containsColumn(c.getName()) && !isIdenticalMatch(c.getName())) 
				log.warn("Removing column "+c.getName()+" of source "+right.getName()+" from joint result, because source "+left.getName()+" already contains column with identical name.");
		}
		columns.addCoordinates(rightRow);
		List<String> fields = new ArrayList<String>();
		for (IColumn c : columns.getColumns()) {
			String prefix = getAliasForOutput(lookup,c.getName());
			fields.add(SQLUtil.prefixName(SQLUtil.quoteName(c.getName(),escapeCharacter),prefix));
		}
		String query = SQLUtil.buildQuery(getJoinExpression(),SQLUtil.enumNames(fields),"","","");
		log.debug("join query is: "+query);
		return query;
	}
	
}
