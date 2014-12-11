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
package com.jedox.etl.components.load;

import java.util.HashMap;
import java.util.HashSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.load.DimensionConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;

public abstract class AbstractDimensionLoad extends Load {

	private static final Log log = LogFactory.getLog(AbstractDimensionLoad.class);
	private String dimensionName;

	public DimensionConfigurator getConfigurator() {
		return (DimensionConfigurator)super.getConfigurator();
	}

	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		throw new RuntimeException("OLAP connection is needed for source "+getName()+".");
	}


	protected String getDimensionName() {
		return dimensionName;
	}

	protected String getDatabaseName() throws RuntimeException {
		return getConnection().getDatabase();
	}

	protected IDatabase getDatabase() throws RuntimeException {
		IDatabase database = null;
		try {
			com.jedox.palojlib.interfaces.IConnection conn= (com.jedox.palojlib.interfaces.IConnection) getConnection().open();
			database = conn.getDatabaseByName(getDatabaseName());
		}
		catch (Exception e) {
			log.error(e.getMessage());
		}
		if (database == null) {
			log.error("Database "+getDatabaseName()+" not found via connection "+getConnection().getName());
		}
		return database;
	}

	/**
	 * gets the dimension object
	 * @return
	 */
	protected IDimension getDimension() throws RuntimeException {
		IDatabase database = getDatabase();
		IDimension dimension =  null;
		if (database != null)
			dimension = getDatabase().getDimensionByName(getDimensionName());
		if (dimension == null) {
			log.error("Dimension "+getDimensionName()+" not found in database "+getDatabaseName());
		}
		return dimension;
	}

	private HashSet<IElement> getDescendants(IElement e, HashSet<IElement> set) {
		set.add(e);
		for (IElement c : e.getChildren()) {
			getDescendants(c,set);
		}
		return set;
	}

	protected String getLookupName(String name) {
		return (name == null) ? null : name.toLowerCase();
	}

	protected HashMap<String, IElement> getElementsInDimension(IDimension dim, String root) {
		HashMap<String, IElement> present = new HashMap<String, IElement>();
		IElement[] oldElements = new IElement[0];
		if (root != null) {
			IElement element = dim.getElementByName(root,false);
			if (element != null) {
				HashSet<IElement> descendants = getDescendants(element, new HashSet<IElement>());
				oldElements = descendants.toArray(new IElement[descendants.size()]);
			}
			else
				log.error(getName()+": Dimension root "+root+" not found in Dimension "+getName());
		}
		else
			oldElements = dim.getElements(false);
		for (int i=0;i<oldElements.length;i++) {
			IElement element = (IElement) oldElements[i];
			present.put(getLookupName(element.getName()),element);
		}
		return present;
	}

	public void init() throws InitializationException {
		try {
			super.init();
			dimensionName = getConfigurator().getDimensionName();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
