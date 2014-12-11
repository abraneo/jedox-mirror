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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.service;

/**
 * SOAP Transport class for component type meta data.
 *
 */
public class DrillthroughInfoDescriptor {

	private String connector;
	private String datastore;
	private String tablename;
	private String schemaname;
	private String connectorType;
	
		
	public String getConnector() {
		return connector;
	}
	public void setConnector(String connector) {
		this.connector = connector;
	}
	public String getDatastore() {
		return datastore;
	}
	public void setDatastore(String datastore) {
		this.datastore = datastore;
	}
	public String getTablename() {
		return (tablename==null) ? "" :  tablename;
	}
	public void setTablename(String tablename) {
		this.tablename = tablename;
	}
	public String getSchemaname() {
		return (schemaname==null) ? "" : schemaname;
	}
	public void setSchemaname(String schemaname) {
		this.schemaname = schemaname;
	}
	public String getConnectorType() {
		return connectorType;
	}
	public void setConnectorType(String connectorType) {
		this.connectorType = connectorType;
	}
	
	
	
}
