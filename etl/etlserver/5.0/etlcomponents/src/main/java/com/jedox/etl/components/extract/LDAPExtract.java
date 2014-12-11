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
package com.jedox.etl.components.extract;

import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Types;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;
import java.util.Vector;
import javax.naming.NamingEnumeration;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.DirContext;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;
import javax.sql.rowset.RowSetMetaDataImpl;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.extract.LDAPExtractConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.ILDAPConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;
/**
 *
 * @author gerhard, kais haddadin
 *
 */
public class LDAPExtract extends TableSource implements IExtract {


	private static final Log log = LogFactory.getLog(LDAPExtract.class);

	private class LDAPProcessor extends Processor {

		// These rules are the rules stored in the cube only
		private Row row;
		private int count;// the count of the rules
		Vector<Hashtable<String, Object>> data = null;
		ResultSetMetaData meta = null;

		/**
		 * constructor
		 * @throws RuntimeException
		 */
		public LDAPProcessor(int size) throws RuntimeException {
			try {
				data = getResultInternal(size);
				meta = getMetaData();
				row = PersistenceUtil.getColumnDefinition(meta);
				count = 0;
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to initialize processor "+getName()+": "+e.getMessage());
			}
		}

		/**
		 * read the rules and fill them in a row
		 */
		protected boolean fillRow(Row row) throws Exception {
			// try first to fill the static rules
			if (data.size() > count) {
				for (int i=1; i<=meta.getColumnCount(); i++) {
					Object o = data.get(count).get(meta.getColumnName(i));
					row.getColumn(i-1).setValue(o);
				}
				count++;
				return true;
			}
			else{
				//finished ... do some cleanup
				count = 0;
				return false;
			}

		}

		/**
		 * get row
		 */
		protected Row getRow() {
			return row;
		}
	}

	public LDAPExtract() {
		super();
		setConfigurator(new LDAPExtractConfigurator());
		// TODO what to do here?
		// TODO even make some more convenience constructors which accept various parameters for connection and querying.
	}

	public LDAPExtractConfigurator getConfigurator() {
		return (LDAPExtractConfigurator)super.getConfigurator();
	}

	public ILDAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof ILDAPConnection))
			return (ILDAPConnection) connection;
		throw new RuntimeException("LDAP connection is needed for extract: "+getName()+".");
	}


	private Vector<Hashtable<String, Object>> getResultInternal(int size) throws RuntimeException {
		Vector<Hashtable<String, Object>> data = new Vector<Hashtable<String, Object>>();
		try
		{
			StringBuffer querybuf = new StringBuffer();
			querybuf.append(getConfigurator().getQuery());
			StringBuffer ocbuf = new StringBuffer();

			for (String oc: getConfigurator().getClasses().split(" "))
			{
				ocbuf.append("(objectClass=").append(oc).append(")");
			}
			if (querybuf.length() > 0)
			{
				querybuf.append(ocbuf);
			}
			querybuf.insert(0, "(&").append(")");
			String query = querybuf.toString();

			String searchBase = getConfigurator().getBase();
			int searchScope = getConfigurator().getScope();
			SearchControls sc = new SearchControls();
			sc.setSearchScope(searchScope);
			DirContext initialCtx = getConnection().open();
			NamingEnumeration<SearchResult> bds = initialCtx.search(searchBase, query, sc);
			int numRows = 0;
			int max = (size==0?Integer.MAX_VALUE:size);
			while (bds.hasMore() && numRows< max)
			{
				SearchResult ncp = bds.next();

				Hashtable<String, Object> row = new Hashtable<String, Object>();
				row.put("dn", ncp.getNameInNamespace());
				data.add(row);
				Attributes attrs = ncp.getAttributes();
				NamingEnumeration<? extends Attribute> nea = attrs.getAll();
				while (nea.hasMore()) {
					Attribute attr = nea.next();
					row.put(attr.getID(), attr.get());
				}
				numRows++;
			}
		} catch (Exception e) {
			throw new RuntimeException("Failed to query ldap: "+e.getMessage());
		}
		return data;
	}

	/* (non-Javadoc)
	 * @see java.sql.ResultSet#getMetaData()
	 */
	private ResultSetMetaData getMetaData() throws SQLException {
		// TODO: currently there is distinction of MUST and MAY fields, but it is not used anywhere.
		//       let this distinction be here for now, maybe this will be useful in the future.
		RowSetMetaDataImpl result = new RowSetMetaDataImpl();
		try
		{
			// for local use only to cache results and count overall elements
			Set<String>musts = new HashSet<String>();
			Set<String>mays = new HashSet<String>();

			//for(String schemaName: ((String)environment.get("objectclasses")).split(" "))
			DirContext initialCtx = getConnection().open();
			for(String schemaName: getConfigurator().getClasses().split(" "))
			{
				DirContext schemactx = initialCtx.getSchema("");
				DirContext schema = (DirContext)schemactx.lookup("ClassDefinition/" + schemaName);
				Attributes attrs = schema.getAttributes("");
				// DEBUG
				/*NamingEnumeration<? extends Attribute> bds = attrs.getAll();
				while (bds.hasMore()) {
					Attribute x = bds.next();
					if (x.getID().equals("MAY") || x.getID().equals("MUST"))
						System.out.println(x.toString());
				}*/
				// DEBUG

				if (attrs.get("MUST") != null)
				{
					NamingEnumeration<?> attrValues = attrs.get("MUST").getAll();
					while (attrValues.hasMoreElements())
					{
						musts.add((String)attrValues.nextElement());
					}
				}
				if (attrs.get("MAY") != null)
				{
					NamingEnumeration<?> attrValues = attrs.get("MAY").getAll();
					while (attrValues.hasMoreElements())
					{
						mays.add((String)attrValues.nextElement());
					}
				}
			}

			// TODO: for now just use String and CHAR for ldap fields. (this means, the application has to deal with the contents)
			//       for future, it would be nice to implement at least some sort of character conversion, so that the correct encoding ist written
			//       to the ldap, and on Java side there is always Unicode.
			Set<String> merged = new HashSet<String>(musts);
			merged.addAll(mays);

			result.setColumnCount(merged.size() + 1);
			int i=1;
			for(String attr: merged)
			{
				result.setColumnName(i, attr);
				result.setColumnType(i, Types.CHAR);
				result.setColumnTypeName(i, "java.lang.String");
				i++;
			}
			result.setColumnName(merged.size() + 1, "dn");
			result.setColumnType(merged.size() + 1, Types.CHAR);
			result.setColumnTypeName(merged.size() + 1, "java.lang.String");
		} catch (Exception e)
		{
			log.error("Failed to construct LDAP-MetaData Object:" + e.getMessage());
			log.debug(e);
		}
		return result;
	}

	protected IProcessor getSourceProcessor(int size) throws RuntimeException {

		try {
			LDAPProcessor processor = new LDAPProcessor(size);
			return processor;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to processor source "+getName()+": "+e.getMessage());
		}
	}

	public String rowToString(ResultSet rs) throws SQLException
	{
		StringBuffer buf = new StringBuffer();
		for (int i=1; i <= rs.getMetaData().getColumnCount(); i++)
		{
			buf.append(rs.getMetaData().getColumnName(i)).append(": ");
			buf.append(rs.getString(i)).append(", ");
		}
		return buf.toString();
	}

	public void init() throws InitializationException {
		super.init();

		// TODO: the following attributes are not used anywhere.
		//String attributes = getConfigurator().getAttributes();
		//if (attributes != null) environment.setProperty("listOfAttributes", attributes);
		//environment.setProperty("name", getConfigurator().getName());

	}

}
