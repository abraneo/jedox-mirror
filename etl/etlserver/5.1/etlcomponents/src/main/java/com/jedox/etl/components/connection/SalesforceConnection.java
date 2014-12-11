/**
*   @brief <Description of Class>
*  
*   @file
*  
*   Copyright (C) 2008-2014 Jedox AG
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

import java.io.StringWriter;
import java.net.ConnectException;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Properties;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.connection.SalesforceConnectionConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.MetadataWriter;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;
import com.sforce.soap.partner.DescribeGlobalResult;
import com.sforce.soap.partner.DescribeGlobalSObjectResult;
import com.sforce.soap.partner.DescribeSObjectResult;
//import com.sforce.soap.partner.PackageVersion;
import com.sforce.soap.partner.PartnerConnection;
import com.sforce.ws.ConnectionException;
import com.sforce.ws.ConnectorConfig;

/**
 * 
 * @author kais.haddadin@jedox.com
 *
 */

public class SalesforceConnection  extends Connection implements IConnection {

	private ConnectorConfig config = new ConnectorConfig();
	private PartnerConnection partnerConnection = null;
	private SSLModes sslMode;
	
	private static final Log log = LogFactory.getLog(SalesforceConnection.class);
	
	public enum MetadataSelectors {
		table,field
	}
	
	public enum MetadataFilters {
		tableName
	}

	public SalesforceConnection() {
		setConfigurator(new SalesforceConnectionConfigurator());
	}

	public SalesforceConnectionConfigurator getConfigurator() {
		return (SalesforceConnectionConfigurator)super.getConfigurator();
	}
	
	@Override
	public Object open() throws RuntimeException {
		if (partnerConnection == null){
			checkSSL();
			try {
				partnerConnection = new PartnerConnection(config);
				/*PackageVersion pk = new PackageVersion();
				pk.setMajorNumber(30);
				pk.setMinorNumber(0);
				partnerConnection.setPackageVersionHeader(new PackageVersion[]{pk});*/
			} catch (Exception e) {
				String msg=e.getMessage()==null?e.toString():e.getMessage();
				throw new RuntimeException("Failed to open Salesforce connection "+getName()+":"+msg);
			}  
		}
		return partnerConnection;
	}

	@Override
	public void close() {
		if(partnerConnection!=null)
			try {
				partnerConnection.logout();
			} catch (ConnectionException e) {
				log.debug(e.getMessage());
			}
	}
	
	protected void checkSSL() throws RuntimeException{
		try {
			if(sslMode.equals(SSLModes.trust))
				SSLUtil.getInstance().addCertToKeyStore(new URL(config.getAuthEndpoint()));
		} catch (Exception e) {
			if (e instanceof UnknownHostException)
				throw new RuntimeException("Host "+config.getAuthEndpoint()+" is unknown.");
			if (e instanceof ConnectException)
				throw new RuntimeException("Could not connect to host "+config.getAuthEndpoint()+" : "+e.getMessage());
			throw new RuntimeException(e);
		}		
	}
	
	
	public void init() throws InitializationException {
		try {
			super.init();
			if(!FileUtil.isURL(getConfigurator().getDatabase())){
				throw new InitializationException(getConfigurator().getDatabase() + " is not a valid url.");
			}
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.trust.toString()));
			
	        config.setUsername(getConfigurator().getUser());
	        config.setPassword(getConfigurator().getPassword()+getConfigurator().getSecurityToken());
	        config.setAuthEndpoint(getConfigurator().getDatabase());
	        config.setConnectionTimeout(30000);
			
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}  
	}
	public StringWriter getTables(String tablePattern) throws RuntimeException {
		log.info("Getting Tables from connection: "+getName());
		StringWriter out = new StringWriter();
		MetadataWriter writer = new MetadataWriter(out);
		String header[] = {"Name"};
		Pattern pattern = null;
		if(tablePattern!=null)
			pattern  = Pattern.compile(tablePattern);
		writer.println(header);
		try {
			PartnerConnection partnerConnection = (PartnerConnection)open();
			DescribeGlobalResult dgr = partnerConnection.describeGlobal();
			for(int i=0;i<dgr.getSobjects().length;i++){
				DescribeGlobalSObjectResult sObjectResult = dgr.getSobjects()[i];
				if (sObjectResult.isQueryable()) {
					String tablename = sObjectResult.getName();
					if(pattern==null || pattern.matcher(tablename).find())
						writer.println(tablename);
				}	
			}
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get tables: "+e.getMessage());
		}finally{
			close();
		}
	}
	
	public StringWriter getTableFields(String tablename) throws RuntimeException {
		log.info("Getting Table field from table "  + tablename + " from connection: "+getName());
		StringWriter out = new StringWriter();
		MetadataWriter writer = new MetadataWriter(out);
		String header[] = {"Name"};
		writer.println(header);
		try {
			PartnerConnection partnerConnection = (PartnerConnection)open();
			DescribeSObjectResult dsor = partnerConnection.describeSObject(tablename);
			for(int i=0;i<dsor.getFields().length;i++)
				writer.println(dsor.getFields()[i].getName());
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get fields from table " + tablename + " : "+e.getMessage());
		}finally{
			close();
		}
	}

	@Override
	public String getMetadata(Properties properties) throws RuntimeException {
		String selector = properties.getProperty("selector");
		String tableName = properties.getProperty("tableName");
		String tablePattern = properties.getProperty("tablePattern");
			
		MetadataSelectors sel;
		try {
			sel = MetadataSelectors.valueOf(selector);
		}
		catch (Exception e) {
			throw new RuntimeException("Property 'selector' must be one of: "+getMetadataSelectorValues());
		}
		switch (sel) {
			case table: return getTables(tablePattern).toString();
			case field: return getTableFields(tableName).toString(); 
		default: return null;
		}
	}

	@Override
	public MetadataCriteria[] getMetadataCriterias() {
		String[] tableFilters = {"tablePattern"};
		String[] fieldFilters = {"tableName"};
		
		ArrayList<MetadataCriteria> criterias = new ArrayList<MetadataCriteria>();		
		for (MetadataSelectors s : MetadataSelectors.values()) {
			MetadataCriteria c = new MetadataCriteria(s.toString());
			switch (s) {
			case table: {c.setFilters(tableFilters); break; }
			case field: { c.setFilters(fieldFilters); break; }
			}
			criterias.add(c);
		}
		return criterias.toArray(new MetadataCriteria[criterias.size()]);
	}

}
