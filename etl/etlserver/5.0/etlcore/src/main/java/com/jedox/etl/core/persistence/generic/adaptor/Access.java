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
package com.jedox.etl.core.persistence.generic.adaptor;

import java.math.BigDecimal;
import java.sql.Timestamp;
import java.sql.Connection;
import java.util.Date;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Time;
import java.text.NumberFormat;
import java.util.Hashtable;
import java.util.Locale;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

public class Access extends GenericPersistor {

	//external connection
	private IRelationalConnection connection;
	private String localName = "";
	//private String database;

	public Access(IConnection connection) {
		if (connection instanceof IRelationalConnection)
			this.connection = (IRelationalConnection)connection;
		try {
			localName = connection.getConfigurator().getParameter("WindowsLocale", "");
			getLog().debug("Locale " + localName + " will be applied, in case it does not exist, english language will be used.");
		} catch (ConfigurationException e) {
		}
	}

	@Override
	protected Connection setupConnection() throws RuntimeException {
		if (connection != null){
			return connection.open();
		}
		return null;
	}

	public String getPersistentName(Locator locator) throws RuntimeException {
		return escapeName(locator.getName());
		//return database+ ".dbo." + locator.getName();
	}

	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "TEXT");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "DOUBLE");
		lookup.put(Float.class.getCanonicalName(), "SINGLE");
		lookup.put(Date.class.getCanonicalName(), "DATE/TIME");
		lookup.put(Time.class.getCanonicalName(), "DATE/TIME");
		lookup.put(Timestamp.class.getCanonicalName(), "LONG");
		lookup.put(BigDecimal.class.getCanonicalName(), "SINGLE");
		lookup.put(Boolean.class.getCanonicalName(), "YES/NO");
		lookup.put(Byte.class.getCanonicalName(), "BYTE");
		lookup.put(Short.class.getCanonicalName(), "INTEGER");
		lookup.put(Long.class.getCanonicalName(), "LONG");
		lookup.put(Object.class.getCanonicalName(), "OLE OBJECT");
		lookup.put(byte[].class.getCanonicalName(), "OLE OBJECT");
		lookup.put("default", "TEXT");
		return lookup;
	}

	protected String getGeneratedKeySyntax() {
		return " AUTOINCREMENT NOT NULL";
	}

	protected void fillStatement(PreparedStatement statement, IColumn column, int pos) throws RuntimeException {
		try {

			if(column.getValueType().equals("java.lang.Double") && !localName.equals("")){

				String localizedValue = NumberFormat.getNumberInstance(new Locale(localName)).format(Double.parseDouble(column.getValueAsString()));
				statement.setString(pos, localizedValue);
			}
			else{
				statement.setString(pos, column.getValueAsString());
			}
		}
		catch (SQLException e) {
			throw new RuntimeException(e);
		}
	}
}
