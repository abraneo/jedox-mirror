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

package com.jedox.etl.components.extract;

import java.util.ArrayList;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.components.config.extract.RuleExtractConfigurator;
import com.jedox.etl.components.config.extract.RuleExtractConfigurator.CubeRule;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.*;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IRule;


public class RuleExtract extends TableSource implements IExtract  {

	private class RuleProcessor extends Processor {

		private Row row;
		private int count;// the count of the rules
		private ArrayList<CubeRule> cubeRules;

		/**
		 * read rules from the cube and set the columns names
		 * @throws RuntimeException
		 */
		public RuleProcessor() throws RuntimeException {
			try {
				if(cubeName!= null)
					cubeRules =  readCubeRules();
				row = PersistenceUtil.getColumnDefinition(getAliasMap(),getColumns());
				count = 0;
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to get rules from cube "+cubeName+": "+e.getMessage());
			}
		}

		protected boolean fillRow(Row row) throws Exception {
			CubeRule currentRule = getConfigurator().new CubeRule();
			// try first to fill the static rules
			if (staticRules.size() > count) {
				currentRule=staticRules.get(count);
				count++;
			}
			// fill the rules from cube
			else if (cubeRules!= null && cubeRules.size() > count) {
				currentRule = cubeRules.get(count);
				count++;
			}
			else{
				//finished ... do some cleanup
				count = 0;
				return false;
			}
			row = setRowFromRule(currentRule,row);
			return true;
		}

		protected Row getRow() {
			return row;
		}
	}

	private String cubeName;
	private boolean isIDFormat;
	// rules retrieved from the tags <rules> (or in other words the static rules)
	private ArrayList<CubeRule> staticRules;

	/**
	 * constructor
	 */
	public RuleExtract() {
		setConfigurator(new RuleExtractConfigurator());
	}

	/**
	 * get the configurator
	 */
	public RuleExtractConfigurator getConfigurator() {
		return (RuleExtractConfigurator)super.getConfigurator();
	}

	/**
	 * get the connection
	 */
	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		throw new RuntimeException("OLAP connection is needed for this source: "+getName()+". Connection set is of type: "+ connection.toString());
	}

	/**
	 * get columns names
	 * @return array of string that represent the columns names
	 */
	private String[] getColumns() {
		String [] columns = {"Definition","Comment","ExternID","Timestamp"};
		String [] columnsIdFormat = {"ID","Comment","Definition","Active"};
		if (isIDFormat)
			return columnsIdFormat;
		else
			return columns;
	}

	private Row setRowFromRule (CubeRule rule, Row row) {
		if (isIDFormat) {
			row.getColumn(0).setValue(rule.id);
			if(rule.comment != null ){
				row.getColumn(1).setValue(rule.comment);
			}
			row.getColumn(2).setValue(rule.definition);
			row.getColumn(3).setValue(rule.active);
		}
		else {
			row.getColumn(0).setValue((rule.active ? "" : "#") + rule.definition);
			row.getColumn(1).setValue(rule.comment);
			row.getColumn(2).setValue(rule.externalIdentifier);
			row.getColumn(3).setValue(rule.timestamp);
		}
		return row;
	}

	/**
	 * get the cube rules from Olap Server
	 * @return array of rules
	 * @throws RuntimeException
	 */
	private ArrayList<CubeRule> readCubeRules() throws RuntimeException{
		ArrayList<CubeRule> cubeRules = new ArrayList<CubeRule>();
		if(getConnection() == null){
			return cubeRules;
		}
		com.jedox.palojlib.interfaces.IConnection con = (com.jedox.palojlib.interfaces.IConnection) getConnection().open();
		if(getConnection().getDatabase() == null){
			return cubeRules;
		}
		String database = getConnection().getDatabase();

		IDatabase d = con.getDatabaseByName(database);
		if (d == null) {
			throw new RuntimeException("Database "+database+" not found in connection "+getConnection().getName());
		}
		ICube cube = d.getCubeByName(cubeName);
		if (cube == null) {
			throw new RuntimeException("Cube "+cubeName+" not found in database "+database);
		}

		for (IRule r : cube.getRules()) {
			CubeRule cubeRule = getConfigurator().new CubeRule();
			cubeRule.active=r.isActive();
			cubeRule.id= String.valueOf(r.getIdentifier());
			cubeRule.comment=r.getComment();
			cubeRule.definition=r.getDefinition();
			cubeRule.externalIdentifier=r.getExternalIdentifier();
			cubeRule.timestamp=r.getTimestamp();
			cubeRules.add(cubeRule);
		}
		return cubeRules;
	}

	/**
	 * get a specific number of rows
	 */
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor result = new RuleProcessor();
		result.setLastRow(size);
		return result;
	}

	/**
	 * initialize the source
	 */
	public void init() throws InitializationException {
		try {
			super.init();
			cubeName = getConfigurator().getCubeName();
			staticRules  = getConfigurator().getStaticRules();
			isIDFormat = getConfigurator().isIDFormat();
		}
		catch (Exception e) {
			invalidate();
			throw new InitializationException(e);
		}
	}

}
