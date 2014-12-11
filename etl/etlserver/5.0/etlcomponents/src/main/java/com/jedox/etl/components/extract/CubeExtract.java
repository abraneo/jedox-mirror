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
package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.extract.CubeExtractConfigurator;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.components.connection.OLAPConnection;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.olap4j.ExtendedCellExportContext;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.DatastoreManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;
import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;

/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CubeExtract extends TableSource implements IExtract {

	private class CubeProcessor extends Processor {

		private CubeFilter filter;
		//private ExportDataset export;
		private Row row;
		private int count;
		//private ExportContext context;
		private ICellsExporter exporter;

		public CubeProcessor(CubeFilter filter) throws RuntimeException {
			try {
				this.filter = filter;
				setName(CubeExtract.this.getName());
				setInfo(true,"extract");
				getParameter();
				init();
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to get cells from cube "+getCubeName()+": "+e.getMessage());
			}
		}

		private void init() throws RuntimeException {
			count = 0;
			IElement[][] basis = filter.getBasisElements();
			// Basis is null if at least one dimension is empty: No Resultset for Cube Extract
			if (basis == null) {
				row = CubeExtract.this.getOutputDescription();
				return;
			}
			
			log.debug("Parameters for cube extract " + getName() + ": bulkSize=" + blockSize+";" +
					  " isBaseCellsOnly=" + baseCellsOnly+"; ignoreEmptyCells=" + ignoreEmptyCells + ":useRules=" + useRules);
			
			ExtendedCellExportContext context = new ExtendedCellExportContext(cellsExportType, blockSize, useRules, baseCellsOnly, ignoreEmptyCells);
			if (slicerDimensions!=null)
				context.setSlicerDimensions(Arrays.asList(slicerDimensions));
					
			exporter = filter.getCube().getCellsExporter(basis,context);
			row = PersistenceUtil.getColumnDefinition(getAliasMap(),getColumns(exporter.getDimensions()));
		}

		protected boolean fillRow(Row row) throws Exception {
			if (exporter!= null && exporter.hasNext()) {

				try {
					ICell cell = exporter.next();
					if (cell != null) {
						count++;
						String[] path = cell.getPathNames();
						if (path != null) {
							for (int j=0; j<path.length; j++) {
								row.getColumn(j).setValue(path[j]);
							}
							//value
							row.getColumn(row.size()-1).setValue(getStringValue(cell.getValue()));
							//row.getColumn(row.size()-1).setValue(getStringValue(context.getProgress()));
						}
					}else{
						// this sometimes returns from olap (ticket: 12027)
						return false;
					}
				}
				catch (Exception e) {
					log.error("Error in cell export of cube "+getName()+" for row "+count+": "+e.getMessage());
				}
				return true;
			}
			else { //finished ... do some cleanup
				return false;
			}
		}

		protected Row getRow() {
			return row;
		}

	}

	private String cubeName;
	private String valueName;
	private boolean useRules;
	private boolean ignoreEmptyCells;
	private boolean baseCellsOnly;
	private CellsExportType cellsExportType;
	private boolean drillthrough;
	
	private int blockSize;
	private String[] slicerDimensions;	

	private List<DimensionFilterDefinition> definitions;
	private static final Log log = LogFactory.getLog(CubeExtract.class);

	public CubeExtract() {
		setConfigurator(new CubeExtractConfigurator());
		notifyRetrieval = true;
	}

	public CubeExtractConfigurator getConfigurator() {
		return (CubeExtractConfigurator)super.getConfigurator();
	}

	private String[] getColumns(IDimension[] dimensions) {
		String [] names = new String[dimensions.length+1];
		for (int i=0; i<dimensions.length; i++) {
			names[i] = dimensions[i].getName();
			//make a standard alias to the name of the column.
			getAliasMap().map(new AliasMapElement(names[i],i+1));
		}
		names[dimensions.length] = valueName;
		getAliasMap().map(new AliasMapElement(names[dimensions.length], dimensions.length+1));
		return names;
	}

	private void setCubeName(String cubeName) {
		this.cubeName = cubeName;
	}

	private String getCubeName() {
		return cubeName;
	}

	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		if (connection != null)
			throw new RuntimeException("OLAP connection is needed for extract "+getName()+".");
		return null;
	}

	private CubeFilter getCubeFilter() throws RuntimeException {
		
		CubeFilter filter = new CubeFilter(getCubeObject());
		filter.configure(definitions);
		return filter;
	}

	private String getStringValue(Object value) {
		if (value == null) return null;
		return value.toString();
	}

	private IProcessor getDrillthroughProcessor(int size) throws RuntimeException {
		IDimension[] dims = getCubeObject().getDimensions();
		String[] names = new String[dims.length];
		for (int i=0; i<dims.length; i++)
			names[i]=dims[i].getName();

		IElement[][] basis = getCubeFilter().getBasisElements();
		
		int[] lengths = new int[dims.length];
		List<String> values = new ArrayList<String>();
		for (int i=0;i<dims.length; i++) {
			if (basis==null) {
				lengths[i]=1;
				values.add("");
			}
			else if (basis[i]==null)
				lengths[i]=0;
			else {
				lengths[i]=basis[i].length;
				for (IElement el : basis[i]) {
					values.add(el.getName());
				}
			}	
		}			
		String datastore=getConnection().getDatabase().toUpperCase()+"."+getCubeName().toUpperCase();
		Datastore ds = DatastoreManager.getInstance().get(datastore);
		if (ds==null)
			throw new RuntimeException("No Drillthrough defined for cube "+getCubeName()+" in Connection "+getConnection().getName());

		return ds.getFilteredProcessor(names, values.toArray(new String[values.size()]), lengths, size);
	}
	
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor result;
		if (drillthrough) {
			return getDrillthroughProcessor(size);
		}
		else {
			result = new CubeProcessor(getCubeFilter());
			result.setLastRow(size);
		}	
		return result;
	}
	
	private ICube getCubeObject() throws RuntimeException {
		com.jedox.palojlib.interfaces.IConnection con = (com.jedox.palojlib.interfaces.IConnection) getConnection().open();
		String database = getConnection().getDatabase();
		IDatabase d = con.getDatabaseByName(database);
		if (d == null) {
			throw new RuntimeException("Database "+database+" not found in connection "+getConnection().getName());
		}
		ICube cube = d.getCubeByName(cubeName);
		if (cube == null) {
			throw new RuntimeException("Cube "+cubeName+" not found in database "+database);
		}
		
		return cube;		
	}
	
	public Row getOutputDescription() throws RuntimeException {
		if (drillthrough)
			return getDrillthroughProcessor(1).current();
		else
			return PersistenceUtil.getColumnDefinition(getColumns(getCubeObject().getDimensions()));
	}
	
	
	public void initBlockSize() throws Exception {
		blockSize = getConfigurator().getBulkSize();		
		String blockDefinition=getConfigurator().getBlockDefinition();

		if ((getConnection() instanceof OLAPConnection)) {
			if (blockDefinition!=null)
				throw new InitializationException("Blocksize cannot be changed for this connection type.");
		}
		else {
			if (blockDefinition==null)
				return;
			else if (blockDefinition.startsWith("#"))			
				slicerDimensions = blockDefinition.substring(1).split(",");
			else {			
				try {
					blockSize = Integer.parseInt(blockDefinition);
				}	
				catch (Exception e) {
					throw new InitializationException("Blocksize must be a numerical value or start with # for indication of slicer dimensions.");
				}
			}			
		}
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			setCubeName(getConfigurator().getCubeName());
			valueName = getConfigurator().getValueName();
			useRules = getConfigurator().getUseRules();
			definitions = getConfigurator().getFilterDefinitions();
			ignoreEmptyCells = getConfigurator().getIgnoreEmptyCells();
			baseCellsOnly =  getConfigurator().isBaseCellsOnly();
			cellsExportType = CellsExportType.valueOf(getConfigurator().getCellType().toString());
			drillthrough=getConfigurator().isDrillthrough();
			initBlockSize();
		}
		catch (Exception e) {
			invalidate();
			throw new InitializationException(e);
		}
	}

}
