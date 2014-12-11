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
package com.jedox.etl.components.config.connection;

import com.jedox.etl.core.config.connection.ConnectionConfigurator;
import com.jedox.etl.core.component.ConfigurationException;


public class OLAPGlobalConnectionConfigurator extends ConnectionConfigurator {

	/**
	 * gets the reference of the Palo connection in the Palo Suite.
	 * @return the localization
	 * @throws ConfigurationException
	 */
	public String getGlobalReference() throws ConfigurationException {
		return getParameter("globalReference","");
	}

	/**
	 * gets the config path to the Palo Suite which stores the connection data of the central Palo OLAP Server instance
	 * @return the localization
	 * @throws ConfigurationException
	 */
	public String getConfigPath() throws ConfigurationException {
		return getParameter("ConfigPath","");
	}
		
}