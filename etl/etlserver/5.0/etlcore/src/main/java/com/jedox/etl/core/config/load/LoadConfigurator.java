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
@author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.config.load;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.ViewSource;

/**
 * Basic Configurator class for Loads, writing data to external resources.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class LoadConfigurator extends BasicConfigurator {

	//private static final Log log = LogFactory.getLog(ExportConfigurator.class);
	
	
	public LoadConfigurator() {
	}
	
	/**
	 * determines if this load depends on a source. Default is true
	 * @return true, if this load depends on a source.
	 */
	public boolean hasSource() {
		return true;
	}
	
	/**
	 * gets the Input Source of this Load
	 * @return the Input Source
	 * @throws CreationException
	 */
	public ISource getSource() throws CreationException {
		Element source = getXML().getChild("source");
		ISource viewSource = SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), this, getContext(), source);
		return viewSource;
	}
	
	/**
	 * gets the connection for writing data to the external resource.
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
	
	protected void setDefaultName() throws ConfigurationException {
		if (getName() == null) 
			setName(getXML().getChild("source").getAttributeValue("nameref")); 
	}
	
	private String printModes() {
		StringBuffer output = new StringBuffer();
		for (Modes mode : Modes.values()) {
			output.append(mode.toString() +" ");
		}
		return output.toString();
	}
	
	protected Modes parseMode(String name) throws ConfigurationException {
		Modes mode;
		try { 
			mode = Modes.valueOf(name.toUpperCase());
		}
		catch (Exception e) {
			String message = "Load Mode has to be set to either "+printModes()+"."; 
			throw new ConfigurationException(message);
		}	
		return mode;
	}
	
	/**
	 * gets the Mode this Load operates in.
	 * @return the Load operation mode
	 * @throws ConfigurationException
	 */
	public Modes getMode() throws ConfigurationException {
		return parseMode(getParameter("mode","update"));
	}
		
	public void configure() throws ConfigurationException {
		super.configure();
		setDefaultName();
	}

}
