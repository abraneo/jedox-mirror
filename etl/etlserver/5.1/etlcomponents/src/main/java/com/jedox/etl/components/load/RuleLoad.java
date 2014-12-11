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
 *   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.components.load;

import java.util.ArrayList;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.RuleConfigurator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IRule;


public class RuleLoad extends AbstractOlapLoad implements ILoad {

	private class Rule extends com.jedox.palojlib.main.Rule implements IRule {

		private Rule(String definition, String externalIdentifier, String comment, long timestamp, boolean active){
			super(0, definition, externalIdentifier, comment, timestamp, active);
		}				
	}
	
	private static final Log log = LogFactory.getLog(RuleLoad.class);	
	private RuleConfigurator conf;

	public RuleLoad() {
		conf = new RuleConfigurator();
		setConfigurator(conf);
	}

	public RuleConfigurator getConfigurator() {
		return (RuleConfigurator)super.getConfigurator();
	}

	/**
	 * get the rule from OLAP cube if it exists
	 * Note: these rules are of type org.palo.api.Rule
	 * @param Cube c
	 * @param new Rule
	 * @return the needed rule if existed
	 */
	private IRule getRuleFromCube(ICube c, IRule newRule) {
		IRule[] rules = c.getRules();
		for (IRule r: rules) {
			// Check identity of rules on Rule definition
			if (r.getDefinition().equals(newRule.getDefinition()))
					return r;
		}
		return null;
	}

	/**
	 * load the rules to a certain cube with a certain load mode
	 * @param c OLAP cube
	 * @param mode the load mode (only update and delete are allowed)
	 */
	private void exportRules(ICube c, Modes mode) throws RuntimeException {
		int countDelete = 0;
		int countAdd = 0;
		int countUpdate = 0;
		// get
		ArrayList<IRule> rules = readRules();
		// search the rules in the load, if they exist already in the cube then delete them
		switch (mode) {
		case DELETE: {
			log.debug("Start deleting rules");
			ArrayList<IRule> cubeRules =new ArrayList<IRule>();
			for (IRule rule : rules) {
				IRule r = getRuleFromCube(c, rule);
				if (r != null) {
					cubeRules.add(r);
					countDelete ++;
				}
				else
					log.warn("Rule "+rule.getComment()+" does not exist and therefore can not be deleted.");
			}
			if (cubeRules.size()!=0)
				c.removeRules(cubeRules.toArray(new IRule[0]));
			break;
		}
		case INSERT: {
			log.debug("Starting inserting rules.");
			// search the rules in the load, if they exist already in the cube then delete them
			for (IRule rule : rules) {
				//remove before add to avoid duplicate rules
				IRule r = getRuleFromCube(c,rule);
				if (r != null){
					//r.update(rule.definition, rule.externalIdentifier ,true, rule.comment,rule.active);
					c.updateRule(r.getIdentifier(), rule.getDefinition(), rule.isActive(), rule.getExternalIdentifier(), rule.getComment());
					countUpdate ++;
				}
				else {
					c.addRule(rule.getDefinition(), rule.isActive(), rule.getExternalIdentifier(),rule.getComment());
					countAdd ++;
				}
			}
			break;
		}
		case UPDATE: {
			//delete the old rules
			log.debug("Deleting existing rules in cube " + c.getName());
			countDelete = c.getRules().length;
			if(countDelete!=0) {
				c.removeRules();
			}
			//add the new ones
			log.debug("Inserting the new rules in cube " + c.getName());
			for (IRule rule : rules) {
				c.addRule(rule.getDefinition(), rule.isActive(), rule.getExternalIdentifier() , rule.getComment());
				countAdd ++;
			}
			break;
		}
		default: {
			log.error("Unsupported mode in load "+getName());
		}
		}
		if (countDelete>0)
			log.info("Rules deleted in Cube "+c.getName()+": " + countDelete);
		if (countUpdate>0)
			log.info("Rules updated in Cube "+c.getName()+": " + countUpdate);
		if (countAdd>0)
			log.info("Rules added in Cube "+c.getName()+": " + countAdd);
		if (countDelete==0 && countUpdate==0 && countAdd==0)
			log.info("No Rules were loaded to Cube "+c.getName());

	}
	/**
	 * execute the load
	 */
	public void executeLoad() {
		log.info("Starting Rule load "+getName()+" into Cube "+ conf.getCubeName());
		try {
			IDatabase d = getConnection().getDatabase(false,true);
			ICube c = d.getCubeByName(conf.getCubeName());
			if (c != null){
				exportRules(c,getMode());
			}
			else{
				throw new Exception("Cube "+ conf.getCubeName()+" does not exist.");
			}
		}
		catch (Exception e) {
			log.error("Cannot load rules into Cube "+ conf.getCubeName()+": "+e.getMessage());
			log.debug("",e);
		}
		log.info("Finished Rule load "+getName()+" into Cube "+  conf.getCubeName()+".");
	}

	private IRule getRuleFromRow (Row r) {		
			// Line-Format for Rules as used in Palo Excel Modeler Export, new in ETL 3.3
			// Active(/#)Definition;Comment;Extern ID;Timestamp
			// #['Actual'] = ['2006'];Actual rule 1;;1294151138
		boolean active;
		String definition;
		String column0 = r.getColumn(0).getValueAsString();
		if (column0==null || column0.length()==0) {
			active = true;
			definition = "";
		} else {
			if (column0.startsWith("#")) {
				active = false;
				definition = column0.substring(1);
			} else {
				active = true;
				definition = column0;
			}
		}
		Rule rule = new Rule(definition,
	   	                     r.getColumn(2).getValueAsString(),	
		                     r.getColumn(1).getValueAsString(),
		                     Long.parseLong(r.getColumn(3).getValueAsString()),
		                     active);
		return rule;
	}

	/**
	 * read the rules from the processor and return them as an array list of rules
	 * @return array list of rules
	 * @throws RuntimeException
	 */
	private ArrayList<IRule> readRules() throws RuntimeException {
		ArrayList<IRule> rules = new ArrayList<IRule>();
		IProcessor rulesProcessor = getProcessor();
		if (rulesProcessor == null)
			return rules;

		Row r = rulesProcessor.current();
		// Both Rule Formats require 4 columns
		if(r.size() != 4){
			throw new RuntimeException("The number of columns in the referenced source "+rulesProcessor.getName()+" should be 4");
		}

		while (rulesProcessor.next() != null) {
			rules.add(getRuleFromRow(r));
		}
		return rules;
	}
}
