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
 *   You may obtain a copy of the License at
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

package com.jedox.palojlib.test;

import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.main.ConnectionConfiguration;
import com.jedox.palojlib.main.ConnectionManager;
import com.jedox.palojlib.main.Cube;
import com.jedox.palojlib.main.SessionConfiguration;

/**
 * @author khaddadin
 *
 */
public class TestAdoptSession {

	public static void main(String[] args) {
		
		IConnectionConfiguration oldConfig = new ConnectionConfiguration();
		TestSettings.getInstance().setConfigFromFile(oldConfig);
		TestSettings.getInstance().setSSL();
		//TestSettings.getInstance().setDebugLevel();
		
	
		//IConnection con = ConnectionManager.getInstance().getConnection(oldConfig);
		//String session = con.open();
		//System.out.println(session);
		
		String session = "OLAP_SESSION_STR";
		
		SessionConfiguration config = new SessionConfiguration();
		config.setHost(oldConfig.getHost());
		config.setPort(oldConfig.getPort());
		config.setSession(session);
		config.setSslPreferred(oldConfig.isSslPreferred());
		config.setTimeout(oldConfig.getTimeout());		
		
		IConnection con2 = ConnectionManager.getInstance().getConnection(config);
		con2.open();
		IDatabase[] dbs = con2.getDatabases();
		dbs[0].getCubes();
		
		//con.close(true);
		try {
			// wait until the cache expires
			Thread.sleep(Cube.defaultExpiryDuration*1000 +1);
			(con2.getDatabases()[0]).getCubeByName("USER");
		} catch (Exception e) {
			// the connection is already closed, so an exception is done here
			e.printStackTrace();
			
		}

	}

}
