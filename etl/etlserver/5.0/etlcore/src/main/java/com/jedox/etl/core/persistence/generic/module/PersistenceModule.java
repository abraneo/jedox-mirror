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
package com.jedox.etl.core.persistence.generic.module;

import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Hashtable;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public abstract class PersistenceModule {
	
	private static final Log log = LogFactory.getLog(PersistenceModule.class);
	private Hashtable<String,PreparedStatement> lookup = new Hashtable<String,PreparedStatement>();
	private Hashtable<PreparedStatement, Integer> batchMap = new Hashtable<PreparedStatement,Integer>();
	private int bulkSize = 100;
	private int logbulkSize = 50000; // Minimal number of records without log messages
	private boolean isLogging = false;
	
	protected void addStatement(String persistentName, PreparedStatement statement) {
		lookup.put(persistentName, statement);
		batchMap.put(statement, 0);
	}
	
	public PreparedStatement getStatement(String persistentName) {
		return lookup.get(persistentName);
	}
	
	public PreparedStatement removeStatement(String persistentName) {
		PreparedStatement statement = lookup.remove(persistentName);
		if (statement != null) batchMap.remove(statement);
		return statement;
	}
	
	public void closeStatement(String persistentName) throws SQLException {
		PreparedStatement statement = getStatement(persistentName);
		if (statement != null) {
			flush(statement);
			statement.close();
		}
		removeStatement(persistentName);
	}
	
	public int addBatch(PreparedStatement statement) throws SQLException {

		if(bulkSize>=2){
			Integer recordCount = batchMap.get(statement);
			statement.addBatch();
			
			batchMap.put(statement, ++recordCount);
			if ((recordCount % bulkSize)==0) {
				statement.executeBatch();
				if (isLogging && (bulkSize>=logbulkSize || recordCount%logbulkSize<bulkSize))
						// For a small bulksize set the minimal blocksize for logs to logbulksize
						log.info("Records loaded to table: "+recordCount);				
			}
			return recordCount;
		}else{
			return statement.executeUpdate();
		}
	}
	
	public void flush(PreparedStatement statement) throws SQLException {
		Integer batchSize = batchMap.get(statement);
		if (batchSize != null && batchSize > 0) {
			statement.executeBatch();
			if (isLogging) {
				log.info("Records loaded to table: "+batchSize);
			}			
		}
	}
	
	public void setBulkSize(int bulkSize) {
		this.bulkSize = bulkSize;
	}

	public void setLogging(boolean isLogging) {
		this.isLogging = isLogging;
	}
	
}
