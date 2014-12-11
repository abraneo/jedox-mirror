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
package com.jedox.etl.components.config.connection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jasypt.util.text.BasicTextEncryptor;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.connection.ConnectionConfigurator;

/**
 * 
 * @author kais.haddadin@jedox.com
 *
 */
public class SalesforceConnectionConfigurator extends ConnectionConfigurator{

	private static final Log log = LogFactory.getLog(SalesforceConnectionConfigurator.class);
	
	public String getSecurityToken() throws ConfigurationException {
		
		Element passwordElement = getXML().getChild("securitytoken");
		String password = "";
		if (passwordElement != null) {
			password = passwordElement.getTextTrim();
			if (passwordElement.getAttributeValue("encrypted","false").equalsIgnoreCase("true")) {
				try {
					BasicTextEncryptor crypt = new BasicTextEncryptor();
					crypt.setPassword(Settings.getInstance().getContext(Settings.EncryptionCtx).getProperty("password"));
					password = crypt.decrypt(password);
				}
				catch (Exception e) {
					log.debug("Decryption of password for connection "+getName()+" failed.");
//					password = " ";
				}
			}
		}
		return password;
		
	}
	
}