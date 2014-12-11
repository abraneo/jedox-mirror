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
import java.sql.Connection;
import java.util.Date;
import java.sql.Time;
import java.sql.Timestamp;
import java.util.Hashtable;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.persistence.generic.GenericPersistor;

public class Sqlserver extends GenericPersistor {

	//external connection
    private IRelationalConnection connection;
    //private String database;
    
    public Sqlserver(IConnection connection) {
    	if (connection instanceof IRelationalConnection)
    		this.connection = (IRelationalConnection)connection;
    }
    
	@Override
	protected Connection setupConnection() throws RuntimeException {
		if (connection != null){
			//database = connection.getDatabase();
			return connection.open();
		}
		return null;
	}
	
	protected Hashtable<String, String> getLookUp() {
		Hashtable<String, String> lookup = new Hashtable<String, String>();
		lookup.put(String.class.getCanonicalName(), "NVARCHAR(4000)");
		// For SQL Server > 2005: NVARCHAR(MAX) is available as type 
		lookup.put(Integer.class.getCanonicalName(), "INT");
		lookup.put(Double.class.getCanonicalName(), "FLOAT");
		lookup.put(Float.class.getCanonicalName(), "REAL");
		lookup.put(Date.class.getCanonicalName(), "DATE");
		lookup.put(Time.class.getCanonicalName(), "TIME(7)");
		lookup.put(Timestamp.class.getCanonicalName(), "DATETIME");
		lookup.put(BigDecimal.class.getCanonicalName(), "DECIMAL(18,?)");
		lookup.put(Boolean.class.getCanonicalName(), "BIT");
		lookup.put(Byte.class.getCanonicalName(), "BINARY(50)");
		lookup.put(Short.class.getCanonicalName(), "SMALLINT");
		lookup.put(Long.class.getCanonicalName(), "BIGINT");
		lookup.put(Object.class.getCanonicalName(), "VARBINARY(8000)");
		lookup.put(byte[].class.getCanonicalName(), "VARBINARY(8000)");
		lookup.put("default", "NVARCHAR(4000)");
		return lookup;
	}
	
	protected String getGeneratedKeySyntax() {
		return "INT IDENTITY(0,1) NOT NULL";
	}
	

}
