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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.core.connection;

import org.jdom.Element;
import org.jdom.Document;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.XMLReader;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.persistence.hibernate.HibernateUtil;
import com.jedox.etl.core.persistence.mem.MemPersistor;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * The Internal Connection Manager handles all relational connections which are used internally
 */

public class InternalConnectionManager {

	private static InternalConnectionManager instance = null;
	private static final String temporaryConnection = "Temporary";
	private static final String historyConnection = "History";
	private static final String historyLegacyConnection = "History_Legacy";
	private static final String drillthroughConnection = "Drillthrough";
	private ConnectionManager globalConnections;
	

	private InternalConnectionManager() {
		globalConnections = new ConnectionManager();
	}
	
	private static Element getTemporaryInternalConnectionXML(Element orig)
	{
		Element element = (Element)orig.clone();
		Element parameters = element.getChild("parameters");
		if(parameters != null) for(int i = 0; i < parameters.getChildren().size(); i++)
		{
			Element p = (Element)parameters.getChildren().get(i);
			if("#drop".equals(p.getAttributeValue("name")))
			{
				p.setText("false");
			}
		}
		return element;
	}

	/**
	 * gets the singleton instance of this ConfigManager
	 * @return the productive instance
	 */
	public synchronized static final InternalConnectionManager getInstance() throws ConfigurationException {
		if (instance == null) {
			instance = new InternalConnectionManager();
			//read global persistence connections
			XMLReader reader = new XMLReader();
			Document document = reader.readDocument(Settings.getConfigDir()+File.separator+"connections.xml");
			Element root = (Element) document.getRootElement().clone();
			List<?> connections = root.getChildren();
			for (int i=0; i<connections.size(); i++) {
				Element c = (Element) connections.get(i);
				try {
					IConnection connection = instance.globalConnections.add(c);
					//cschw: set #drop to false, since db has been already deleted and connection will be re-instantiated per use and we do not want to delete our own results in this process!
					connection.getConfigurator().setXML(getTemporaryInternalConnectionXML(c), new ArrayList<String>());
				}
				catch (Exception e) {
					throw new ConfigurationException(e);
				}
			}
		}
		return instance;
	}
	
	
	public IRelationalConnection getInternalConnection() throws ConfigurationException {
		return getGlobalConnection(temporaryConnection);
	}
	
	public IRelationalConnection getDrillthroughConnection() throws ConfigurationException {
		return getGlobalConnection(drillthroughConnection);
	}
	
	public IRelationalConnection getPersistenceConnection() throws ConfigurationException {
		return getGlobalConnection(historyConnection);
	}
	
	public IRelationalConnection getPersistenceLegacyConnection() throws ConfigurationException {
		return getGlobalConnection(historyLegacyConnection);
	}
	
	public IRelationalConnection getGlobalConnection(String name) throws ConfigurationException {
		if (name.isEmpty()) //fallback for compatibility with old persistent definitions.
			name = drillthroughConnection;
		IComponent connection = globalConnections.get(name);
		if (connection instanceof IRelationalConnection) {
			return (IRelationalConnection) connection;
		}
		else throw new ConfigurationException("Error in connections.xml. Connection '"+name+"' is not of type relational.");
	}

	/**
	 * Shuts this manager down and does cleanup operations.
	 */
	public void shutDown() {
		globalConnections.disconnect();
		MemPersistor.close();
		Executor.getInstance().shutdown();
		HibernateUtil.shutdown();
	}

}
