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

import org.jdom.Element;
import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;




/**
 * Manager Class for Connection Management
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ConnectionManager extends Manager {

	public static final String palo = "palo";
	//private static final Log log = LogFactory.getLog(ConnectionManager.class);

	//private Hashtable<String, IConnection> lookup = new Hashtable<String, IConnection>();
	//private Hashtable<String, Element> configs = new Hashtable<String, Element>();

	/**
	 * adds a connection
	 */
	public IConnection add(IComponent connection) throws RuntimeException {
		if (connection instanceof IConnection) {
			return (IConnection) super.add(connection);
		}
		throw new RuntimeException("Failed to add non-Connection object.");
	}

	/**
	 * gets a connection by its name
	 */
	public IConnection get(String name) {
		return (IConnection) super.get(name);
	}

	/**
	 * gets all connections of this manager
	 */
	public IConnection[] getAll() {
		IComponent[] components = super.getAll();
		IConnection[] connections = new IConnection[components.length];
		for (int i=0; i<components.length; i++) {
			connections[i] = (IConnection) components[i];
		}
		return connections;
	}

	/**
	 * removes a connection from this manager
	 */
	public IConnection remove(String name) {
		return (IConnection) super.remove(name);
	}

	/**
	 * closes all open connections.
	 *
	 */
	public void disconnect() {
		//disconnect from internal connection
		//Persistence.getInstance().disconnect();
		//disconnect from sitemap connections
		try {
			for (IConnection connection : getAll()) {
				connection.close();
			}
		}
		catch (Exception e) {}
		clear();
	}

	/**
	 * delegates the creation of a connection from an XML config to the {@link com.jedox.etl.core.component.ComponentFactory ComponentFactory}
	 */
	public IConnection add(Element config) throws CreationException, RuntimeException {
		IConnection connection = ComponentFactory.getInstance().createConnection(config.getAttributeValue("type"), this, getContext(), config);
		add(connection);
		return connection;
	}


	public String getName() {
		return ITypes.Connections;
	}

}
