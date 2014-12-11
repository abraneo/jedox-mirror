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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.util.Properties;

import javax.naming.Context;
import javax.naming.NamingException;
import javax.naming.directory.DirContext;
import javax.naming.directory.InitialDirContext;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.ILDAPConnection;
import com.jedox.etl.core.connection.MetadataCriteria;

public class LDAPConnection extends Connection implements ILDAPConnection {
	
	DirContext initialCtx = null;
	private static final Log log = LogFactory.getLog(LDAPConnection.class);
	
	private String getConnectionString() {
		StringBuffer p = new StringBuffer(); 
		p.append(getDBType()+"://");
		String host = getHost();
		if (host == null) host = "localhost";
		p.append(host);
		String port = getPort();
		if (port == null) port = "389";
		p.append(":"+port+"/");
		return p.toString();
	}
	
	/**
	 * Internal connection creation
	 * @param prop
	 * @return
	 */
	private DirContext connect2LDAP() throws RuntimeException {	
		log.debug("Opening connection "+getName());
		Properties environment = new Properties();
		environment.setProperty(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
		environment.setProperty(Context.PROVIDER_URL, getConnectionString());
		environment.setProperty(Context.SECURITY_AUTHENTICATION, "simple");
		environment.setProperty(Context.SECURITY_PRINCIPAL, getUser());
		environment.setProperty(Context.SECURITY_CREDENTIALS, getPassword());
		try
		{
			DirContext context = new InitialDirContext(environment);
			log.debug("Connection "+getName()+" is open.");
			return context;
		} catch (NamingException e)
		{
			throw new RuntimeException("Failed to open LDAP connection: "+getName()+". Please check your connection specification. "+e.getMessage());
		}
	}
	
	public DirContext open() throws RuntimeException {
		if (initialCtx == null) initialCtx = connect2LDAP();
		return initialCtx;
	}
	
	public void close() {
		log.debug("Closing connection "+getName());
		try {
			if (!isKept() && (initialCtx != null)) {
				initialCtx.close();
				initialCtx = null;
			}
		}
		catch (Exception e) {};
	}
	
	public String getServerName() {
		return "ldap";
	}

	public MetadataCriteria[] getMetadataCriterias() {
		return null;
	}
		
	public String getMetadata(Properties properties) throws RuntimeException {
		throw new RuntimeException("Not implemented in "+this.getClass().getCanonicalName());
	}


	
}
