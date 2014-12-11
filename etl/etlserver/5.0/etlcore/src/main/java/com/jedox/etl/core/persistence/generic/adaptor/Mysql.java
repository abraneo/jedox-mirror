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
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Time;
import java.util.Hashtable;

import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

public class Mysql extends GenericPersistor {

	//external connection
    private IRelationalConnection connection;
    
    public Mysql(IConnection connection) {
    	if (connection instanceof IRelationalConnection)
    		this.connection = (IRelationalConnection)connection;
    }
    
	@Override
	protected Connection setupConnection() throws RuntimeException {
		if (connection != null)
			return connection.open();
		return null;
	}
	
	public String getPersistentName(Locator locator) throws RuntimeException {
		return escapeName(locator.getName());
	}
	
	protected int getMaxRows() {
		return 50000000;
	}
	
	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "TEXT");
		lookup.put(Integer.class.getCanonicalName(), "INTEGER");
		lookup.put(Double.class.getCanonicalName(), "DOUBLE");
		lookup.put(Date.class.getCanonicalName(), "DATE");
		lookup.put(Time.class.getCanonicalName(), "TIME");
		lookup.put(Timestamp.class.getCanonicalName(), "TIMESTAMP");
		lookup.put(BigDecimal.class.getCanonicalName(), "NUMERIC(30,?)");
		lookup.put(Boolean.class.getCanonicalName(), "CHAR");
		lookup.put(Byte.class.getCanonicalName(), "TINYINT");
		lookup.put(Short.class.getCanonicalName(), "SMALLINT");
		lookup.put(Long.class.getCanonicalName(), "BIGINT");
		lookup.put(Object.class.getCanonicalName(), "VARBINARY");
		lookup.put(byte[].class.getCanonicalName(), "VARBINARY");
		lookup.put("default", "TEXT");
		return lookup;
	}

	protected String getGeneratedKeySyntax() {
		return "int auto_increment not null PRIMARY KEY";
	}
	
	public void renameTable(Locator source, Locator target, Row columnDefinition, boolean recreateEmptySource) throws RuntimeException {
		String sourceName = getPersistentName(source);
		String targetName = getPersistentName(target);
		dropTable(target);
		try {
			Statement stm = getConnection().createStatement();
			String sql = "RENAME TABLE "+sourceName+" TO "+targetName;
			stm.execute(sql);
		}
		catch (SQLException e) {
			throw new RuntimeException("Failed to copy data from "+source+" to "+"target: "+e.getMessage());
		}
		if (recreateEmptySource)
			createTable(source, columnDefinition);
	}

}
