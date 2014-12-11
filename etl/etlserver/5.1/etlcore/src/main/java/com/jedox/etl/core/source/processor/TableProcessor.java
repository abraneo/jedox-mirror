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
package com.jedox.etl.core.source.processor;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.RelationalNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.TypeConversionUtil;

import java.sql.ResultSetMetaData;
import java.util.HashMap;
import com.jedox.etl.core.component.RuntimeException;

/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TableProcessor extends Processor {

	private ResultSet rs;
	private Row row = new Row();
	private Statement statement;
	private ResultSetMetaData meta;
	private boolean ignoreInternalKey = true;
	private IAliasMap map;
	private String query;
	private HashMap<Integer,Integer> columnMapping = new HashMap<Integer,Integer>();

	private static final Log log = LogFactory.getLog(TableProcessor.class);

	public TableProcessor(IAliasMap map, ResultSet rs, boolean ignoreInternalKey) {
		this.ignoreInternalKey = ignoreInternalKey;
		this.rs = rs;
		this.map = map;
	}
	
	public TableProcessor(IAliasMap map, Statement statement, String query, boolean ignoreInternalKey) {
		this.ignoreInternalKey = ignoreInternalKey;
		this.statement = statement;
		this.map = map;
		this.query = query;
	}

	private String getColumnName(String name, int pos, IAliasMap map) {
		if (name == null)
			return map.getAlias(pos, "column"+pos);
		return name;
	}

	private boolean isValidColumn(String name) {
		if (isIgnoreInternalKey() && (NamingUtil.internalKeyName().equals(name) || NamingUtil.internalHibernateKeyName().equals(name)))
			return false;
		else 
			return true;
	}
	
	protected void init(IAliasMap map, ResultSetMetaData meta) throws Exception {
		if (map == null)
			map = new AliasMap();
		for (int i=1; i<=meta.getColumnCount();i++) {
			String name = "";
			try{
				name = meta.getColumnLabel(i);
			}catch (Exception e) {}
			if (name.isEmpty())
				name = meta.getColumnName(i);			
			if (isValidColumn(name)) {	
				RelationalNode c = ColumnNodeFactory.getInstance().createRelationalNode(getColumnName(name,i,map),null);
				try{
					if(!meta.getColumnClassName(i).equals("java.lang.Object"))
						c.setValueType(Class.forName(meta.getColumnClassName(i)));
					else
						c.setValueType(TypeConversionUtil.convertSqlTypetoJavaClass((meta.getColumnType(i))));
				}catch(Exception w){
					c.setValueType(null);
				}
				// Sets the scale parameter which represents the number of digits to the right of the decimal point if available
				try{
					c.setScale(meta.getScale(i));
				}catch(Exception w){ 
					c.setScale(0);					
				}
				
				//c.setColumnIndex(i);
	
				//in case there is already a column with the same name (which is OK in some databases, and where statement such "select col1 as col2 .."  is not supported by the driver)
				// the name then should be changed
				if(row.getColumn(c.getName())!= null){
					String newName = c.getName()+ "_Column_"+ i;
					log.warn("Duplicate column name "+c.getName()+" in source "+getName()+". It is renamed to "+newName);
					c.setName(newName);
				}
				row.addColumn(c);
				columnMapping.put(row.indexOf(c), i);
			}
		}
		row.setAliases(map);
	}

	protected ResultSet getResult() throws Exception {
		if (rs == null) {
			rs = statement.executeQuery(query);
		}
		return rs;
	}

	protected boolean fillRow(Row row) throws Exception {
		boolean hasData = getResult().next();
		if (hasData) { //set the data is there is one
			for (int i=0; i<row.size();i++) {
				Object value = null;
				String field = null;
				try {
					field = row.getColumn(i).getName();
					value = getResult().getObject(columnMapping.get(i));
					// for ODBC driver with Unicode types getObject() returns NULL due to a JDBC/ODBC bridge driver issue. getString() has to be called explicitly 
					if (value==null && row.getColumn(i).getValueType().equals(String.class)) {
						try {
							value = getResult().getString(columnMapping.get(i));
						}	
						catch (Exception e2) {
							log.debug("Failed to get String value "+value+" for field "+field+" of source "+getName()+": "+e2.getMessage());
						}
					}			
					row.getColumn(i).setValue(value);
				}
				catch (Exception e) {
					log.debug(e.getMessage());
					String validMessage = NamingUtil.reduceLength(NamingUtil.removeAxis2IllegalCharacters(e.getMessage()));
					log.warn("Failed to set Row value "+value+" for field "+field+" of source "+getName()+": "+ validMessage);
				}
			}
		}
		return hasData;
	}

	/**
	 * Gets the actual Row object filled with the current values of the underlying RowSet
	 * @return the actual Row
	 */
	protected Row getRow() {
		return row;
	}

	public void deleteRow() throws Exception {
		getResult().deleteRow();
	}
	
	public boolean isIgnoreInternalKey() {
		return ignoreInternalKey;
	}

	public void setIgnoreInternalKey(boolean ignoreInternalKey) {
		this.ignoreInternalKey = ignoreInternalKey;
	}

	public void close() {
		super.close();
		if (rs != null) {
			try {
				rs.close();
			} catch (SQLException e) {
				log.warn("Exception while closing the resultset in " + getName() + ": " + e.getMessage());
			}
		}
		if (statement != null) {
			try {
				statement.close();
			} catch (SQLException e) {
				log.warn("Exception while closing the statement in " + getName() + ": " + e.getMessage());
			}
		}
	}

	protected ResultSet getResultSet(Statement stmt, String query) throws Exception, SQLException{
		stmt.execute(query);
		boolean found = false;
		ResultSet rs = stmt.getResultSet();
		if(rs!=null)
			found = true;
		
		while(!found && !((stmt.getMoreResults() == false) && (stmt.getUpdateCount() == -1))){
			ResultSet rsTemp = stmt.getResultSet();
			if(rsTemp!=null){
				rs = rsTemp;
				found = true;
			}
		}
		if (!found) {
			log.debug("SQL Query: "+query);
			throw new Exception("The query was executed successfully, but no result set is returned.");
		}			
		return rs;
	}
	
	@Override
	protected void init() throws RuntimeException {
		try {
			if (rs == null)
				rs = getResultSet(statement,query);
			meta = rs.getMetaData();
			init(map,meta);
		}
		catch (Exception e) {
			close();
			throw new RuntimeException(e.getMessage());
		}
	}

}
