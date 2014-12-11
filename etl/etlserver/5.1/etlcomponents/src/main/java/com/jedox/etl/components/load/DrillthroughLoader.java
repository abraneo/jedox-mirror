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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.components.load;

import java.util.ArrayList;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.CubeConfigurator.DrillthroughDescription;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode.UpdateModes;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.PersistenceUtil;

/**
 * @author khaddadin
 *
 */
public class DrillthroughLoader{

	private DrillthroughDescription drillthrough;
	private static final Log log = LogFactory.getLog(DrillthroughLoader.class);
	private String palodb;
	private String cubename;
	private IProcessor rows;
	private Modes writeMode;
	//private HashMap<String, Map<String, IElement[]>> childrenMaps = new HashMap<String, Map<String,IElement[]>>();

	protected void init(DrillthroughDescription drillthrough) throws InitializationException {
		this.drillthrough = drillthrough;
	}

	protected void initInternal(IProcessor rows,String palodb,String cubename,Modes writeMode){
		if(drillthrough!=null){
			this.rows = rows;
			this.palodb = palodb.toUpperCase();
			this.cubename = cubename.toUpperCase();
			this.writeMode = writeMode;
			/*if(this.writeMode.equals(Modes.DELETE) && !splashMode.equals(SplashMode.SPLASH_NOSPLASHING)){
				IDimension[] dims = cube.getDimensions();
				for(IDimension dim:dims){
					this.childrenMaps.put(dim.getName().toUpperCase(), dim.getChildrenMap());
				}
			}*/
		}
	}
	
	public boolean containAnnex(String columnName){
		if(drillthrough!= null){
			return drillthrough.annexColumns.contains(columnName);
		}
		return false;
	}
	
	// check if each annex column is a column in the source, check done none case-sensitive
	private void checkAnnexColumns() throws RuntimeException{
		ArrayList<String> columnsUpper = new ArrayList<String>();
		for (IColumn col : rows.current().getColumns())
			columnsUpper.add(col.getName().toUpperCase());
		for (String annex : drillthrough.annexColumns) {
			if (!columnsUpper.contains(annex))
				throw new RuntimeException("Column "+annex+" is not available in source "+rows.getName()+".");
		}		
	}

	
	public void addDrillthroughPersistor() throws RuntimeException{
		
		if (drillthrough != null) {
			
				Locator loc = new Locator();
				loc.add(palodb);
				loc.add(cubename);
				String schemaName = (drillthrough.schemaName!=null ? drillthrough.schemaName.toUpperCase() : palodb);
				if (drillthrough.connection.isSchemaSupported())
					loc.setPersistentSchema(schemaName);
				else
					loc.setPersistentSchema("");
				
				String tablename = (drillthrough.tableName!=null ? drillthrough.tableName.toUpperCase() : cubename);
				loc.setPersistentTable(tablename);
				PersistorDefinition definition = new PersistorDefinition();
				definition.setConnection(drillthrough.connection);
				if(drillthrough.drillThroughBulksSize!=-1)
					definition.setBulkSize(drillthrough.drillThroughBulksSize);
				definition.setLocator(loc);
				definition.setMode(writeMode);
				definition.setAggregateMode(drillthrough.aggregation);
				//definition.setChildrenMaps(childrenMaps);
				definition.setDirectSource(drillthrough.directSource);
				definition.setLogging(true);

				//define keys
				for (IColumn col : rows.current().getColumns()) {
					col.setName(col.getName().toUpperCase());
					definition.setRole(col.getName(), UpdateModes.key);
					definition.setType(col.getName(), String.class);
				}
				checkAnnexColumns();

				//define value
				IColumn valueColumn = rows.current().getColumn(rows.current().size()-1);
				if (valueColumn.getValueType().equals(Double.class)) {
					definition.setRole(valueColumn.getName(), PersistenceUtil.getDataPersistenceMode(writeMode));
					definition.setType(valueColumn.getName(), Double.class);
				}
				else {
					definition.setRole(valueColumn.getName(), UpdateModes.last);
					definition.setType(valueColumn.getName(), String.class);
				}

				this.rows.addPersistor(definition);

				if (drillthrough.directSource==null)
					log.info("Loading data for "+loc.toString()+" to table "+loc.getPersistentName("")+" for drillthrough");
				else
					log.info("Direct Drillthrough for "+loc.toString()+" is done with link to extract "+drillthrough.directSource.getName());
					
				log.debug("Drillthrough Connection: "+drillthrough.connection.getName()+". Schema: "+schemaName);	
		}

	}


}
