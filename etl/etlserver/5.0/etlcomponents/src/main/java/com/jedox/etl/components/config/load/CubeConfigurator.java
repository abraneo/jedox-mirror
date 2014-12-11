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

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.load.LoadConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.extract.IRelationalExtract;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.persistence.PersistorDefinition.AggregateModes;
import com.jedox.etl.core.source.ISource;


/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CubeConfigurator extends LoadConfigurator {

	public static enum SplashModes {
		DEFAULT, ADD, SET, DISABLED
	}

	public static enum Aggregations {
		NONE, SUM
	}

	/* default type decide how to react to cells that dont belong to the cube in its currect status
	 *  warning (default): only give a warning if this happens
	 *  mapDefault (before defaultBase): the missing dimension element is replaced by the "default" element
	 *  createElement (before defaultConsolidate): the missing dimension element is created and consolidated under default element
	 *  in defaultBase and defaultConsolidate: default element should already exist in the dimension, otherwise a warning
	 *
	 * */
	public static enum DefaultType {
		WARNING, MAPTODEFAULT, CREATEUNDERDEFAULT
	}

	public class DrillthroughDescription {
		public IRelationalConnection connection;
		public String aggregation = "bulk";
		public String schemaName;
		public String tableName;
		public ArrayList<String> annexColumns = new ArrayList<String>();
		public ISource directSource;
		public int drillThroughExternalBulksSize=-1;
	}


	private DrillthroughDescription drillthrough;
	private DefaultType defaultType = DefaultType.WARNING;
	private String defaultValue;
	private Map<String,String> dimensions;

	public void configure() throws ConfigurationException {
		super.configure();
		setDefaultMode();
		setDrillthroughDescription();
		setDimensions();
	}

	private void setDefaultMode() throws ConfigurationException{
		Element defaultElement = getXML().getChild("default");
		if(defaultElement != null){
			defaultValue = defaultElement.getTextTrim();
			String defType=defaultElement.getAttributeValue("type");
			// for backwards compatibility to 3.3
/*			
			if (defType.equals("defaultBase"))
				defType="mapToDefault";
			if (defType.equals("defaultConsolidate"))
				defType="createUnderDefault";
*/						
			defaultType = DefaultType.valueOf(defType.toUpperCase());
			if(defaultValue.equals("") && ( defaultType.equals(DefaultType.MAPTODEFAULT) || defaultType.equals(DefaultType.CREATEUNDERDEFAULT)))
				throw new ConfigurationException("Default element has to be specified for default type "+defaultType.toString()+".");
		}
	}

	public String getDefaultValue(){
		return defaultValue;
	}

	public DefaultType getDefaultType(){
		return defaultType;
	}

	public String getCubeName() throws ConfigurationException {
		String name = getName();
		Element cube = getXML().getChild("cube");
		if (cube != null) {
			name = cube.getAttributeValue("name",name).trim();
		}
		return name;
	}

	private void setDrillthroughDescription() throws ConfigurationException {
		Element cube = getXML().getChild("cube");
		if (cube != null) {
			Element drill = cube.getChild("drillthrough");
			if (drill != null) {
				drillthrough = new DrillthroughDescription();
				String connectionID = drill.getAttributeValue("connection");
				if (connectionID == null) //assume internal connection
					drillthrough.connection = ConfigManager.getInstance().getDrillthroughConnection();
				else {//get external connection
					IConnection connection = (IConnection)ConfigManager.getInstance().getComponent(getLocator().getRootLocator().add(ITypes.Connections).add(connectionID.trim()), getContextName());
					drillthrough.drillThroughExternalBulksSize = getDrillThroughExternalBulksSize();
					if (connection instanceof IRelationalConnection)
						drillthrough.connection = (IRelationalConnection)connection;
					else
						throw new ConfigurationException("Connection '"+connectionID+"' does not refer to a relational Connection.");
				}
				// Direct Drillthrough with a Relational Extract defining the SQL-Query for the Drillthrough request
				String directextract = drill.getAttributeValue("directextract");
				if (directextract!=null && !directextract.isEmpty()) {
					drillthrough.directSource = (ISource)ConfigManager.getInstance().getComponent(getLocator().getRootLocator().add(ITypes.Extracts).add(directextract.trim()), getContextName());
					if (!(drillthrough.directSource instanceof IRelationalExtract))
						throw new ConfigurationException("Source "+drillthrough.directSource.getName()+" has to be a Relational Extract in order to use direct drillthrough");
					// For direct drillthrough Aliases are allowed but no Mappings to default values
					if (drillthrough.directSource.getAliasMap().hasDefaultValues())
						throw new ConfigurationException("Source "+drillthrough.directSource.getName()+" must not contain an alias map with default values in order to be used for direct drillthrough");
				}
				
				drillthrough.aggregation = drill.getAttributeValue("aggregate",AggregateModes.bulk.toString());
				if (drillthrough.aggregation.equalsIgnoreCase("false"))
						drillthrough.aggregation = AggregateModes.none.toString();
				if (drillthrough.aggregation.equalsIgnoreCase("true"))
					drillthrough.aggregation = AggregateModes.bulk.toString();

				@SuppressWarnings("unchecked")			
				List<Element> annexes = drill.getChildren();
				if (!annexes.isEmpty())
					throw new ConfigurationException("Obsolete annex definition used."); 
				/*					
				for (int i=0; i<annexes.size(); i++) {
					Element e = (Element) annexes.get(i);
					drillthrough.annexColumns.add(e.getAttributeValue("input").toUpperCase());
				}
*/				
	
				drillthrough.tableName = drill.getAttributeValue("tablename");
				drillthrough.schemaName = drill.getAttributeValue("schemaname");
				if((drillthrough.tableName!=null || drillthrough.schemaName!=null) && drillthrough.directSource!=null){
					throw new ConfigurationException("Direct drillthrough can not be used with parameters table name and schema name.");
				}
								
				if(drillthrough.schemaName!=null && !drillthrough.connection.isSchemaSupported()){
					throw new ConfigurationException("Schemas can not be defined for connection " + drillthrough.connection.getName() + ", the corresponding database type does not allow both database and schema levels.");
				}
			}
		}
	}
	
	private void setDimensions() throws ConfigurationException {
		dimensions = new LinkedHashMap<String,String>();
		Element cube = getXML().getChild("cube");
		if (cube != null) {
			Element dimensionsElement = cube.getChild("dimensions");
			if (dimensionsElement != null) {
				List<?> dims = dimensionsElement.getChildren();
				for (int i=0; i<dims.size(); i++) {
					Element dimensionElement = (Element) dims.get(i);
					String inputValue = dimensionElement.getAttributeValue("input");
					String name = dimensionElement.getAttributeValue("name");
					// If no name is given the input column is skipped
					if (name!=null) {
						dimensions.put(inputValue, dimensionElement.getAttributeValue("name", inputValue));
						if(drillthrough!=null && drillthrough.directSource!=null && !dimensions.get(inputValue).equalsIgnoreCase(inputValue)){
							throw new ConfigurationException("The input " + inputValue + " should have the same name as the dimension if direct drillthrough is used.");
						}	
					} else if (drillthrough!=null) {
						drillthrough.annexColumns.add(inputValue.toUpperCase());
					}
				}
			}
		}
	}
	
	public Map<String,String> getDimensions() {
		return dimensions;
	}

	public DrillthroughDescription getDrillthroughDescription() throws ConfigurationException {
		return drillthrough;
	}

	public SplashModes getSplashMode() throws ConfigurationException {
		Element cube = getXML().getChild("cube");
		try {
			if (cube != null) {
				return SplashModes.valueOf(cube.getAttributeValue("splash","disabled").toUpperCase());
			}
		}
		catch (IllegalArgumentException e) {
			throw new ConfigurationException("Illegal Splash Mode");
		}
		return SplashModes.DEFAULT;
	}

	public int getBulksSize() throws ConfigurationException {
		return Integer.parseInt(getParameter("bulkSize",String.valueOf(10000)));
	}

	public Boolean getDeactivateRules() throws ConfigurationException {
		Element cube = getXML().getChild("cube");		
		return Boolean.valueOf((cube.getAttributeValue("deactivateRules","false")));
	}

	public Boolean useEventProcessor() throws ConfigurationException {
		Element cube = getXML().getChild("cube");		
		boolean deactivateSVS = Boolean.valueOf((cube.getAttributeValue("deactivateSVS","false")));
		return !deactivateSVS;
	}
	
	public int getDrillThroughExternalBulksSize() throws ConfigurationException {
		return Integer.parseInt(getParameter("drillThroughExternalBulkSize",String.valueOf(10000)));
	}
		
	public Boolean getCompleteLocking() throws ConfigurationException {
		Element cube = getXML().getChild("cube");		
		boolean locking = Boolean.valueOf((cube.getAttributeValue("completelock","false")));
		
		if(locking == true && defaultType.equals(DefaultType.CREATEUNDERDEFAULT)){
			throw new ConfigurationException("Locking can not be used with default type "+DefaultType.CREATEUNDERDEFAULT+".");
		}
		
		if(locking == true && !getSplashMode().equals(SplashModes.DISABLED)){
			throw new ConfigurationException("Locking can only be done when writing bases cells, therefore splashing should be disabled.");
		}
		
		if(locking == true && !getMode().equals(Modes.INSERT)){
			throw new ConfigurationException("Locking can only be done with mode \"insert\", since no aggregating is possible when the cube is locked..");
		}
		
		return locking;
	}


}
