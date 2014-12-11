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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.extract.IRelationalExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;



/**
 * DataSource for a SQL-Backend
 * The following properties have to be specified for creation by the factory:
 * name: the name of the datasource
 * connection: the name of the connection
 * query: the SQL for querying the database.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class SQLExtract extends TableSource implements IRelationalExtract {

	private static final Log log = LogFactory.getLog(SQLExtract.class);

	public SQLExtract() {
		setConfigurator(new TableSourceConfigurator());
	}

	public TableSourceConfigurator getConfigurator() {
		return (TableSourceConfigurator) super.getConfigurator();
	}


	public IRelationalConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IRelationalConnection))
			return (IRelationalConnection) connection;
		throw new RuntimeException("Relational connection is needed for extract "+getName()+".");
	}
	
	protected IProcessor doErrorHandling(Exception e) throws RuntimeException {
		String errorMessage = e.getMessage();
		log.debug("Failed to connect to extract "+getName() + ": " + errorMessage);
		/* cschw: I would solve this like that...
		if(e.toString().substring(0,e.toString().indexOf(':')).equals("java.sql.SQLException")){
			int i = e.getMessage().indexOf('[')+1;
			int j = Math.max(e.getMessage().indexOf(']'),i);
			if (i != j) 
				errorMessage = e.getMessage().substring(i, j);
			throw new RuntimeException("Error in SQL Query of extract "+getName() + ": " + errorMessage);
		}
		else{
			throw new RuntimeException(errorMessage);
		} 
		*/
		if(e.toString().indexOf(':') != -1 && e.toString().substring(0,e.toString().indexOf(':')).equals("java.sql.SQLException")){
			if(e.getMessage().indexOf('[') != -1){
				errorMessage = e.getMessage().substring(e.getMessage().indexOf('[')+1, e.getMessage().indexOf(']'));
			}
			else{
				errorMessage = e.getMessage().substring(e.getMessage().indexOf(':')+2,e.getMessage().indexOf('\n'));
			}
			throw new RuntimeException("Error in SQL Query of extract "+getName() + ": " + errorMessage);
		}
		else{
			throw new RuntimeException(errorMessage);
		}
	}
	
	public Row getOutputDescription() throws RuntimeException {
		log.debug(getQuerySource());
		try {
				String query = getQuerySource();
				// Only fetch the header line to get the layout of the data
				IProcessor processor = getConnection().getProcessor(query, getAliasMap(), true, true, 1);
				processor.setName(getName());
				Row od = processor.getOutputDescription();
				od.setAliases(getAliasMap());
				return od;
		}
		catch (Exception e) {
			return doErrorHandling(e).current();
		}
	}


	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		log.debug(getQuerySource());
		try {
				String query = getQuerySource();
				IProcessor processor = getConnection().getProcessor(query, getAliasMap(),false, true, size);
				processor.current().setAliases(getAliasMap());
				processor.setName(getName());
				return processor;
				//return new TableProcessor(getName(),getAliasMap(),getConnection().createStatement(getFetchMode(), size).executeQuery(query));
		}
		catch (Exception e) {
			return doErrorHandling(e);
		}
	}

	public String getSQLQuery() {
		return getQuerySource();
	}
	
	public void init() throws InitializationException {
		super.init();
	}

}
