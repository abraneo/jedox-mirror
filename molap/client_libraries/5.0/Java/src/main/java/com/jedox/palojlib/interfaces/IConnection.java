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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.interfaces;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.main.SvsInfo;

/**
 * Interface that represents a connection to server
 * @author khaddadin
 *
 */
public interface IConnection {

	/**
	 * get the connection configuration of this connection. The configuration includes information like host,port,user name,password and timout.
	 * @return connection configuration of this connection
	 */
	public IConnectionConfiguration getConnectionConfiguration();

	/**
	 * get the server info object of this connection. Server info object include information like 	major,minor, bugfix version numbers.
	 * @return server info object  of this connection
	 */
	public IConnectionInfo getServerInfo();

	/**
	 * get the server connection status.
	 * @return true if connection is opened,false otherwise
	 */
	public boolean isConnected();

	/**
	 * open the connection. 
	 * This should be called before working on the connection.
	 * @throws PaloException
	 */
	public void open() throws PaloException;

	/**
	 * close the connection. 
	 * This be done when finished from the connection to expire the olap session.
	 * @throws PaloException
	 */
	public void close() throws PaloException;

	/**
	 * get a list of the databases on this server.
	 * @return list of the databases.
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IDatabase[] getDatabases() throws PaloException, PaloJException;


	/**
	 * create a new database
	 * @param name the name of the database
	 * @return the newly created database
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public IDatabase addDatabase(String name) throws PaloException, PaloJException;

	/**
	 * get database using its name
	 * @param name
	 * @return database object or null if does not exists
	 * @throws PaloException 
	 * @throws PaloJException
	 */
	public IDatabase getDatabaseByName(String name) throws PaloJException, PaloException;
	
	/**
	 * saves a database to disk, but does not save cube logs.
	 * @throws PaloException
	 * @throws PaloJException 
	 */
	public void save() throws PaloException, PaloJException;
	
	/**
	 * get the info of supervision server {@link SvsInfo}
	 * @return svs info 
	 * @throws PaloException
	 * @throws PaloJException
	 */
	public SvsInfo getSvsInfo() throws PaloException, PaloJException;

}
