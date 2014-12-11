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
 *   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.etl.components.connection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.connection.OLAPGlobalConnectionConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.palojlib.premium.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.main.ClientInfo;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.premium.main.ConnectionManager;

public class OLAPGlobalConnection extends OLAPConnection implements IOLAPConnection {

	private static final Log log = LogFactory.getLog(OLAPGlobalConnection.class);
	private int timeout;
	private String globalReference; // used only in PaloSuite Connection
	private IConnectionConfiguration connectionConfiguration;

	public OLAPGlobalConnection() {
		setConfigurator(new OLAPGlobalConnectionConfigurator());
	}

	public OLAPGlobalConnectionConfigurator getConfigurator() {
		return (OLAPGlobalConnectionConfigurator)super.getConfigurator();
	}

	protected IConnection connect2OLAP() throws RuntimeException {

		try {
			log.debug("Opening connection "+getName());

			IConnection palosuiteConnection = ConnectionManager.getInstance().getConnection(connectionConfiguration);
			palosuiteConnection.openInternal();
			log.debug("Connection to Palo Suite OLAP Server on "+connectionConfiguration.getHost()+" on port "+connectionConfiguration.getPort()+" is open.");

			IDatabase configdb = palosuiteConnection.getDatabaseByName("Config");
			if (configdb == null)
				throw new RuntimeException("Database Config not found.");
			IDimension connectionHierarchy = configdb.getDimensionByName("connections");
			if (connectionHierarchy == null)
				throw new RuntimeException("Dimension connections not found.");
			IElement connectionProperties = connectionHierarchy.getElementByName(globalReference,true);
			if(connectionProperties == null)
				throw new RuntimeException("Connection " + globalReference + " is not found.");

			String globalConnTypet = (String) connectionProperties.getAttributeValue("type");

			if(!globalConnTypet.equals("palo"))
				throw new RuntimeException("Connection " + getDatabase() + " is not a Palo Connection.");

			String globalConnHost = (String) connectionProperties.getAttributeValue("host");
			String globalConnPort = (String) connectionProperties.getAttributeValue("port");
			String globalConnUsername = (String) connectionProperties.getAttributeValue("username");
			String globalConnPassword = Settings.getInstance().decrypt((String) connectionProperties.getAttributeValue("password"));
			int globalConnActive = Integer.parseInt((String) connectionProperties.getAttributeValue("active"));
			int useLoginCred = Integer.parseInt((String) connectionProperties.getAttributeValue("useLoginCred"));
			palosuiteConnection.close();

			if(globalConnActive != 1)
				throw new RuntimeException("Connection " + getDatabase() + " is inactive and therefor can not be used.");
			if(useLoginCred == 1)
				log.info("Login credentials of connection " + getDatabase() + " will be ignored. A connection with fix username should be used.");

			IConnectionConfiguration cc = new ConnectionConfiguration();
			cc.setHost(globalConnHost);
			cc.setPort(globalConnPort);
			cc.setUsername(globalConnUsername);
			cc.setPassword(globalConnPassword);
			cc.setTimeout(timeout*1000);
			cc.setSslPreferred(true);
			cc.setClientInfo(new ClientInfo("Jedox ETL","connection " + getName() + " in " + getLocator().getRootName()) );
			connection = ConnectionManager.getInstance().getConnection(cc);
			connection.openInternal();
			setVersion(connection);
			log.debug("OLAP Connection Timeout set to "+timeout+" seconds.");
			return connection;

		} catch (Exception e) {
			throw new RuntimeException("Failed to open OLAP connection "+getName()+". Please check your connection specification. "+e.getMessage());
		}
	}

	public void init() throws InitializationException {
		try {
			super.init();
			globalReference = getConfigurator().getGlobalReference();
			timeout = getConfigurator().getTimeout();
			connectionConfiguration = Settings.getInstance().getSuiteConnectionConfiguration();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
