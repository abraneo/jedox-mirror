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
package com.jedox.etl.components.config.extract;

import java.util.List;
import java.util.ArrayList;
import org.jdom.Element;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.FilterModes;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.source.filter.RegexEvaluator;
import com.jedox.etl.core.util.NamingUtil;

/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CubeExtractConfigurator extends TableSourceConfigurator {

	protected String cubeName;
	protected String valueName = NamingUtil.internal("Value");
	private boolean useRules;
	private boolean ignoreEmptyCells;
	private boolean baseCellsOnly;
	private CellType celltype= CellType.BOTH;
	public String blockDefinition;
	private boolean isDrillthrough;
	
	// equivalent to palojlib.interfaces.ICube.CellsExportType but required for CubeExtractLegacy
	public static enum CellType {
		BOTH,ONLY_NUMERIC,ONLY_STRING
	}	

	public List<DimensionFilterDefinition> getFilterDefinitions() throws ConfigurationException {
		ArrayList<DimensionFilterDefinition> definitions = new ArrayList<DimensionFilterDefinition>();
		Element query = getXML().getChild("query");
		if (query != null) {
			List<Element> dimensions = getChildren(query,"dimension");
			for (Element e : dimensions) {
				DimensionFilterDefinition definition = new DimensionFilterDefinition(getContext(),e,null);
				//if there is no filter in this dimension, a new filter is added that keeps only the base cells
				if(definition.getConditionsExtended().size() == 0){
					definition.addCondition("ACCEPT", new RegexEvaluator("."), FilterModes.onlyNodes,null);
				}
				//else{
					//check if the filters return only base cells, if not set baseCellsOnly to false
				//	for(ConditionExtended ce : definition.getConditionsExtended())
				//		if( !ce.getFilterMode().equals(FilterModes.onlyBases) || !defaultFilterMode.equals(FilterModes.onlyBases))
				//			baseCellsOnly = false;
				//}
				definitions.add(definition);
			}
		}
		return definitions;
	}

	protected void setCube() {
		Element cube = null;
		Element query = getXML().getChild("query");
		if (query != null){
			useRules = Boolean.parseBoolean(query.getAttributeValue("useRules","false"));
			ignoreEmptyCells = Boolean.parseBoolean(query.getAttributeValue("ignoreEmptyCells","true"));
			baseCellsOnly = Boolean.parseBoolean(query.getAttributeValue("onlyBasisAsDefault","true"));
			isDrillthrough = Boolean.parseBoolean(query.getAttributeValue("drillthrough","false"));
			
			if(query.getAttribute("celltype") != null){		
				String celltypevalue = query.getAttribute("celltype").getValue().toUpperCase();
				// renaming of option numeric and string in 3.3, allow backwards compatibility without migration
/*				
				if (celltypevalue.equalsIgnoreCase("NUMERIC"))
					celltype = CellType.ONLY_NUMERIC;
				else if (celltypevalue.equalsIgnoreCase("TEXT"))
					celltype = CellType.ONLY_STRING;
				else
*/				 
					celltype = CellType.valueOf(celltypevalue);					 
			}
			
			blockDefinition = query.getChildTextTrim("blocksize");
			if (blockDefinition!=null && blockDefinition.isEmpty())
			   blockDefinition=null;
			
//			if (blockDefinition==null || blockDefinition.isEmpty())
//				blockDefinition=""+defaultblocksize;
			
			cube = query.getChild("cube");
		}
		cubeName = getName();
		if (cube != null) {
			cubeName = cube.getAttributeValue("name").trim();
			valueName = cube.getAttributeValue("valuename",valueName).trim();
		}
	}

	public String getCubeName() throws ConfigurationException {
		return cubeName;
	}

	public boolean getUseRules() throws ConfigurationException {
		return useRules;
	}

	public boolean getIgnoreEmptyCells() throws ConfigurationException {
		return ignoreEmptyCells;
	}

	public boolean isBaseCellsOnly() throws ConfigurationException {
		return baseCellsOnly;
	}

	public String getValueName() throws ConfigurationException {
		return valueName;
	}

	public CellType getCellType() throws ConfigurationException {
		return celltype;
	}
	
	public String getBlockDefinition() throws ConfigurationException {
		return blockDefinition;
	}
	
	public int getBulkSize() throws ConfigurationException {
		return Integer.parseInt(getParameter("bulkSize",String.valueOf(10000)));
	}
	

	public boolean isDrillthrough() throws ConfigurationException {
		return isDrillthrough;
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		setCube();
	}

}