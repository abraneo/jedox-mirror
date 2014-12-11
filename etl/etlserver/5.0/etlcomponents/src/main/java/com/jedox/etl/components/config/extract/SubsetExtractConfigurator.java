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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.extract;

import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.*;

public class SubsetExtractConfigurator extends TableSourceConfigurator {

/*	private String dimName; // the name of the cube, optional used always with a connection


	*//**
	 * read the dimension name
	 * @throws ConfigurationException
	 *//*
	protected void setDimension() throws ConfigurationException {

		Element dim = getXML().getChild("dimension");

		if(dim != null)
			dimName = dim.getAttributeValue("name");

	}

	*//**
	 * get dimension name
	 * @return dimension name
	 * @throws ConfigurationException
	 *//*
	public String getDimName() throws ConfigurationException {
		return dimName;
	}
	*//**
	 * configure the source by configuring the super class, and setting the cube name
	 *//*
	public void configure() throws ConfigurationException {
		super.configure();
		setDimension();
	}
*/
}
