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

public class MySQLConnection extends RelationalConnection {
	
	protected String getConnectionString(String parametersStr) {
		String connectionString = getProtocol();
		if (getHost() != null) connectionString += "//"+getHost();
		if (getPort() != null) connectionString += ":"+getPort();
		if (getHost() != null) connectionString += "/";
		connectionString += getDatabase() + "?";
		// Initial date values return NULL instead of an Exception
		if (!parametersStr.contains("zeroDateTimeBehavior=")) {
			connectionString += "zeroDateTimeBehavior=convertToNull&";
		}	
		if (!parametersStr.isEmpty()) {
			connectionString += parametersStr.replace(";", "&");
		}
		return connectionString;
	}
	
	public Statement createStatement(int size) throws SQLException, RuntimeException {
		Statement statement;
		switch (getFetchMode()) {
		case BUFFERED: {
			statement = open().createStatement(java.sql.ResultSet.TYPE_FORWARD_ONLY,java.sql.ResultSet.CONCUR_READ_ONLY);
			statement.setFetchSize(Integer.MIN_VALUE);
			break;
		}  
		default: statement = open().createStatement();
		}
		if (size>50000000)
			statement.setMaxRows(0);
		else
			statement.setMaxRows(size);
		return statement;
	}

	public IProcessor getProcessor(String query, IAliasMap aliasMap, Boolean onlyHeader, Boolean ignoreInternalKey, int size) throws RuntimeException {
		IProcessor processor = super.getProcessor(query, aliasMap, onlyHeader, ignoreInternalKey, size);
		// In MySQL sql_select_limit does not apply to SELECT statements executed within stored routines
		// so that statement.setMaxRows(size) is ignored
		// Explicit limitation of processor is therefore necessary
		processor.setLastRow(size);
		return processor;
	}
	
	
	public boolean isSchemaSupported () {
		return false;
	}
	
	public boolean allowInRelationalSP() {
		return true;
	}
	
}
