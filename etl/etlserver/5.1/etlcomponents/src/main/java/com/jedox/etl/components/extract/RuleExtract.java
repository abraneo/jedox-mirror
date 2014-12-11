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

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.components.config.extract.RuleExtractConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.*;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IRule;


public class RuleExtract extends TableSource implements IExtract  {

	private class RuleProcessor extends Processor {

		private Row row;
		private int count;// the count of the rules
		private IRule[] cubeRules;

		String [] columns = {"Definition","Comment","ExternID","Timestamp"};
		
		/**
		 * read rules from the cube and set the columns names
		 * @throws RuntimeException
		 */
		public RuleProcessor() {
			
		}

		protected boolean fillRow(Row row) throws Exception {
			IRule currentRule;
			// try first to fill the static rules
			if (conf.getStaticRules().length > count) {
				currentRule=conf.getStaticRules()[count];
				count++;
			}
			// fill the rules from cube
			else if (cubeRules!= null && cubeRules.length > count) {
				currentRule = cubeRules[count];
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

		@Override
		protected void init() throws RuntimeException {
			try {
				cubeRules =  readCubeRules();
				row = PersistenceUtil.getColumnDefinition(getAliasMap(),columns);
				count = 0;
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to get rules from cube "+conf.getCubeName()+": "+e.getMessage());
			}
		}
	}

	RuleExtractConfigurator conf;
	
	public RuleExtract() {
		conf = new RuleExtractConfigurator();
		setConfigurator(conf);
	}

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

	private Row setRowFromRule (IRule rule, Row row) {
		row.getColumn(0).setValue((rule.isActive() ? "" : "#") + rule.getDefinition());
		row.getColumn(1).setValue(rule.getComment());
		row.getColumn(2).setValue(rule.getExternalIdentifier());
		row.getColumn(3).setValue(rule.getTimestamp());
		return row;
	}

	/**
	 * get the cube rules from Olap Server
	 * @return array of rules
	 * @throws RuntimeException
	 */
	private IRule[] readCubeRules() throws RuntimeException{
		if(conf.getCubeName() == null) {
			return null;
		}
		//com.jedox.palojlib.interfaces.IConnection con = (com.jedox.palojlib.interfaces.IConnection) getConnection().open();
		String database = getConnection().getDatabase();
		if (database == null) {
			return null;
		}
		IDatabase d = getConnection().getDatabase(false,true);
		ICube cube = d.getCubeByName(conf.getCubeName());
		if (cube == null) {
			throw new RuntimeException("Cube "+conf.getCubeName()+" not found in database "+database);
		}
		
		return cube.getRules();		
	}

	/**
	 * get a specific number of rows
	 */
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor result = initProcessor(new RuleProcessor(),Facets.OUTPUT);	
		result.setLastRow(size);
		return result;
	}

}
