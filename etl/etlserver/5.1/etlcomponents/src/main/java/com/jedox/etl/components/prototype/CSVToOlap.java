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
*/
package com.jedox.etl.components.prototype;

import java.util.List;

import org.jdom.Element;

import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.util.SSLUtil.SSLModes;

public class CSVToOlap extends FileToOlap {

			
	public Element addConnection () {
		Element component = addComponent(ITypes.Connections, "C_File", "File");
		addChild(component,"database",params.getProperty("FILENAME"));			
		addChild(component,"header","true");
		addChild(component,"delimiter",params.getProperty("DELIMITER"));
		String quote = params.getProperty("QUOTE");
		if(!quote.isEmpty())
			addChild(component,"quote",quote);
		String encoding = params.getProperty("ENCODING");
		if(!encoding.isEmpty())
			addChild(component,"encoding",encoding);
		addChild(component,"ssl", SSLModes.trust.name());
		return component;
	}
	
	public Element addExtract (String extractName) {
		Element component = addComponent(ITypes.Extracts, extractName, "File");
		addNameref(component, ITypes.Connections, "C_File");
		return component;
	}	
	
	// do nothing always the same extract
	public String addExtractWithColumnIds(Element extract,List<Integer> columnIds,String newName){
		return "E_Data";
	}

	// do nothing always the same extract
	public void addTargetDateFormat(Element extract, String targetDateFormat) {	
	}
	
}
