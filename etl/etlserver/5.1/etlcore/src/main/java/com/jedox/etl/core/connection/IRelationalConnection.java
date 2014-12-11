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
package com.jedox.etl.core.connection;

import java.io.StringWriter;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Interface for Relational based Connections to implement
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IRelationalConnection extends IConnection {

	public java.sql.Connection open() throws RuntimeException;
	/**
	 * creates a processor on the connection with the given settings.
	 * @param query the query to execute via the connection
	 * @param onlyHeader only get header information, but no data if true
	 * @param size the number of rows to fetch
	 * @return the processor holding the data
	 * @throws RuntimeException
	 */
	public IProcessor getProcessor(String query, Boolean onlyHeader, int size) throws RuntimeException;
	/**
	 * creates a processor on the connection with the given settings.
	 * @param query the query to execute via the connection
	 * @param aliasMap the aliasMap to overlay on the source data columns
	 * @param onlyHeader only get header information, but no data if true
	 * @param ignoreInternalKey if true, ignores internal key columns.
	 * @param size the number of rows to fetch
	 * @return the processor holding the data
	 * @throws RuntimeException
	 */
	public IProcessor getProcessor(String query, IAliasMap aliasMap, Boolean onlyHeader, Boolean ignoreInternalKey, int size) throws RuntimeException;
	/**
	 * gets the catalogs of the database in csv format as delivered by a {@link java.sql.DatabaseMetaData} object.
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getCatalogs() throws RuntimeException;
	/**
	 * gets the schemas of the database in csv format as delivered by a {@link java.sql.DatabaseMetaData} object.
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getSchemas() throws RuntimeException;
	/**
	 * gets the tables of the database in csv format as delivered by a {@link java.sql.DatabaseMetaData} object.
	 * @param catalogPattern see {@link java.sql.DatabaseMetaData#getTables(String, String, String, String[])}
	 * @param schemaPattern see {@link java.sql.DatabaseMetaData#getTables(String, String, String, String[])}
	 * @param tablePattern see {@link java.sql.DatabaseMetaData#getTables(String, String, String, String[])}
	 * @param types comma seperated types converted to an array. see {@link java.sql.DatabaseMetaData#getTables(String, String, String, String[])}
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getTables(String catalogPattern, String schemaPattern, String tablePattern, String types) throws RuntimeException;
	/**
	 * gets the columns of the database in csv format as delivered by a {@link java.sql.DatabaseMetaData} object.
	 * @param catalogPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param schemaPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param tablePattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param columnPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getColumns(String catalogPattern, String schemaPattern, String tablePattern, String columnPattern) throws RuntimeException;
	
	/**
	 * gets the parameters of the procedure in a database in csv format as delivered by a {@link java.sql.DatabaseMetaData} object.
	 * @param catalogPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param schemaPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param procedurePattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param columnPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getProcedureParameters(String catalogPattern, String schemaPattern, String procedurePattern, String columnPattern) throws RuntimeException ;
	
	/**
	 * gets the procedues of the database in csv format as delivered by a {@link java.sql.DatabaseMetaData} object.
	 * @param catalogPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param schemaPattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @param procedurePattern see {@link java.sql.DatabaseMetaData#getColumns(String, String, String, String)}
	 * @return a StringWriter object holding the csv data
	 */
	public StringWriter getProcedures(String catalogPattern, String schemaPattern, String procedurePattern) throws RuntimeException;

	/**
	 * gets the driver name for this connection
	 * @return the driver name
	 */
	public String getDriver();

	/**
	 * gets the connection URL for this connection
	 * @return the connection URL
	 * @throws RuntimeException
	 */
	public String getConnectionUrl() throws RuntimeException;

	/**
	 * gets the user name for this connection
	 * @return the user name
	 */
	public String getUser();

	/**
	 * gets the password for this connection
	 * @return the password
	 */
	public String getPassword();

	/**
	 * gets the Quoting character used for quoting names in the database;
	 * @return the identifier quote
	 */
	public String getIdentifierQuote() throws RuntimeException;

	/**
	 * determines if a single connection instance is to be used or if multiple connections may be created.
	 * @return true or false
	 */
	public boolean isSingleton();

	/**
	 * determines if schemas are supported from the database system
	 * @return true or false
	 */
	public boolean isSchemaSupported();

	/**
	 * determines if schemas should be automatically created when needed in the database.
	 * @return true or false
	 */
	public boolean autoCreateSchemata();

	/**
	 * explicitly commits all changes done via this connection
	 * @throws RuntimeException
	 */
	public void commit() throws RuntimeException;

	/**
	 * get the fetch Mode of the connection, the possible values are full or buffered
	 * @throws RuntimeException
	 */
	public FetchModes getFetchMode() throws RuntimeException;
	
	/**
	 * check whether to allow this connection to be used in RelationalSP load
	 * @return
	 */
	public boolean allowInRelationalSP();

}
