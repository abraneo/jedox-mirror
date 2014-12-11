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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.List;
//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.extract.CubeSliceExtractConfigurator;
import com.jedox.etl.components.config.extract.CubeSliceExtractConfigurator.NO_FILTER_MODE;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;
import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;

public class CubeSliceExtract extends TableSource implements IExtract {

	private class SliceProcessor extends Processor {
		
		private Row row;
		private int dimcount;
		private IElement[][] slice;
		private IDimension[] outputDimensions;
		private int[] index;
		private int[] randomIndex;
		private boolean noMoreRows = false;
		private boolean onlyOneRow = false;
		private CubeFilter filter;
		
		private java.util.Random rand = new java.util.Random();

		
		/** Generates a Cube Slice with cross product of filtered elements and all root elements of non-filtered dimensions
		 * No Cube-Export is necessary 
		 * This is particular useful for deletion of a Cube in Cube-Load 
		 * @param Cube filter
		 * @throws RuntimeException
		 */
		public SliceProcessor(CubeFilter filter) {
			this.filter = filter;
//			noMoreRows=!filter.isFiltered();
		}

		private String getSliceValue (int dimindex, int valueindex) throws RuntimeException {			
			IElement element = slice[dimindex][valueindex];
			if (element==null)
				// Should never happen
				throw new RuntimeException("Slice value not found in extract "+getName()+". dimindex:"+dimindex+" valueindex: "+valueindex);
			else 
				return element.getName();
		}

/* Generate Random paths without repetition		
		private void setRandomPath() {
			String path;
			do {
				StringBuffer pathBuffer = new StringBuffer(100);
				for (int j=0; j<dimcount; j++) {
					Integer i = rand.nextInt(slice[j].length);
					randomIndex[j] = i;
					pathBuffer.append(i).append(";");				 
				}
				path=pathBuffer.toString();
			}	
			while (pathSet.contains(path));
			pathSet.add(path);
		}
*/
		
		protected boolean fillRow(Row row) throws RuntimeException {
			if (noMoreRows || onlyOneRow) {
				return false;				
			}	
            // generate random paths
			if (isRandomPaths) {
				for (int j=0; j<dimcount; j++) {
					randomIndex[j] = rand.nextInt(slice[j].length);					
				}	
			}
			// index signifies the current element index of each dimension in cross product loop
			for (int j=0; j<dimcount; j++) {
				int indx = ((isRandomPaths) ? randomIndex[j] : index[j]);
				row.getColumn(j).setValue(getSliceValue(j,indx));
			}
			// Value is set to fixed value
			row.getColumn(dimcount).setValue(getConfigurator().getFixValue());
			// If no Dimension Filter return one row which contains only fix value column
			if (dimcount==0) {
				onlyOneRow=true;
				return true;
			}
				
			// increase the index of first dimension
			index[0] += 1;
			// if one index exceeds the number of elements of the dimension, set it to 0 and increase index of next dimension
			for (int j=0; j<dimcount; j++) {
				if (index[j]==slice[j].length) {
					index[j]=0;
					if (j==dimcount-1) {
						// finished if index of last dimension increases the number of elements
						noMoreRows = true;
					}	
					else
						index[j+1] += 1;
				}	
			}	
			
			return true;
		}

		protected Row getRow() {
			return row;
		}

		@Override
		protected void init() throws RuntimeException {
			try {
				IElement[][] basis = filter.getBasisElements();
				// Basis is null if at least one dimension is empty: No Resultset for Cube Extract
				if (basis == null) {
					noMoreRows = true;
					row = CubeSliceExtract.this.getOutputDescription();					
					return;
				}		
				outputDimensions = getOutputDimensions();
				
				List<IElement[]> sliceList = new ArrayList<IElement[]>();
				for (int i=0; i<basis.length; i++) {
					if (basis[i] != null)
						sliceList.add(basis[i]);
					else if (mode.equals(NO_FILTER_MODE.generateRoots))
						// Take all root elements in bases							
						sliceList.add(getOutputDimensions()[i].getRootElements(false));
					else if (mode.equals(NO_FILTER_MODE.generateBases))
						// Take all root elements in bases							
						sliceList.add(getOutputDimensions()[i].getBasesElements(false));
					else if(mode.equals(NO_FILTER_MODE.emptySource)){
						sliceList.add(new IElement[]{});
						this.noMoreRows = true;
					}
				}
				slice = sliceList.toArray(new IElement[sliceList.size()][]);				
				row = PersistenceUtil.getColumnDefinition(getColumns(outputDimensions));
				
				dimcount = outputDimensions.length;			
				index = new int[dimcount];
				if (isRandomPaths)
					randomIndex = new int[dimcount];
			}
				
			catch (Exception e) {
				throw new RuntimeException("Failed to get slice from cube "+getCubeName()+": "+e.getMessage());
			}	
		}
	}
	
		
	private String cubeName;
	private String valueName;
	private NO_FILTER_MODE mode = NO_FILTER_MODE.exclude;
	private boolean isRandomPaths = true;

	private List<DimensionFilterDefinition> definitions;
	//private static final Log log = LogFactory.getLog(CubeSliceExtract.class);

	public CubeSliceExtract() {
		setConfigurator(new CubeSliceExtractConfigurator());
	}

	public CubeSliceExtractConfigurator getConfigurator() {
		return (CubeSliceExtractConfigurator)super.getConfigurator();
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

	private IDimension[] getOutputDimensions() throws RuntimeException {
		if (!mode.equals(NO_FILTER_MODE.exclude))
			return getCubeObject().getDimensions();
		else
			return getCubeFilter().getFilteredDimensions();
			
	}

	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor result;
		result = initProcessor(new SliceProcessor(getCubeFilter()),Facets.OUTPUT);
		result.setLastRow(size);
		return result;
	}
	
	private ICube getCubeObject() throws RuntimeException {
		IDatabase d = getConnection().getDatabase(false,true);
		ICube cube = d.getCubeByName(cubeName);
		if (cube == null) {
			throw new RuntimeException("Cube "+cubeName+" not found in database "+getConnection().getDatabase());
		}
		
		return cube;		
	}
	
	public Row getOutputDescription() throws RuntimeException {
		return PersistenceUtil.getColumnDefinition(getColumns(getOutputDimensions()));

	}

	public void init() throws InitializationException {
		try {
			super.init();
			setCubeName(getConfigurator().getCubeName());
			valueName = getConfigurator().getValueName();
			definitions = getConfigurator().getFilterDefinitions();
			mode = getConfigurator().getNoFilterMode();
			isRandomPaths = getConfigurator().isRandomPaths();
			if(mode.equals(NO_FILTER_MODE.emptySource) && definitions.size()!=0)
				throw new InitializationException("Mode emptySource cannot be used with filters.");
		}
		catch (Exception e) {
			invalidate();
			throw new InitializationException(e);
		}
	}

}
