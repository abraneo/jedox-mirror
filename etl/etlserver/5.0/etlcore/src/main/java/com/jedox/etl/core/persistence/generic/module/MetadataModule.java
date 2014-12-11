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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.persistence.generic.module;

import java.sql.DatabaseMetaData;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Hashtable;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;

public class MetadataModule extends PersistenceModule {
	
	private Hashtable<String, String> dataTypeLookup = new Hashtable<String, String>();
	private Hashtable<String, Integer> sqlConstantLookup = new Hashtable<String, Integer>();
	private Hashtable<String, String> dbTypeLookup = new Hashtable<String, String>();
	private String databaseName;
	private String quote;
	
	private static final Log log = new MessageHandler(LogFactory.getLog(MetadataModule.class));
	
	public MetadataModule(DatabaseMetaData meta, Map<String,String> lookup) throws RuntimeException {
		dataTypeLookup.putAll(lookup);
		//initialize the db type lookup (java type to db type)
		for (String javaType : lookup.keySet()) {
			//get DB Type without precision quatifiers
			String dbmsName = lookup.get(javaType);
			int pos = dbmsName.indexOf("(");
			if (pos > 0)
				dbmsName = dbmsName.substring(0, pos);
			dbTypeLookup.put(javaType, dbmsName.toLowerCase().trim());
		}
		try {
			databaseName = meta.getDatabaseProductName();
			quote = meta.getIdentifierQuoteString();
			//initialize the sql type lookup (db type to sql type code) table used for fillStatement
			ResultSet rs = meta.getTypeInfo();
			//System.err.println(this.getClass().getCanonicalName());
			while (rs.next()) {
			   int codeNumber = rs.getInt("DATA_TYPE");
			   String dbmsName = rs.getString("TYPE_NAME");
			   //System.err.println(codeNumber + " " +dbmsName);
			   sqlConstantLookup.put(dbmsName.toLowerCase().trim(), codeNumber);
			}
		}
		catch (SQLException e) {
			throw new RuntimeException(e.getMessage());
		}
	}
	
	public String getDataType(String javaName, int scale) {
		String result = dataTypeLookup.get(javaName);
		if (result != null)
			result = result.replaceAll("\\?",""+scale);
		if (result == null) {
			log.warn("Java type "+javaName+" not mapped by adaptor for backend "+databaseName);
			result = dataTypeLookup.get("default");
			if (result == null)
				log.error("Adaptor for backend "+databaseName+" does not provide a default type mapping.");
		}
		return result;
	}
	
	public Integer getSQLConstant(String javaName) {
		String defaultType = dbTypeLookup.get("default");
		String type = dbTypeLookup.get(javaName);
		if (type == null) {
			log.warn("Java type "+javaName+" not mapped by adaptor for backend "+databaseName);
			type = defaultType;
			if (type == null) {
				log.error("Adaptor for backend "+databaseName+" does not provide a default type mapping.");
				return null;
			}
		}
		Integer defaultSQLType = sqlConstantLookup.get(defaultType); //default fallback type, if lookup fails
		Integer sqlType = sqlConstantLookup.get(type);
		if (sqlType == null) {//fallback if there is no mapping
			log.warn("Java type "+javaName+" not mapped by adaptor for backend "+databaseName);
			sqlType = defaultSQLType;
			if (sqlType == null) {
				log.error("Adaptor for backend "+databaseName+" does not provide a default type mapping.");
			}
		}
		return sqlType;
	}
	
	public String getQuote() {
		return quote;
	}

}
