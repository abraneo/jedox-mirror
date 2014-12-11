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

package com.jedox.etl.core.persistence.generic;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.persistence.IPersistence;
import com.jedox.etl.core.persistence.generic.adaptor.Access;
import com.jedox.etl.core.persistence.generic.adaptor.AccessJdbc;
import com.jedox.etl.core.persistence.generic.adaptor.Derby;
import com.jedox.etl.core.persistence.generic.adaptor.H2;
import com.jedox.etl.core.persistence.generic.adaptor.Hsqldb;
import com.jedox.etl.core.persistence.generic.adaptor.Mysql;
import com.jedox.etl.core.persistence.generic.adaptor.Oracle;
import com.jedox.etl.core.persistence.generic.adaptor.Postgres;
import com.jedox.etl.core.persistence.generic.adaptor.Sqlite;
import com.jedox.etl.core.persistence.generic.adaptor.Sqlserver;

/**
 * Allows the access to different persistence back-ends.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class PersistenceManager {

	PersistenceManager() {
	}

	/**
	 * gets the specific persistence implementation for a given back-end
	 * @param connection the connection to the back-end
	 * @return the specific persistence back-end
	 * @throws RuntimeException
	 */
	public static IPersistence getPersistence(IConnection connection) throws RuntimeException {
		String server = connection.getServerName();
		//TODO: for the generic db this will not work, It should use the generic persistence
		if (server.equalsIgnoreCase("access"))
			return new Access(connection);
		// Persistence to Odbc is deprecated. For MS Access specific connection type
		//if (server.equalsIgnoreCase("odbc"))
		//	return new Access(connection);		
		if (server.equalsIgnoreCase("mysql"))
			return new Mysql(connection);
		if (server.equalsIgnoreCase("postgresql"))
			return new Postgres(connection);
		if (server.equalsIgnoreCase("sqlserver"))
			return new Sqlserver(connection);
		if (server.equalsIgnoreCase("derby"))
			return new Derby(connection);
		if (server.equalsIgnoreCase("hsqldb"))
			return new Hsqldb(connection);
		if (server.equalsIgnoreCase("oracle"))
			return new Oracle(connection);
		if (server.equalsIgnoreCase("h2"))
			return new H2(connection);
		if (server.equalsIgnoreCase("sqlite"))
			return new Sqlite(connection);
		if (server.equalsIgnoreCase("drillthrough"))
			return new Derby(connection);
		if (server.equalsIgnoreCase("internal"))
			return new Derby(connection);
		if (server.equalsIgnoreCase("accessjdbc"))
			return new AccessJdbc(connection);
		throw new RuntimeException("Loading data to Relational Database "+server+" not supported.");
	}

}
