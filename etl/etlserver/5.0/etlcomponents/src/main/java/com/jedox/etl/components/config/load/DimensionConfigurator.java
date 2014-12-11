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
package com.jedox.etl.components.config.load;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.load.LoadConfigurator;
import com.jedox.etl.core.load.ILoad.Modes;
// import com.jedox.palojlib.interfaces.IElement.ElementType;

/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class DimensionConfigurator extends LoadConfigurator {

	//private static final Log log = LogFactory.getLog(DimensionConfigurator.class);

	public String getDimensionName() throws ConfigurationException {
		String name = getName();
		Element dimension = getXML().getChild("dimension");
		if (dimension != null) {
			name = dimension.getAttributeValue("name",name).trim();
		}
		return name;
	}

	private Modes getImplicitMode() throws ConfigurationException {
		Element dimension = getXML().getChild("dimension");
		if (dimension != null) {
			return (dimension.getChildren().size() > 0) ? null : getMode();
		}
		else
			return getMode();
	}

	public Modes getElementsMode() throws ConfigurationException  {
		Modes result = getImplicitMode();
		Element dimension = getXML().getChild("dimension");
		if (dimension != null) {
			Element elements = dimension.getChild("elements");
			if (elements != null) {
				String mode = elements.getAttributeValue("mode",getMode().toString());
				result = parseMode(mode);
			}
		}
		return result;
	}

	public Modes getHierarchyMode() throws ConfigurationException  {
		Modes result = getImplicitMode();
		Element dimension = getXML().getChild("dimension");
		if (dimension != null) {
			Element elements = dimension.getChild("consolidations");
			if (elements != null) {
				String mode = elements.getAttributeValue("mode",getMode().toString());
				result = parseMode(mode);
			}
		}
		return result;
	}

	public Modes getAttributesMode() throws ConfigurationException  {
		Modes result = getImplicitMode();
		Element dimension = getXML().getChild("dimension");
		if (dimension != null) {
			Element elements = dimension.getChild("attributes");
			if (elements != null) {
				String mode = elements.getAttributeValue("mode", getMode().toString());
				result = parseMode(mode);
			}
		}
		return result;
	}
/*
	public ElementType getElementsType() throws ConfigurationException {
		String t = null;
		Element dimension = getXML().getChild("dimension");
		if (dimension != null) {
			Element elements = dimension.getChild("elements");
			if (elements != null)
				t = elements.getAttributeValue("type");
		}
		if (t != null) {
			try {
				return ElementType.valueOf(t.toUpperCase());
			}
			catch (Exception e) {
				String message = "Load "+getName()+": Parameter elementtype has to be either TEXT or NUMERIC.";
				throw new ConfigurationException(message);
			}
		}
		return null;
	}
*/
	public int getAttributeBulksSize() throws ConfigurationException {
		return Integer.parseInt(getParameter("attributeBulkSize",String.valueOf(10000)));
	}

	public int getConsolidateBulksSize() throws ConfigurationException {
		return Integer.parseInt(getParameter("consolidateBulkSize",String.valueOf(10000)));
	}


	public void configure() throws ConfigurationException {
		super.configure();
	}
}
