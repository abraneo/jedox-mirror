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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.load;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.load.RelationalConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode.UpdateModes;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IFileConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.source.processor.IProcessor;

import java.util.Map;

public class RelationalLoad extends Load {

	private static final Log log = LogFactory.getLog(RelationalLoad.class);
	private Map<String, UpdateModes> updateStrategy;
	private String tableName;
	private String schemaName;
	private boolean plainNames;
	private boolean createKeyColumn;
	private String primaryKey;
	private String primaryKeyGeneration;
	private String aggregateMode;

	public RelationalLoad() {
		setConfigurator(new RelationalConfigurator());
	}

	public RelationalConfigurator getConfigurator() {
		return (RelationalConfigurator)super.getConfigurator();
	}

	public IRelationalConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IRelationalConnection && !(connection instanceof IFileConnection)))
			return (IRelationalConnection) connection;
		throw new RuntimeException("Relational connection is needed for source "+getName()+".");
	}

	public void executeLoad() {
		
		IProcessor processor=null;
		PersistorDefinition definition=null;
		log.info("Starting load "+getName()+" to table "+tableName);
		try {
			processor = getProcessor();
			Locator loc = getLocator().clone().reduce().add(tableName);
			loc.setPersistentSchema(schemaName);
			loc.setPersistentTable(tableName);
			definition = new PersistorDefinition();
			definition.setConnection(getConnection());
			definition.setLocator(loc);
			definition.setMode(getMode());
			definition.setPlain(plainNames);
			definition.setCreateKeyColumn(createKeyColumn);
			definition.setPrimaryKey(primaryKey);
			definition.setPrimaryKeyGeneration(primaryKeyGeneration);
			definition.setAggregateMode(aggregateMode);
			definition.setBulkSize(Integer.parseInt(getParameter("bulkSize","10000")));
			definition.setLogging(true);
			//check columns, if they match the processor
			for (String s : updateStrategy.keySet()) {
				if (!processor.current().containsColumn(s)) {
					log.error(getName()+": Column "+s+" is not part of underlying source. Aborting...");
					return;
				}
			}
			//set roles
			for (int i=0; i<processor.current().size(); i++) {
				IColumn c = processor.current().getColumn(i);
				UpdateModes mode = updateStrategy.get(c.getName());
				if (mode != null) {
					if (mode.equals(UpdateModes.none)) {//not set exclicitly - determine mode via load mode
						definition.setRole(c.getName(), getDataPersistenceMode());
					}
					else
						definition.setRole(c.getName(), mode);
					//set datatype to double if needed
					switch (mode) {
					case sum: definition.setType(c.getName(), Double.class); break;
					case avg: definition.setType(c.getName(), Double.class); break;
					case count: definition.setType(c.getName(), Double.class); break;
					default:
					}
				}
				else {
					if (!c.getName().equals(definition.getPrimaryKey()))
						definition.setRole(c.getName(), UpdateModes.first);
				}
			}
			processor.addPersistor(definition);
			processor.run();
		}
		catch (Exception e) {
			if(e.getMessage()!=null)
				log.error(e.getMessage().replaceAll("\n", ""));
		}finally{
			if(definition!=null){
				try {
					processor.removePersistor(definition);
				} catch (RuntimeException e) {
					log.error(e.getMessage());
				}
			}
		}
		log.info("Finished load "+getName());
	}
		
	public void init() throws InitializationException {
		try {
			super.init();
			updateStrategy = getConfigurator().getUpateStrategy();
			tableName = getConfigurator().getTableName();
			schemaName = getConfigurator().getSchemaName();
			plainNames = getConfigurator().usePlainNames();
			createKeyColumn = getConfigurator().doCreateKeyColumn();
			primaryKey = getConfigurator().getPrimaryKey();
			primaryKeyGeneration = getConfigurator().getPrimaryKeyGeneration();
			aggregateMode = getConfigurator().getAggregationMode();
			// check if database connection supports Schemas
			if (!schemaName.isEmpty() && !getConnection().isSchemaSupported()) {
				throw new InitializationException("Schema cannot be set in load "+getName()+" for connection "+getConnection().getName()+". It has to be set in the connection.");
			}	
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
