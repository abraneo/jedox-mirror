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


import java.util.List;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.filter.RowFilter;

/**
 * Configurator Class for particular Views on Sources, which enables the (largely) unified handling of table based and tree based sources 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ViewSourceConfigurator extends TreeSourceConfigurator {
	
	//private static final Log log = LogFactory.getLog(ViewSourceConfigurator.class);
	
	protected void setName() throws ConfigurationException {
		setName(getXML().getAttributeValue("nameref"));
	}
	
	/**
	 * gets the underlying source to be viewed.
	 * @return the underlying source
	 * @throws ConfigurationException
	 */
	public ISource getSource() throws ConfigurationException {
		ISource datasource = null;
		String name = getXML().getAttributeValue("nameref");
		datasource = (ISource) ConfigManager.getInstance().getComponent(getLocator().getRootLocator().add(ITypes.Sources).add(name), getContextName());
		return datasource;
	}
	
	protected String getSorter() throws ConfigurationException {
		Element sorter = getXML().getChild("sorter");
		StringBuffer orderString = new StringBuffer();
		if (sorter != null) {
			orderString.append(" order by");
			List<?> inputs = sorter.getChildren("input");
			for (int i=0; i<inputs.size(); i++) {
				Element input = (Element) inputs.get(i);
				if (input != null)  {
					String order = input.getAttributeValue("order","asc");
					String alias = input.getAttributeValue("nameref");
					orderString.append(" "+escapeName(alias)+" "+order);
					if (i<inputs.size()-1) orderString.append(",");
				}
			}
		}
		return orderString.toString();
	}
	
	/**
	 * gets the filter to be applied on the underlying source
	 * @return the filter object
	 * @throws ConfigurationException
	 */
	public RowFilter getFilter() throws ConfigurationException {
		FilterConfigurator util = new FilterConfigurator(getContext(), getParameter());
		return util.getFilter(getXML().getChild("filter"));
	}
	
	protected void setSortQuery() throws ConfigurationException {
		String query = getQuery();
		String sorter = getSorter();
		Locator loc = getLocator().clone().add(getName());
		if (!sorter.equals("")) {
			if (query == null) {
				setQuery("select * from "+loc.getPersistentName()+sorter);
			} else {
				int pos = query.lastIndexOf("order by");
				if (pos == -1) 
					setQuery(query+sorter);
				else 
					setQuery(query.substring(0,pos)+sorter);
			}
		}
	}
	
	/**
	 * gets the format an underlying tree source should be rendered in
	 */
	public Views getFormat() throws ConfigurationException {
		return getFormat(getXML().getAttributeValue("format"),Views.NONE.toString());
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		setSortQuery();
	}

}
