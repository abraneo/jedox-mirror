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

import java.util.Collections;
import java.util.HashMap;
import java.util.List;

import org.jdom.Element;

import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.util.SSLUtil.SSLModes;

public class ExcelToOlap extends FileToOlap {

			
	public Element addConnection () {
		Element component = addComponent(ITypes.Connections, "C_Excel", "ExcelFile");
		addChild(component,"database",params.getProperty("FILENAME"));
		addChild(component,"ssl", SSLModes.trust.name());
		return component;
	}
	
	public Element addExtract (String extractName) {
		Element component = addComponent(ITypes.Extracts, extractName, "Excel");
		addNameref(component, ITypes.Connections, "C_Excel");
		addChild(component,"header", "true");			
		addChild(component,"query", params.getProperty("RANGE"));
		return component;
	}	
	
	public String addExtractWithColumnIds(Element extract,List<Integer> columnIds,String newName){
		Element e = extract; 
		if(!getComponentName(extract).equals(newName)){
			e = cloneComponent(extract, newName);
		}
		
		HashMap<Integer,String> subMap = new HashMap<Integer,String>();
		if(columnIds.size()>0){
			Collections.sort(columnIds);
			String columnsIdsStr = "";
			for(int j=0;j<columnIds.size();j++){
				int i = columnIds.get(j);
				columnsIdsStr+=i+",";
				if(model.getAliasDefaults().get(i)!=null)
					subMap.put(j+1, model.getAliasDefaults().get(i));
			}
			columnsIdsStr = columnsIdsStr.substring(0, columnsIdsStr.length()-1);
			// add the rangeColumnSubset parameter
			addChild(e,"rangeColumnSubset",columnsIdsStr);
			// update the alias map
			addAliasMapToExtract(e, subMap);
		}
		return newName;
	}

	@Override
	public void addTargetDateFormat(Element extract, String targetDateFormat) {
		if(targetDateFormat!=null)
			addChild(extract,"dateTargetFormat", targetDateFormat);
		
	}
	
}
