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
package com.jedox.etl.core.persistence.generic.adaptor;

import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.Date;
import java.sql.Time;
import java.sql.Timestamp;
import java.util.Hashtable;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

/**
 * @author khaddadin
 *
 */
public class H2 extends GenericPersistor {
	
	private IConnection connection;
	
	public H2(IConnection connection){
		this.connection = connection;
	}

	@Override
	protected Connection setupConnection() throws RuntimeException {
		
		if (connection instanceof IRelationalConnection)
			return ((IRelationalConnection)connection).open();
		else
			throw new RuntimeException("H2 connection could not be done.");
		
		
	}

	@Override
	protected String getGeneratedKeySyntax() {
		return "INTEGER PRIMARY KEY AUTO_INCREMENT";
	}

	@Override
	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "VARCHAR (32672)");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "DOUBLE");
		lookup.put(Date.class.getCanonicalName(), "DATE");
		lookup.put(Time.class.getCanonicalName(), "TIME");
		lookup.put(Timestamp.class.getCanonicalName(), "TIMESTAMP");
		lookup.put(BigDecimal.class.getCanonicalName(), "NUMERIC(30,?)");
		lookup.put(Boolean.class.getCanonicalName(), "CHAR () FOR BIT DATA");
		lookup.put(Byte.class.getCanonicalName(), "SMALLINT");
		lookup.put(Short.class.getCanonicalName(), "SMALLINT");
		lookup.put(Long.class.getCanonicalName(), "BIGINT");
		lookup.put(Float.class.getCanonicalName(), "FLOAT");	
		lookup.put(Object.class.getCanonicalName(), "LONG VARCHAR FOR BIT DATA");
		lookup.put(byte[].class.getCanonicalName(), "LONG VARCHAR FOR BIT DATA");
		lookup.put("default", "VARCHAR (32672)");
		return lookup;
	}
	
	public void disconnectThread(){
		connection.close();
	}

}
