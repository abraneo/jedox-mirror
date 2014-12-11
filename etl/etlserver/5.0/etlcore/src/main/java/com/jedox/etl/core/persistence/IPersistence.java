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
package com.jedox.etl.core.persistence;

import java.sql.Connection;
import java.sql.PreparedStatement;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Interface for generalized operations on persistence back-ends.
 * The persistence back-end is used to export data to relational databases or internally store intermediate results in a temporary database (found in ./db/etl_temporary) for later access.
 * The persistence back-end is NOT intended to be used for the storage of permanent internal ETLServer data. For this purpose the JPA Interface (implemented by Hibernate) is used. The database created therefore is ./db/etl_persistence.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IPersistence {

	/**
	 * Enumerates the available update modes, when data is written to a persistence back-end. The update modes can be defined on a per column basis.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum UpdateModes {
		/**
		 * columns identified by mode key are used like primary keys to find fitting data vectors to be updated.
		 */
		key,
		/**
		 * the data of columns identified by mode keep is simple kept. The new value is dropped.
		 */
		first,
		/**
		 * the data of columns identified by mode put is written to the persistence and replaces old data.
		 */
		last,
		/**
		 * the data of column identified by mode add is added to the old value found in the persistence. The new value there is the sum of the old and the new data.
		 */
		sum,
		/**
		 * new value is the minimum value.
		 */
		min,
		/**
		 * new value is the maximum value
		 */
		max,
		/**
		 * new value is the number of values.
		 */
		count,
		/**
		 * new value is the average value
		 */
		avg,
		/**
		 * none defined.
		 */
		none
	}

	/**
	 * Gets the SQL Connection to the persistence database server.
	 * @return the SQL Connection
	 */
	public Connection getConnection() throws RuntimeException;

	/**
	 * Commits all changes done to the persistence layer. Necessary if the persistence layer has auto-commit disabled, does not support it or used PreparedStatements on the object have to be closed explicitly.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component
	 */
	public void commit(Locator locator);

	/**
	 * Gets an escaped version of a name. Certain database back-ends have reserved words, that can conflict with names chosen by the user in project definition files. Thus all names should be escaped by default. Unfortunately escaping of names is - like many other things concerning database - vendor specific.
	 * @param name the name to be escaped
	 * @return the escaped name.
	 * @throws RuntimeException
	 */
	public String escapeName(String name) throws RuntimeException;
	/**
	 * Inserts a {@org.proclos.etlcore.node.Row} of {@org.proclos.etlcore.node.IColumn Columns} holding data into the persistence layer.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param row the row holding the data columns.
	 */
	public void insert(Locator locator, Row row) throws RuntimeException;
	/**
	 * Deletes the data held by the {@org.proclos.etlcore.node.Row} of {@org.proclos.etlcore.node.IColumn Columns} from the persistence layer. Data must be an exact match to be deleted. Otherwise nothing is deleted.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param row the row holding the data columns.
	 */
	public void delete(Locator locator, Row row) throws RuntimeException;
	/**
	 * Updates the data of the persistence layer with new data held by the {@org.proclos.etlcore.node.Row} of {@org.proclos.etlcore.node.IColumn Columns}. Update mechanism is specified by the updateStrategy parameter.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param row the row holding the data columns.
	 */
	public void update(Locator locator, Row keys, Row toSet, Row row) throws RuntimeException;
	/**
	 * Fills an existing table with data from a Processor. The data from the processor must match the table, which always is guaranteed, when {@link #createTable(Locator, Row)} was called first with the same locator.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param rows the Processor delivering the data to be persisted.
	 * @param append append to existing data or recreate with actual data only
	 */
	public void populate(Locator locator, IProcessor rows, boolean append) throws RuntimeException;
	/**
	 * Creates a table according to the specification of a Row defining its structure. If the table exists, it is dropped and recreated.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param columnDefinition the Row serving for the definition of the column names and column data types of the table.
	 * @return a PreparedStatement object capable of inserting single lines of data into the table.
	 */
	public PreparedStatement createTable(Locator locator, Row columnDefinition) throws RuntimeException;

	/**
	 * Gets a table according to the specification of a Row defining its structure. If the table does not exist, it is created.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param columnDefinition the Row serving for the definition of the column names and column data types of the table.
	 * @return a PreparedStatement object capable of inserting single lines of data into the table.
	 */
	public PreparedStatement getTable(Locator locator, Row columnDefinition) throws RuntimeException;

	/**
	 * Drops a table
	 * @param locator Target schema and table- name is determined from the locator
	 */
	public void dropTable(Locator locator) throws RuntimeException;
	/**
	 * renames a table or copies its data if renaming is not supported by implementing backend.
	 * @param source the locator specifying the source name and schema
	 * @param target the locator specifying the destination table name and schema
	 * @param columnDefinition the columns to copy, if renaming is not supported
	 * @param recreateEmptySource if true, the source table is recreated (empty).
	 * @throws RuntimeException
	 */
	public void renameTable(Locator source, Locator target, Row columnDefinition, boolean recreateEmptySource) throws RuntimeException;
	/**
	 * copies table data from source to target. The target table will be created.
	 * @param source the locator specifying the source name and schema
	 * @param target the locator specifying the destination table name and schema
	 * @param columnDefinition the columns to copy
	 * @throws RuntimeException
	 */
	public void copyTable(Locator source, Locator target, Row columnDefinition) throws RuntimeException;

	/**
	 * Determines if a table be be created by a given column Definition and locator.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component providing the data. Target schema and table- name is determined from the locator.
	 * @param columnDefintion the Row serving for the definition of the column names and column data types of the table.
	 * @return true if a table can be created according to the given specification. False otherwise.
	 */
	public boolean isPersistable(Locator locator, Row columnDefintion);
	/**
	 * Gets data from the persistence layer querying the target table with a query.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component reading the data. Target schema and table- name is determined from the locator.
	 * @param query the SQL query string. Please note, that the fields and table name must follow the escape strategy applied at table creation time.
	 * @param size the number of lines to return. Zero for all lines.
	 * @return The processor holding the data.
	 */
	public IProcessor getQueryResult(Locator locator, String query, int size) throws RuntimeException;
	/**
	 * Gets data from the persistence layer querying the whole table (select * ...)
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component reading the data. Target schema and table- name is determined from the locator.
	 * @param size the number of lines to return. Zero for all lines.
	 * @return The processor holding the data.
	 */
	public IProcessor getTableResult(Locator locator, int size) throws RuntimeException;
	/**
	 * Gets the persistent name of a component (schema and table) by its locator. This name should be used as the 'from' token in a query statement when using {@link #getQueryResult(Locator, String, int)}. The appropriate escaping mechanism of the underlying backend is applied.
	 * @param locator locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component. Target schema and table- name is determined from the locator.
	 * @return the persistent name of the object
	 */
	public String getPersistentName(Locator locator) throws RuntimeException;
	/**
	 * Enables or disables the identifier escaping mechanism used by the back-end to escape field names, table names and schema names. Default is true.
	 * @param usePlain escape if true. Do not escape, when false.
	 */
	public void setPlainNames(boolean usePlain);

	/**
	 * Drops a schema. Schema needs to be empty before.
	 * @param locator the {@link com.jedox.etl.core.component.Locator} holding the addressing information of the component using the schema
	 * @throws RuntimeException
	 */
	public void dropSchema(Locator locator) throws RuntimeException;

	/**
	 * Disconnects from the underlying back-end.
	 */
	public void disconnect();
	
	/**
	 * Disconnects from the underlying back-end only for the current thread.
	 */
	public void disconnectThread();
	
	public void setBulkSize(int bulkSize);
	
	public void setLogging(boolean isLogging);
	
	//public void setChildrenMaps(HashMap<String, Map<String, IElement[]>> childrenMaps);
}
