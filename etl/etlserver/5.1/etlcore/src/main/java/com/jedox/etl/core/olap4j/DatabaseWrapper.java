package com.jedox.etl.core.olap4j;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.main.DatabaseInfo;

import org.olap4j.OlapException;
import org.olap4j.metadata.*;

public class DatabaseWrapper implements IDatabase {
	
	private Catalog catalog;
	private Map<String,DimensionWrapper> dimensionLookup = new HashMap<String,DimensionWrapper>();	
	private Map<String,CubeWrapper> cubeLookup = new HashMap<String,CubeWrapper>();
	
	public DatabaseWrapper(Catalog catalog) {
		this.catalog = catalog;
		try {
			/*
			ResultSet rs = catalog.getMetaData().getDimensions("Demo", null, null, null);
			CSVWriter writer = new CSVWriter(System.out);
			writer.write(rs);
			for (Dimension d : getSchema().getSharedDimensions()) {
				System.out.println(d.getName());
			}
			for (Cube c : getSchema().getCubes()) {
				System.out.println(c.getName());
			}
			for (Cube c : getSchema().getCubes()) {
				for (Dimension d : c.getDimensions()) {
					System.out.println(d.getName());
					for (Hierarchy h: d.getHierarchies()) {
						System.out.println(h.getName());
					}
				}
			}
			*/
		}
		catch (Exception e) {
			System.err.println("Error getting catalog metadata: "+e.getMessage());
		}
	}

	public Schema getSchema() throws OlapException {
		List<Schema> schemas = catalog.getSchemas();
		if (schemas.isEmpty()) {
			throw new PaloJException("No schema defined for catalog "+catalog.getName());
		}	
		return catalog.getSchemas().get(0);
	}
	
	@Override
	public int getId() {
		try {
			return catalog.getDatabase().getCatalogs().indexOf(catalog);
		}
		catch (OlapException e) {
			return 0;
		}
	}

	@Override
	public String getName() {
		return catalog.getName();
	}

	@Override
	public DatabaseType getType() {
		return DatabaseType.DATABASE_NORMAL;
	}
	
	protected List<Dimension> getSharedDimensions() throws OlapException {
		ArrayList<Dimension> result = new ArrayList<Dimension>();
		// List<Dimension> result = getSchema().getSharedDimensions();      
		// if (result.isEmpty()) {   
		// Workaround for broken / not implemented Method, 
		// with olap4j 1.1.0 dimensions used in several cubes are returned for each cubes they are used in
			Set<Dimension> set = new HashSet<Dimension>();
			for (Cube c : getSchema().getCubes()) {
				for (Dimension d: c.getDimensions()) {
					set.add(d);
				}
			}
			result.addAll(set);
		//}
		return result;
	}

	@Override
	public IDimension[] getDimensions() throws PaloException {
		try {
			List<Dimension> dims = getSharedDimensions();
			int size = dims.size();
			IDimension[] dimensions = new IDimension[size];
			for (int i=0; i<size;i++) {
				dimensions[i] = dimensionLookup.get(dims.get(i).getName());
				if (dimensions[i] == null) {
					DimensionWrapper wrapper = new DimensionWrapper(dims.get(i),getSchema());
					dimensions[i] = wrapper;
					dimensionLookup.put(wrapper.getName(), wrapper);
				}
			}
			return dimensions;
		}
		catch (OlapException e) {
			throw new PaloJException(e.getMessage());
		}
	}

	@Override
	public ICube[] getCubes() throws PaloException {
		try {
			int size = getSchema().getCubes().size();
			ICube[] cubes = new ICube[size];
			for (int i=0; i<size;i++) {
				cubes[i] = cubeLookup.get(getSchema().getCubes().get(i).getName());
				if (cubes[i] == null) {
					CubeWrapper wrapper =  new CubeWrapper(this,getSchema().getCubes().get(i));
					cubes[i] = wrapper;
					cubeLookup.put(wrapper.getName(), wrapper);
				}
			}
			return cubes;
		}
		catch (OlapException e) {
			throw new PaloJException(e.getMessage());
		}
	}

	@Override
	public IDimension addDimension(String name) throws PaloException {
		throw new PaloJException("Adding dimension is not supported by OLAP4j provider.");
	}

	@Override
	public ICube addCube(String name, IDimension[] dimensionsNames) throws PaloException {
		throw new PaloJException("Adding cube is not supported by OLAP4j provider.");
	}

	@Override
	public DimensionWrapper getDimensionByName(String name) throws PaloJException {
		DimensionWrapper result = dimensionLookup.get(name);
		if (result == null) {
			try {
				for (Dimension d : getSharedDimensions()) {
					if (d.getName().equals(name)) {
						DimensionWrapper wrapper = new DimensionWrapper(d,getSchema());
						dimensionLookup.put(wrapper.getName(), wrapper);
						return wrapper;
					}
				}
				throw new PaloJException("Dimension "+name+" cannot be found.");
			}
			catch (OlapException e) {
				throw new PaloJException(e.getMessage());
			}
		}
		return result;
	}
	
	@Override
	public ICube getCubeByName(String name) throws PaloJException {
		ICube result = cubeLookup.get(name);
		if (result == null) {
			try {
				for (Cube c : getSchema().getCubes()) {
					if (c.getName().equals(name)) {
						CubeWrapper wrapper = new CubeWrapper(this,c);
						cubeLookup.put(wrapper.getName(), wrapper);
						return wrapper;
					}
				}
				throw new PaloJException("Cube "+name+" cannot be found.");
			}
			catch (OlapException e) {
				throw new PaloJException(e.getMessage());
			}
		}
		return result;
	}

	@Override
	public void removeCube(ICube cube) throws PaloException {
		throw new PaloJException("Renaming cube is not supported by OLAP4j provider.");
	}

	@Override
	public void removeDimension(IDimension dimension) throws PaloException {
		throw new PaloJException("Removing dimension is not supported by OLAP4j provider.");
	}

	@Override
	public void save() throws PaloException {
		//do nothing here, since we cannot create any objects using this wrapper anyway
	}

	@Override
	public void rename(String newname) throws PaloException {
		throw new PaloJException("Renaming database is not supported by OLAP4j provider.");
	}
	
	public void clearLookup() {
		for (DimensionWrapper wrapper : dimensionLookup.values()) {
			wrapper.getElementCache().clear();
		}
		dimensionLookup.clear();
		for (CubeWrapper wrapper: cubeLookup.values()) {
			wrapper.clearLookup();
		}
		cubeLookup.clear();
	}

	@Override
	public DatabaseInfo getDatabaseInfo() {
		throw new PaloJException("getDatabaseInfo is not supported by OLAP4j provider.");
	}

	
	public void setCacheTrustExpiries(int arg0, int arg1, int arg2) {
		//nothing to to
	}

	public void resetCaches() {
		// TODO Auto-generated method stub
	}

	@Override
	public ICube[] getCubes(IDimension arg0) throws PaloException,
			PaloJException {
		// TODO Auto-generated method stub
		return null;
	}

}
