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
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IRule;


public class RuleLoad extends Load implements ILoad {

	private static final Log log = LogFactory.getLog(RuleLoad.class);
	private String cubeName;
	private boolean isIDFormat;

	private class CubeRule {
		public String definition; // the value of the rule
		public String id; // an optional id for the rule
		public String comment; // optional comment
		public boolean active; // whether it is active or not
		public String externalIdentifier;
		public long timestamp;
	}

	/**
	 * constructor that set the configurator
	 */

	public RuleLoad() {
		setConfigurator(new RuleConfigurator());
	}

	/**
	 * get the configurator
	 */
	public RuleConfigurator getConfigurator() {
		return (RuleConfigurator)super.getConfigurator();
	}

	/**
	 * get the connection
	 */
	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		throw new RuntimeException("OLAP connection is needed for source "+getName()+".");
	}

	/**
	 * gets the name of the database hosting this cube
	 * @return
	 */
	public String getDatabaseName() throws RuntimeException {
		return getConnection().getDatabase();
	}

	/**
	 * get the rule from OLAP cube if it exists
	 * Note: these rules are of type org.palo.api.Rule
	 * @param Cube c
	 * @param new Rule
	 * @return the needed rule if existed
	 */
	private IRule getRuleFromCube(ICube c, CubeRule newRule) {
		IRule[] rules = c.getRules();
		for (IRule r: rules) {
			// Check identity of rules on Rule ID in ID-format, otherwise on Rule definition
			if ((isIDFormat && String.valueOf(r.getIdentifier()).equals(newRule.id)) ||
				(!isIDFormat &&r.getDefinition().equals(newRule.definition)))
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
		boolean isOldOlapVersion = getConnection().isOlderVersion(5, 0, 4098);
		// get
		ArrayList<CubeRule> rules = readRules();
		// search the rules in the load, if they exist already in the cube then delete them
		switch (mode) {
		case DELETE: {
			log.debug("Start deleting rules");
			ArrayList<IRule> cubeRules =new ArrayList<IRule>();
			for (CubeRule rule : rules) {
				IRule r = getRuleFromCube(c, rule);
				if (r != null) {
					if(isOldOlapVersion)
						c.removeRules(new IRule[]{r});
					cubeRules.add(r);
					countDelete ++;
				}
				else
					log.warn("Rule "+rule.comment+" does not exist and therefore can not be deleted.");
			}
			if(!isOldOlapVersion && cubeRules.size()!=0)
				c.removeRules(cubeRules.toArray(new IRule[0]));
			break;
		}
		case INSERT: {
			log.debug("Starting inserting rules.");
			// search the rules in the load, if they exist already in the cube then delete them
			for (CubeRule rule : rules) {
				//remove before add to avoid duplicate rules
				IRule r = getRuleFromCube(c,rule);
				if (r != null){
					//r.update(rule.definition, rule.externalIdentifier ,true, rule.comment,rule.active);
					c.updateRule(r.getIdentifier(), rule.definition, rule.active, rule.externalIdentifier,rule.comment);
					countUpdate ++;
				}
				else {
					c.addRule(rule.definition, rule.active, rule.externalIdentifier,rule.comment);
					countAdd ++;
				}
			}
			break;
		}
		case UPDATE: {
			//delete the old rules
			log.debug("Deleting existing rules in cube " + c.getName());
			countDelete = c.getRules().length;
			if(countDelete!=0){
				if(!isOldOlapVersion)
					c.removeRules();
				else
					for(IRule r:c.getRules())
						c.removeRules(new IRule[]{r});
			}
			//add the new ones
			log.debug("Inserting the new rules in cube " + c.getName());
			for (CubeRule rule : rules) {
				c.addRule(rule.definition, rule.active, rule.externalIdentifier , rule.comment);
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
	public void execute() {
		if (isExecutable()) {
			log.info("Starting Rule load "+getName()+" into Cube "+ cubeName);
			try {
				 com.jedox.palojlib.interfaces.IConnection connection =  (com.jedox.palojlib.interfaces.IConnection)getConnection().open();
				IDatabase d = connection.getDatabaseByName(getDatabaseName());

				if(d==null){
					throw new Exception("Database "+getDatabaseName()+" does not exist.");
				}
				ICube c = d.getCubeByName(cubeName);
				if(c != null){
					exportRules(c,getMode());
				}
				else{
					throw new Exception("Cube "+cubeName+" does not exist.");
				}
			}
			catch (Exception e) {
				log.error("Cannot load rules into Cube "+ cubeName+": "+e.getMessage());
				log.debug("",e);
			}
			log.info("Finished Rule load "+getName()+" into Cube "+ cubeName+".");
		}
	}

	private CubeRule getRuleFromRow (Row r) {
		CubeRule rule = new CubeRule();
		if (isIDFormat) {
			// Line-Format for Rules with Rule IDs, now deprecated
			// "ID";"Comment";"Definition";"Active"
			// "1";"Actual rule 1";"['Actual'] = ['2006']";"false"
			rule.id = r.getColumn(0).getValueAsString();
			rule.comment = r.getColumn(1).getValueAsString();
			rule.definition = r.getColumn(2).getValueAsString();
			rule.active = r.getColumn(3).getValueAsString().equalsIgnoreCase("false");
		} else {
			// Line-Format for Rules as used in Palo Excel Modeler Export, new in ETL 3.3
			// Active(/#)Definition;Comment;Extern ID;Timestamp
			// #['Actual'] = ['2006'];Actual rule 1;;1294151138
			String column0 = r.getColumn(0).getValueAsString();
			if (column0==null || column0.length()==0) {
				rule.active = true;
				rule.definition = "";
			} else {
				if (column0.startsWith("#")) {
					rule.active = false;
					rule.definition = column0.substring(1);
				} else {
					rule.active = true;
					rule.definition = column0;
				}
			}
			rule.comment = r.getColumn(1).getValueAsString();
			rule.externalIdentifier = r.getColumn(2).getValueAsString();
			rule.timestamp = Long.parseLong(r.getColumn(3).getValueAsString());
		}
		return rule;
	}

	/**
	 * read the rules from the processor and return them as an array list of rules
	 * @return array list of rules
	 * @throws RuntimeException
	 */
	private ArrayList<CubeRule> readRules() throws RuntimeException {
		ArrayList<CubeRule> rules = new ArrayList<CubeRule>();
		IProcessor rulesProcessor = getProcessesor();
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

	/**
	 * get the processor that contains the rules, it should contain exactly 4 columns
	 * @return the processor that contains the rules
	 */
	private IProcessor getProcessesor() {
		TableSource rulesSource = null;
		try {
			rulesSource = (TableSource)(getConfigurator().getSource());
		} catch (CreationException e) {
			log.error("Unable to read source table: " + e.getMessage());
			log.debug(e);
			return null;
		}

		try {
			return rulesSource.getProcessor();
		} catch (RuntimeException e) {
			log.error("Unable to read read source table: " + e.getMessage());
			log.debug(e);
			return null;
		}
	}

	public void init() throws InitializationException {
		try {
			super.init();
			cubeName = getConfigurator().getCubeName();
			isIDFormat = getConfigurator().isIDFormat();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
