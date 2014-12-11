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
package com.jedox.etl.components.config.extract;

import javax.naming.directory.SearchControls;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;

public class LDAPExtractConfigurator extends TableSourceConfigurator {
	
	public static enum ScopeType {
		BASE,ONE,SUBTREE
	}		
	
	public String[] getClasses() throws ConfigurationException {
		String classes=getXML().getChildTextTrim("classes");
		if (classes==null || classes.isEmpty())
			return new String[0];
		else
			return classes.split(" ");
	}
	
	public String getAttributes() throws ConfigurationException {
		return getXML().getChildTextTrim("attributes");
	}
	
	public int getScope() throws ConfigurationException {
		ScopeType scopeType = ScopeType.SUBTREE;
		String sScope = getXML().getChildTextTrim("scope");
		if (sScope!=null) {
			try {
				scopeType=ScopeType.valueOf(sScope.toUpperCase());
			}
			catch (Exception e){};
		}	
		switch (scopeType) {
		case BASE: return SearchControls.OBJECT_SCOPE;
		case ONE: return SearchControls.ONELEVEL_SCOPE;
		case SUBTREE: return SearchControls.SUBTREE_SCOPE;
		}
		throw new ConfigurationException("Invalid scope type.");
	}
	
	public String getBase() throws ConfigurationException {
		String base = getXML().getChildTextTrim("base");
		if (base == null) {
			base = "";
		}
		return base;
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
 	}
}
