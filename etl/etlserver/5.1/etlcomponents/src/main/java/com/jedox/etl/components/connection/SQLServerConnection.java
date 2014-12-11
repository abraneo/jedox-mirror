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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.sql.SQLException;
import java.sql.Statement;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.TableProcessor;

public class SQLServerConnection extends RelationalConnection {
	
	protected String getConnectionString(String parameter) {
		String connectionString = getProtocol();
		if (getHost() != null) connectionString += "//"+getHost();
		if (getPort() != null) connectionString += ":"+getPort();
		if (getHost() != null) connectionString += ";database=";
		connectionString += getDatabase();
		if (parameter.length() > 0) {
			connectionString += ";"+parameter;
		}
		return connectionString;
	}
	
	public Statement createStatement(int size) throws SQLException, RuntimeException {
		Statement statement;
			switch (getFetchMode()) {
			case BUFFERED: {
				getConnectionParameters().setProperty("responseBuffering", "adaptive");
				statement = open().createStatement();
				break;
			}  
			default: {
				getConnectionParameters().setProperty("responseBuffering", "full");
				statement = open().createStatement();
			}
			}
		if (size != 0) statement.setMaxRows(size);
		return statement;
	}
	
	public IProcessor getProcessor(String query, IAliasMap aliasMap, Boolean onlyHeader, Boolean ignoreInternalKey, int size) throws RuntimeException {
		// done because of ticket 19722, setMaxRows does cause that no data comes for some stored procedures
		if(query.trim().toLowerCase().startsWith("exec")) {
			IProcessor processor = super.getProcessor(query, aliasMap, onlyHeader, ignoreInternalKey, 0);
			processor.setLastRow(size);
			return processor;
		} else {
			return super.getProcessor(query, aliasMap, onlyHeader, ignoreInternalKey, size);
		}
	}
	
	
	public boolean allowInRelationalSP() {
		return true;
	}
	
	/*public void init() throws InitializationException {
		super.init();
		//drop database if needed
		if (getConnectionParameters().getProperty(NamingUtil.internal("drop"), "false").equalsIgnoreCase("true")) {
			ResultSet tableRs;
			try {
				tableRs = open().getMetaData().getTables(getDatabase(), null, null, new String[]{"TABLE"});
				open().getCatalog();
				PreparedStatement stmt = open().prepareStatement("DROP TABLE ?");
				while (tableRs.next()) {
					stmt.setString(1, tableRs.getString(2)+"."+tableRs.getString(3));
					stmt.execute();
				}
				open().commit();
			} catch (Exception e) {
				System.out.print(e.getMessage());
				
			} 
		}
	}*/

}
