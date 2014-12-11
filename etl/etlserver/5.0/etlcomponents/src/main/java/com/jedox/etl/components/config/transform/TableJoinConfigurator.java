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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.transform;

import java.util.List;
import java.util.ArrayList;
//import java.util.HashMap;

import org.jdom.Element;

import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.FilterConfigurator;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.config.transform.TransformConfigUtil;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.source.filter.RowFilter;

public class TableJoinConfigurator extends TableSourceConfigurator implements
		ITransformConfigurator {
	
	public enum Conditions {
		EQ, NE, LE, LT, GT, GE
	}
	
	public class Key {
		private String name;
		private boolean constant;
		
		public Key(Element key) {
			String nameref = key.getAttributeValue("nameref");
			String constant = key.getAttributeValue("constant");
			if (nameref != null) {
				this.name = nameref;
				this.constant = false;
			} else {
				this.name = constant;
				this.constant = true;
			}
		}
		
		public String getName() {
			return name;
		}
		public boolean isConstant() {
			return constant;
		}
	}
	
	public class Match {
		
		private String leftSource;
		private String rightSource;
		private Key leftKey;
		private Key rightKey;
		private String condition;
		
		public Match(String leftSource, Key leftKey, String rightSource, Key rightKey, String condition) {
			this.leftSource = leftSource;
			this.rightSource = rightSource;
			this.leftKey = leftKey;
			this.rightKey = rightKey;
			this.condition = condition;
		}
		
		public Key getLeftKey() {
			return leftKey;
		}
		public Key getRightKey() {
			return rightKey;
		}

		public String getLeftSource() {
			return leftSource;
		}

		public String getRightSource() {
			return rightSource;
		}
		
		public boolean containsSources(List<String> sources) {
			for (String source : sources) {
				if (leftSource.equals(source) || rightSource.equals(source)) return true;
			}
			return false;
		}
		
		public Conditions getCondition() {
			return Conditions.valueOf(condition);
		}

		public String getConditionOperator() {
			switch (Conditions.valueOf(condition)) {
			case EQ: return "=";
			case NE: return "!=";
			case LE: return "<=";
			case LT: return "<";
			case GE: return ">=";
			case GT: return ">";
			default: return "=";
			}
		}
		
	}
	
	public class JoinDefinition {
		private String leftSource;
		private String rightSource;
		private String type;
		private ArrayList<Match> matches = new ArrayList<Match>();
		
		public JoinDefinition(String leftSource, String rightSource, String type) {
			this.leftSource = leftSource;
			this.rightSource = rightSource;
			this.type = type;
		}
		
		public String getLeftSource() {
			return leftSource;
		}
		public String getRightSource() {
			return rightSource;
		}
		public String getType() {
			return type.equalsIgnoreCase("full") ? "inner" : type;
		}
		protected void addMatch(Match match) {
			matches.add(match);
		}
		public List<Match> getMatches() {
			return matches;
		}
	}
	
	private ColumnManager manager;
	private TransformConfigUtil util;

	public ColumnManager getColumnManager() throws RuntimeException {
		/*
		if (manager == null) {
			try {
				manager = util.getColumns();
				setAliasMap(AliasMap.build(manager.getRow().getOutputDescription()));
			} 
			catch (ConfigurationException e) {
				throw new RuntimeException(e);
			}
		}
		*/
		return manager;
	}

	public RowFilter getFilter() throws ConfigurationException {
		FilterConfigurator fc = new FilterConfigurator(getContext(), getParameter());
		return fc.getFilter(getXML().getChild("filter"));
	}

	public List<IComponent> getSources() throws ConfigurationException {
		return util.getSources();
	}

	public List<IComponent> getFunctions() throws ConfigurationException {
		return util.getFunctions();
	}
	
	private String getSourceName(String name) throws ConfigurationException {
		//compensate for intermediate view layer.
		IComponent c = util.getSourcesLookup().get(name);
		if (c != null)
			return c.getName();
		throw new ConfigurationException("Source "+name+" not found.");
	}
	
	
	public List<JoinDefinition> getJoins() throws ConfigurationException {
		List<Element> joins = getChildren(getXML(),"join");
		ArrayList<JoinDefinition> definitions = new ArrayList<JoinDefinition>();
		for (Element join : joins) {
			Element left;
			Element right;
			String joinType = join.getAttributeValue("type","inner").toLowerCase();
			if (joinType.equals("right outer")) { //switch left/right and use left outer join
				left = join.getChild("right");
				right = join.getChild("left");
				joinType = "left outer";
			}
			else { //use left / right as defined
				left = join.getChild("left");
				right = join.getChild("right");
			}
			//compensate for intermediate view layer.
			String leftName = getSourceName(left.getAttributeValue("nameref"));
			String rightName = getSourceName(right.getAttributeValue("nameref"));
			JoinDefinition def = new JoinDefinition(leftName,rightName,joinType);
			List<Element> leftkeys = getChildren(left,"key");
			List<Element> rightkeys = getChildren(right,"key");
			List<Element> conditions = getChildren(join,"condition");
			for (int i=0; i<leftkeys.size(); i++) {
				Element leftKey = leftkeys.get(i);
				Element rightKey = rightkeys.get(i);
				String condition = Conditions.EQ.toString();
				if (i < conditions.size()) condition = conditions.get(i).getAttributeValue("type",condition).toUpperCase();
				def.addMatch(new Match(leftName, new Key(leftKey),rightName, new Key(rightKey), condition));
			}
			definitions.add(def);
		}
		return definitions;
	}

	/* CSCHW: new join xml format. to be switched to later.
	public List<JoinDefinition> getJoins() throws ConfigurationException {
		List<Element> joins = getChildren(getXML(),"join");
		ArrayList<JoinDefinition> definitions = new ArrayList<JoinDefinition>();
		for (Element join : joins) {
			String leftName;
			String rightName;
			String joinType = join.getAttributeValue("type","inner").toLowerCase();
			boolean switchKeys = false;
			if (joinType.equals("right outer")) { //switch left/right and use left outer join
				leftName =  getSourceName(join.getAttributeValue("right"));
				rightName = getSourceName(join.getAttributeValue("left"));
				joinType = "left outer";
				switchKeys = true;
			}
			else { //use left / right as defined
				leftName =  getSourceName(join.getAttributeValue("left"));
				rightName = getSourceName(join.getAttributeValue("right"));
			}
			List<Element> conditions = getChildren(join,"condition");
			JoinDefinition def = new JoinDefinition(leftName,rightName,joinType);
			for (Element condition : conditions) {
				Element leftKey;
				Element rightKey;
				if (switchKeys) { //switch left/right and use left outer join
					leftKey = condition.getChild("right");
					rightKey = condition.getChild("left");
				} else {
					leftKey = condition.getChild("left");
					rightKey = condition.getChild("right");
				}
				def.addMatch(new Match(leftName, new Key(leftKey),rightName, new Key(rightKey), condition.getAttributeValue("type","EQ").toUpperCase()));
			}
			definitions.add(def);
		}
		return definitions;
	}
	*/
	
	public boolean persist() {
		return getXML().getChild("joins").getAttributeValue("persist","false").equalsIgnoreCase("true");
	}

	public void configure() throws ConfigurationException {
		try {
			setName(getXML().getAttributeValue("name"));
			util = new TransformConfigUtil(getXML(),getLocator(),getContext());
			//manager = util.getColumns();
			setAliasMap(new AliasMap());
		}
		catch (Exception e) {
			//e.printStackTrace();
			throw new ConfigurationException("Failed to configure transform "+getName()+": "+e.getMessage());
		}
	}
}
