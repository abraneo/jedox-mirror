package com.jedox.etl.core.connection;

import java.io.StringWriter;
import java.net.ConnectException;
import java.net.ProxySelector;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Properties;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.MetadataWriter;
import com.jedox.etl.core.util.ProxyUtil;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.IDatabase.DatabaseType;
import com.jedox.palojlib.interfaces.IDimension.DimensionType;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.main.CellExportContext;

public abstract class AbstractOLAPConnection extends Connection implements
		IOLAPConnection {
	
	private SSLModes sslMode;
	private ProxySelector originalProxySelector;
	protected int httpsPort = 0;
	private IDatabase database;
	private static final String subsetCube = "#_SUBSET_GLOBAL";
	
	public enum MetadataSelectors {
		database, cube, dimension, subset
	}

	public abstract com.jedox.palojlib.interfaces.IConnection open() throws RuntimeException;
	
	public void close(){
		if (database != null)
			database = null;
		
		if (originalProxySelector != null)
			ProxySelector.setDefault(originalProxySelector);
	}
	
	public void setProxyIfSpecified() throws RuntimeException{
		if (hasProxy()) {
			originalProxySelector = ProxySelector.getDefault();
			ProxyUtil selector = new ProxyUtil(originalProxySelector);
			int port = getProxyPort();
			if (getProxyType().equals(ProxyTypes.HTTP)) {
				if(port == -1) port=3128; 
				selector.addHttpProxy(getProxyHost(), port);
			}
			else {//assume socks proxy
				if(port == -1) port=8080;
				selector.addSocksProxy(getProxyHost(), port);
			}
			ProxySelector.setDefault(selector);
		}
	}

	private String getBit(boolean value) {
		if (value) return "1";
		return "0";
	}

	private boolean matches(String key, String mask) {
		for (int i=0; i<Math.min(key.length(), mask.length()); i++) {
			if (key.charAt(i) > mask.charAt(i)) return false;
		}
		return true;
	}


	public StringWriter getDatabases(String mask) throws RuntimeException {
		if ((mask == null) || mask.equals("*")) mask = "111";
		StringWriter out = new StringWriter();		
		MetadataWriter writer = new MetadataWriter(out);
		try {
			IDatabase[] dbs = open().getDatabases();
			String header[] = {"Id","Name","isNormalDatabase","isSystemDatabase","isUserInfoDatabase"};
			writer.println(header);
			for (IDatabase db : dbs) {
				String key = getBit(db.getType().equals(DatabaseType.DATABASE_NORMAL))+getBit(db.getType().equals(DatabaseType.DATABASE_SYSTEM))+getBit(db.getType().equals(DatabaseType.DATABASE_USERINFO));
				if (matches(key,mask)) {
					writer.print(db.getId());
					writer.print(db.getName());
					writer.print(db.getType().equals(DatabaseType.DATABASE_NORMAL));
					writer.print(db.getType().equals(DatabaseType.DATABASE_SYSTEM));
					writer.println(db.getType().equals(DatabaseType.DATABASE_USERINFO));
				}
			}
			//close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Databases: "+e.getMessage());
		}
	}
	
	private StringWriter getSubsets(String database, String dimension) throws RuntimeException {
		
		String dbName = (database!=null?database:getDatabase());
		IDatabase db = getDatabase(dbName,false,true);
		
		if(dimension==null)
			throw new RuntimeException("No dimension is given as a filter.");
			
		IDimension dim = db.getDimensionByName(dimension);
		
		if(dim==null)
			throw new RuntimeException("Dimension " + dimension + "  does not exists.");
		
		ICube cube = db.getCubeByName(subsetCube);

		IElement [][] paths = new IElement[2][];	
		IElement internDim = cube.getDimensionByName("#_DIMENSION_").getElementByName(dimension, false);
		paths[0] = new IElement[]{internDim};
		ICellsExporter exporter = cube.getCellsExporter(paths, new CellExportContext(CellsExportType.BOTH, 10000, false, true, true));
		StringWriter out = new StringWriter();
		MetadataWriter writer = new MetadataWriter(out);
		String header[] = {"GlobalSubsetName"};
		writer.println(header);
		while(exporter.hasNext()){
			ICell cell = exporter.next();
			String[] cellPath = cell.getPathNames();
			writer.println(cellPath[1]);
			//writer.print(cell.getValue());
		}
		//close();
		return out;
	}

	public StringWriter getCubes(String database, String dimensionName, String mask) throws RuntimeException{
		if ((mask == null) || mask.equals("*")) mask = "11111";
		// Support Cube Masks before 3.2 with 4 digits, without first digit for type normal. Signification for Userinfo and GPU changed 
		if (mask.length()==4)
			mask="1"+mask;

		StringWriter out = new StringWriter();
		MetadataWriter writer = new MetadataWriter(out);
		try {
			String dbName = (database!=null?database:getDatabase());
			IDatabase db = getDatabase(dbName,false,true);
			ICube[] cubes = db.getCubes();
			String header[] = {"Id","Name","isNormalCube","isAttributeCube","isUserInfoCube","isSystemCube","isGpuCube"};
			writer.println(header);
			for (ICube cube : cubes) {
				if(dimensionName!=null){
					if(cube.getDimensionByName(dimensionName)==null)
						continue;
				}
				CubeType cubeType = cube.getType();
				String key = getBit(cubeType.equals(CubeType.CUBE_NORMAL))+getBit(cubeType.equals(CubeType.CUBE_ATTRIBUTE))+getBit(cubeType.equals(CubeType.CUBE_USERINFO))+getBit(cubeType.equals(CubeType.CUBE_SYSTEM))+getBit(cubeType.equals(CubeType.CUBE_GPU));
				// Cell properties Cubes are Attribute Cubes from Server, currently not used and so considered as System cubes 
				if(cube.getName().startsWith("#_CELL_PROPERTIES")) {
					key = "00010";
				}				
				if (matches(key,mask)) {
					writer.print(cube.getId());
					writer.print(cube.getName());
					writer.print(cubeType.equals(CubeType.CUBE_NORMAL));
					writer.print(cubeType.equals(CubeType.CUBE_ATTRIBUTE));
					writer.print(cubeType.equals(CubeType.CUBE_USERINFO));
					writer.print(cubeType.equals(CubeType.CUBE_SYSTEM));
					writer.println(cubeType.equals(CubeType.CUBE_GPU));
				}
			}
			//close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Cubes for database "+database+": "+e.getMessage());
		}
	}

	private StringWriter printDimensions(IDimension[] dims, String mask) {
		StringWriter out = new StringWriter();
		MetadataWriter writer = new MetadataWriter(out);
		String header[] = {"Id","Name","maxDepth","maxLevel","isNormalDimension","isAttributeDimension","isSystemDimension","isUserInfoDimension","isSystemIdDimension"};
		writer.println(header);
		for (IDimension dim : dims) {
			String key = getBit(dim.getType().equals(DimensionType.DIMENSION_NORMAL))+getBit(dim.getType().equals(DimensionType.DIMENSION_ATTRIBUTE))+getBit(dim.getType().equals(DimensionType.DIMENSION_SYSTEM))+getBit(dim.getType().equals(DimensionType.DIMENSION_USERINFO))+getBit(dim.getType().equals(DimensionType.DIMENSION_SYSTEM_ID));
			if (matches(key,mask)) {
				writer.print(dim.getId());
				writer.print(dim.getName());
				writer.print(dim.getDimensionInfo().getMaximumDepth());
				writer.print(dim.getDimensionInfo().getMaximumLevel());
				writer.print(dim.getType().equals(DimensionType.DIMENSION_NORMAL));
				writer.print(dim.getType().equals(DimensionType.DIMENSION_ATTRIBUTE));
				writer.print(dim.getType().equals(DimensionType.DIMENSION_SYSTEM));
				writer.print(dim.getType().equals(DimensionType.DIMENSION_USERINFO));
				writer.println(dim.getType().equals(DimensionType.DIMENSION_SYSTEM_ID));
			}
		}
		return out;
	}

	public StringWriter getDimensions(String database, String mask) throws RuntimeException{
		if ((mask == null) || mask.equals("*"))
			mask = "11111";
		if (mask.length()==4) // Support Dimension Masks before 5.0 without SystemID-type
			mask=mask.concat("0");	
		if (mask.length()==3)
			mask="1"+mask;		
		try {
			String dbName = (database!=null?database:getDatabase());
			IDatabase db = getDatabase(dbName,false,true);
			IDimension[] dims = db.getDimensions();
			StringWriter out = printDimensions(dims,mask);
			//close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Dimensions for database "+database+": "+e.getMessage());
		}
	}

	public StringWriter getCubeDimensions(String database, String cube, String mask) throws RuntimeException{
		if ((mask == null) || mask.equals("*")) 
			mask = "11111";
		if (mask.length()==4) // Support Dimension Masks before 5.0 without SystemID-type. 
			mask=mask.concat("1"); // By default show cube dimensions of all types		

		try {
			String dbName = (database!=null?database:getDatabase());
			IDatabase db = getDatabase(dbName,false,true);
			ICube c = db.getCubeByName(cube);
			if(c==null){
				throw new RuntimeException("Cube " + cube + " is not found in database " +database);
			}
			IDimension[] dims = c.getDimensions();
			StringWriter out = printDimensions(dims,mask);
			//close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Dimensions for database "+database+" Cube "+cube+": "+e.getMessage());
		}
	}
	
	public MetadataCriteria[] getMetadataCriterias() {
		String[] databaseFilters = {"mask"};
		String[] cubeFilters = {"database","dimension","mask"};
		String[] dimensionFilters = {"database","cube","mask"};
		String[] subsetFilters = {"database","dimension"};
		
		ArrayList<MetadataCriteria> criterias = new ArrayList<MetadataCriteria>();		
		for (MetadataSelectors s : MetadataSelectors.values()) {
			MetadataCriteria c = new MetadataCriteria(s.toString());
			switch (s) {
			case database: {c.setFilters(databaseFilters); break; }
			case cube: { c.setFilters(cubeFilters); break; }
			case dimension: { c.setFilters(dimensionFilters); break;} 
			case subset: { c.setFilters(subsetFilters); } 
			}
			criterias.add(c);
		}
		return criterias.toArray(new MetadataCriteria[criterias.size()]);
	}	
	
	public String getMetadata(Properties properties) throws RuntimeException {
		String selector = properties.getProperty("selector");
		String database = properties.getProperty("database");
		String mask = properties.getProperty("mask");
		String cube = properties.getProperty("cube");
		String dimension = properties.getProperty("dimension");
		MetadataSelectors sel;
		try {
			sel = MetadataSelectors.valueOf(selector);
		}
		catch (Exception e) {
			throw new RuntimeException("Property 'selector' must be one of: "+getMetadataSelectorValues());
		}
		switch (sel) {
		case database: return getDatabases(mask).toString();
		case cube: return getCubes(database,dimension,mask).toString();
		case dimension: {
			if (cube != null)
				return getCubeDimensions(database,cube,mask).toString();
			else
				return getDimensions(database,mask).toString();
		}
		case subset:
			return getSubsets(database,dimension).toString();
		default: return null;
		}
	}

	public String getSslUrl(){
		return "https://" + getHost() + ":" +  httpsPort;
	}
	
	protected void checkSSL() throws RuntimeException {
		try {
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.verify.toString()));
			if (httpsPort!=0 && sslMode.equals(SSLUtil.SSLModes.trust)) {
//				String urlString = getHost() + (getPort() != null ? ":"+getPort() : "");
				String urlString = getSslUrl();
				try {
					URL url = new URL(urlString);
					SSLUtil.getInstance().addCertToKeyStore(url);
				}
				catch (Exception e) {
					if (e instanceof UnknownHostException)
						throw new RuntimeException("Host "+urlString+" is unknown.");
					if (e instanceof ConnectException)
						throw new RuntimeException("Could not connect to host "+urlString+" : "+e.getMessage());
					throw new RuntimeException(e);						
				}
			}
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	protected SSLModes getSSLMode() {
		return sslMode;
	}
	
	public IDatabase getDatabase(boolean createIfExists, boolean throwExceptionIfNotExists) throws RuntimeException {
		if (database != null)
			return database;
		else {
			database = getDatabase(getDatabase(), createIfExists, throwExceptionIfNotExists);
			return database;
		}
	}

	private IDatabase getDatabase(String dbName, boolean createIfExists, boolean throwExceptionIfNotExists) throws RuntimeException {
		IDatabase db = null;
		try {
			db = open().getDatabaseByName(dbName);
			if (db != null) {
				return db;
			} else {
				if (createIfExists) {
					db = open().addDatabase(dbName);
					return db;
				} else {
					if (throwExceptionIfNotExists)
						throw new Exception("Database " + dbName + " does not exist in connection " + getName());
					else
						return null;
				}
			}
		} catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}

	}
	
	public void init() throws InitializationException {
		super.init();
	}
	

}
