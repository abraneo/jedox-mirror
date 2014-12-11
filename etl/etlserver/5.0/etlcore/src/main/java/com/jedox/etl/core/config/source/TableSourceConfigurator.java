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
package com.jedox.etl.core.config.source;

import org.jdom.Element;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.aliases.AliasMapFactory;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IFileConnection;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.util.NamingUtil;
/**
 * Basic Configurator Class for table based sources
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TableSourceConfigurator extends BasicConfigurator {
	
	//private static final Log log = LogFactory.getLog(TableSourceConfigurator.class);
	private String query;
	private IAliasMap map;
	
	/**
	 * gets the Connection of this source for reading data
	 * @return the Connection
	 * @throws ConfigurationException
	 */
	public IConnection getConnection() throws ConfigurationException {
		Element connection = getXML().getChild("connection");
		if (connection != null) {
			String connectionID = connection.getAttributeValue("nameref"); 
			return (IConnection)ConfigManager.getInstance().getComponent(getLocator().getRootLocator().add(ITypes.Connections).add(connectionID), getContextName());
		}
		return null;
	}
	
	protected void setAliasMap(IAliasMap map) {
		this.map = map;
	}
	
	protected void setAliasMap() throws ConfigurationException {
		map = new AliasMap();
		Element aliasMap = getXML().getChild("alias_map");
		if (aliasMap != null) {
			try {
				map = AliasMapFactory.getInstance().newAliasMap(AliasMap.class.getCanonicalName(),this,getContext(),aliasMap);
			}
			catch (CreationException e) {
				throw new ConfigurationException(e);
			}
		}
	}
	
	/**
	 * gets the specified AliasMap for this source, mapping columns to names for use in project XMLs.
	 * @return the AliasMap
	 */
	public IAliasMap getAliasMap() {
		return map;
	}
	
	protected void setQuery(String query) {
		if (query != null) this.query = query;
	}
	
	protected void setQuery() {
		query = getXML().getChildTextTrim("query");
	}
	
	/**
	 * Determines if this Configurator provides a query
	 * @return true, if so.
	 */
	public boolean hasQuery() {
		return getXML().getChild("query") != null;
	}
	
	/**
	 * gets the query for querying data via the given Connection 
	 * @return the Query as String
	 * @throws ConfigurationException
	 */
	public String getQuery() throws ConfigurationException {
		String result = query;
		//special file source default
		IConnection connection = getConnection();
		if ((connection != null) && (connection instanceof IFileConnection)) {
			if (result != null) {
				// remove all "tablename." before columns because they are not needed and will make the an error later
				result = result.replace(escapeName(connection.getLocator().getName())+".", ""); 
				result = result.replace(escapeName(connection.getLocator().getName()), NamingUtil.internalDatastoreName());
				}
		}
		return result;
	}
	
	private String printViews() {
		StringBuffer output = new StringBuffer();
		for (Views view : Views.values()) {
			output.append(view.toString() +" ");
		}
		return output.toString();
	}
	
//	private String printFetchModes() {
//		StringBuffer output = new StringBuffer();
//		for (FetchModes mode : FetchModes.values()) {
//			output.append(mode.toString() +" ");
//		}
//		return output.toString();
//	}
	
	protected Views getFormat(String format, String defaultFormat) throws ConfigurationException {
		String f = (format == null) ? defaultFormat : format;
		try { 
			return Views.valueOf(f.toUpperCase());
		}
		catch (Exception e) {
			throw new ConfigurationException("Attribute 'format' has to be set to either "+printViews()+".");
		}	
	}
	
	/**
	 * gets the format of the source, when multiple formats are available 
	 * @return the format
	 * @throws ConfigurationException
	 */
	public Views getFormat() throws ConfigurationException {
		return getFormat(getXML().getAttributeValue("format"),Views.NONE.toString());
	}
	
	/**
	 * gets the fetch mode of this source.
	 * @return the fetch mode
	 * @throws ConfigurationException
	 */
//	public FetchModes getFetchMode() throws ConfigurationException {
//		String mode = getParameter("fetch","buffered");
//		try { 
//			return FetchModes.valueOf(mode.toUpperCase());
//		}
//		catch (Exception e) {
//			throw new ConfigurationException("Attribute 'fetch' has to be set to either "+printFetchModes()+".");
//		}	
//	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		setAliasMap();
		setQuery();
 	}

	

}
