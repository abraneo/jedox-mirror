package com.jedox.etl.core.olap4j;

import java.math.BigInteger;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import org.olap4j.OlapException;
import org.olap4j.OlapStatement;
import org.olap4j.metadata.Cube;
import org.olap4j.metadata.Dimension;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellExportContext;
import com.jedox.palojlib.interfaces.ICellLoadContext;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IRule;
import com.jedox.palojlib.main.CellExportContext;
import com.jedox.palojlib.main.CellLoadContext;

public class CubeWrapper implements ICube {
	
	private Cube cube;
	private DatabaseWrapper database;
	private Map<String,DimensionWrapper> dimensionLookup = new HashMap<String,DimensionWrapper>();	
	
	public CubeWrapper(DatabaseWrapper database, Cube cube) {
		this.cube = cube;
		this.database = database;
	}

	@Override
	public String getName() {
		return cube.getName();
	}

	@Override
	public int getId() {
		try {
			return cube.getSchema().getCubes().indexOf(cube);
		}
		catch (OlapException e) {
			return 0;
		}
	}

	@Override
	public BigInteger getNumberOfCells() throws PaloException {
		throw new PaloException("Getting number of cells is not supported by OLAP4j provider.");
	}

	@Override
	public BigInteger getNumberOfFilledCells() throws PaloException {
		throw new PaloException("Getting number of filled cells is not supported by OLAP4j provider.");
	}

	@Override
	public IDimension[] getDimensions() throws PaloException {
		int size = cube.getDimensions().size();
		IDimension[] dimensions = new IDimension[size];
		for (int i=0; i<size;i++) {
			//look at local dimension first
			DimensionWrapper wrapper = dimensionLookup.get(cube.getDimensions().get(i).getName());
			if (wrapper == null) {//look at global dimensions
//				wrapper = database.getDimensionByName(cube.getDimensions().get(i).getName());
//			}
//			if (!wrapper.getDimension().equals(cube.getDimensions().get(i))) { 
				//Cube has local dimension with identical name then global dimension.
				wrapper = new DimensionWrapper(cube.getDimensions().get(i),cube.getSchema());
				dimensionLookup.put(wrapper.getName(), wrapper);
			}
			dimensions[i] = wrapper;
		}
		return dimensions;
	}

	@Override
	public ICellsExporter getCellsExporter(IElement[][] area, ICellExportContext context) throws PaloException {
		if (!(context instanceof ExtendedCellExportContext)) throw new PaloException("Export context must be an Olap4j context.");
		return new Olap4jCellsExporter(area,(ExtendedCellExportContext)context,this);
	}

	@Override
	public ICell getCell(IElement[] path) throws PaloException {
		IElement[][] area = new IElement[getCube().getDimensions().size()][1];
		for (int i=0; i<path.length; i++) {
			area[i][0] = path[i];
		}
		ExtendedCellExportContext context = new ExtendedCellExportContext(CellsExportType.BOTH,1000,true,false,false);
		ICellsExporter exporter = new Olap4jCellsExporter(area,context,this);
		if (exporter.hasNext()) return exporter.next();
		return null;
	}

	@Override
	public void clearCells(IElement[][] area) throws PaloException {
		throw new PaloException("Clearing cells not implemented yet.");
	}

	@Override
	public void addRule(String definition, boolean activate,
			String externalIdentifier, String comment) throws PaloException {
		throw new PaloException("Adding rules is not supported by OLAP4j provider.");

	}

	@Override
	public IRule[] getRules() {
		throw new PaloException("Getting rules is not supported by OLAP4j provider.");
	}

	@Override
	public void convert(CubeType type) throws PaloException {
		throw new PaloException("Cube format conversion is not supported by OLAP4j provider.");
	}

	@Override
	public void clear() throws PaloException {
		StringBuilder buffer = new StringBuilder("UPDATE CUBE "+cube.getName()+" SET ROOT() = Null NO_ALLOCATION");
		try {
			OlapStatement statement = getCube().getSchema().getCatalog().getDatabase().getOlapConnection().createStatement();
			statement.executeUpdate(buffer.toString());
		} catch (OlapException e) {
			throw new PaloException("Error updating cube "+getName()+": "+e.getMessage());
		} catch (SQLException e) {
			throw new PaloException("Error updating cube "+getName()+": "+e.getMessage());
		} catch (UnsupportedOperationException e) {
			throw new PaloException("Updateing cube is not supported by OLAP4j provider.");
		}
	}

	@Override
	public CubeType getType() {
		return CubeType.CUBE_NORMAL;
	}

	@Override
	public void loadCells(IElement[][] paths, Object[] values, ICellLoadContext context) throws PaloException {
		throw new PaloException("Loading cells not implemented yet.");

	}

	@Override
	public void removeRules(IRule[] rules) throws PaloException {
		throw new PaloException("Removing rules is not supported by OLAP4j provider.");
	}

	@Override
	public void updateRule(int id, String definition, boolean activate,
			String externalIdentifier, String comment) throws PaloException {
		throw new PaloException("Updateing rules is not supported by OLAP4j provider.");
	}

	@Override
	public void save() throws PaloException {
		throw new PaloException("Saving cube is not supported by OLAP4j provider.");
	}

	@Override
	public void rename(String newname) throws PaloException {
		throw new PaloException("Renaming cube is not supported by OLAP4j provider.");
	}

	@Override
	public void commitLock(int lockId) throws PaloException {
		throw new PaloException("Locking is not supported by OLAP4j provider.");
	}

	@Override
	public long getCBToken() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public long getCCToken() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int lockArea(IElement[][] area) throws PaloException {
		throw new PaloException("Locking is not supported by OLAP4j provider.");
	}

	@Override
	public int lockComplete() throws PaloException {
		throw new PaloException("Locking is not supported by OLAP4j provider.");
	}

	@Override
	public DimensionWrapper getDimensionByName(String name) throws PaloJException {
		DimensionWrapper wrapper = dimensionLookup.get(name);
		if (wrapper != null) {
			return wrapper;
		}
		else {
			//wrapper = database.getDimensionByName(name);
			for (Dimension d : cube.getDimensions()) {
				if ((d.getName().equals(name) /*&& !wrapper.getDimension().getUniqueName().equals(d.getUniqueName())*/)) {
					wrapper = new DimensionWrapper(d,cube.getSchema());
					dimensionLookup.put(wrapper.getName(), wrapper);
				}
			}
			return wrapper;
		}
	}
	
	public Cube getCube() {
		return cube;
	}
	
	public void clearLookup() {
		for (DimensionWrapper w : dimensionLookup.values()) {
			w.getElementCache().clear();
		}
		dimensionLookup.clear();
	}

	@Override
	public void removeRules() throws PaloException {
		throw new PaloException("RemoveRules is not supported by OLAP4j provider.");
	}

	@Override
	public void activateRules(IRule[] arg0) throws PaloException {
		throw new PaloException("activateRules is not supported by OLAP4j provider.");		
	}

	@Override
	public void deactivateRules(IRule[] arg0) throws PaloException {
		throw new PaloException("deactivateRules is not supported by OLAP4j provider.");		
	}

}
