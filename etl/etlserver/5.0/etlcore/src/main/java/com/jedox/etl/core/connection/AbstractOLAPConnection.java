package com.jedox.etl.core.connection;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.ProxySelector;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Properties;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.util.ProxyUtil;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.IDatabase.DatabaseType;
import com.jedox.palojlib.interfaces.IDimension.DimensionType;

public abstract class AbstractOLAPConnection extends Connection implements
		IOLAPConnection {
	
	private SSLModes sslMode;
	private ProxySelector originalProxySelector;
	
	public enum MetadataSelectors {
		database, cube, dimension
	}

	public abstract com.jedox.palojlib.interfaces.IConnection open() throws RuntimeException;
	
	public abstract void close();
	
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
	
	public void clearProxyIfExists(){
		if (originalProxySelector != null)
			ProxySelector.setDefault(originalProxySelector);
		
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
		PrintWriter writer = new PrintWriter(out);
		try {
			IDatabase[] dbs = open().getDatabases();
			writer.println("Id;Name;isNormalDatabase;isSystemDatabase;isUserInfoDatabase");
			for (IDatabase db : dbs) {
				String key = getBit(db.getType().equals(DatabaseType.DATABASE_NORMAL))+getBit(db.getType().equals(DatabaseType.DATABASE_SYSTEM))+getBit(db.getType().equals(DatabaseType.DATABASE_USERINFO));
				if (matches(key,mask)) {
					writer.print(db.getId()+";");
					writer.print(db.getName()+";");
					writer.print(db.getType().equals(DatabaseType.DATABASE_NORMAL)+";");
					writer.print(db.getType().equals(DatabaseType.DATABASE_SYSTEM)+";");
					writer.println(db.getType().equals(DatabaseType.DATABASE_USERINFO));
				}
			}
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Databases: "+e.getMessage());
		}
	}

	public StringWriter getCubes(String database, String dimensionName, String mask) throws RuntimeException{
		if ((mask == null) || mask.equals("*")) mask = "11111";
		// Support Cube Masks before 3.2 with 4 digits, without first digit for type normal. Signification for Userinfo and GPU changed 
		if (mask.length()==4)
			mask="1"+mask;
	
		if (database == null)
			database = getDatabase();
		StringWriter out = new StringWriter();
		PrintWriter writer = new PrintWriter(out);
		try {
			IDatabase db = open().getDatabaseByName(database);
			if (db==null) {
				throw new RuntimeException("Database "+database+" not found");
			}
			ICube[] cubes = db.getCubes();
			writer.println("Id;Name;isNormalCube;isAttributeCube;isUserInfoCube;isSystemCube;isGpuCube");
			for (ICube cube : cubes) {
				if(dimensionName!=null){
					if(cube.getDimensionByName(dimensionName)==null)
						continue;
				}
				//Special Case: add a check if the cube name starts with "#_#". It should be considered as a
				//system Cube rather than an attribute cube as it is delivered from JPalo
				/*if(cube.getName().startsWith("#_#")){
					if(mask.charAt(2) == '1'){// From mask, system cubes should be included
						writer.print(cube.getId()+";");
						writer.print(cube.getName()+";");
						writer.print(cube.getType().equals(CubeType.CUBE_ATTRIBUTE)+";");
						writer.print(false+";");
						writer.print(cube.getType().equals(CubeType.CUBE_SYSTEM)+";");
						writer.println(cube.getType().equals(CubeType.CUBE_USERINFO));
						continue;
					}
					else if(mask.charAt(0) == '1'){// From mask, attributes cubes should be included
						continue;
					}
					else{}
				}*/
								
				// General Case
				String key = getBit(cube.getType().equals(CubeType.CUBE_NORMAL))+getBit(cube.getType().equals(CubeType.CUBE_ATTRIBUTE))+getBit(cube.getType().equals(CubeType.CUBE_USERINFO))+getBit(cube.getType().equals(CubeType.CUBE_SYSTEM))+getBit(cube.getType().equals(CubeType.CUBE_GPU));
				// Cell properties Cubes are Attribute Cubes from Server, currently not used and so considered as System cubes 
				if(cube.getName().startsWith("#_CELL_PROPERTIES")) {
					key = "00010";
				}
				
				if (matches(key,mask)) {
					writer.print(cube.getId()+";");
					writer.print(cube.getName()+";");
					writer.print(cube.getType().equals(CubeType.CUBE_NORMAL)+";");
					writer.print(cube.getType().equals(CubeType.CUBE_ATTRIBUTE)+";");
					writer.print(cube.getType().equals(CubeType.CUBE_USERINFO)+";");
					writer.print(cube.getType().equals(CubeType.CUBE_SYSTEM)+";");
					writer.println(cube.getType().equals(CubeType.CUBE_GPU));
				}
			}
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Cubes for database "+database+": "+e.getMessage());
		}
	}

	private void printDimensions(PrintWriter writer, IDimension[] dims, String mask) {
		writer.println("Id;Name;maxDepth;maxLevel;isNormalDimension;isAttributeDimension;isSystemDimension;isUserInfoDimension");
		for (IDimension dim : dims) {
			String key = getBit(dim.getType().equals(DimensionType.DIMENSION_NORMAL))+getBit(dim.getType().equals(DimensionType.DIMENSION_ATTRIBUTE))+getBit(dim.getType().equals(DimensionType.DIMENSION_SYSTEM))+getBit(dim.getType().equals(DimensionType.DIMENSION_USERINFO));
			if (matches(key,mask)) {
				writer.print(dim.getId()+";");
				writer.print(dim.getName()+";");
				writer.print(dim.getDimensionInfo().getMaximumDepth()+";");
				writer.print(dim.getDimensionInfo().getMaximumLevel()+";");
				writer.print(dim.getType().equals(DimensionType.DIMENSION_NORMAL)+";");
				writer.print(dim.getType().equals(DimensionType.DIMENSION_ATTRIBUTE)+";");
				writer.print(dim.getType().equals(DimensionType.DIMENSION_SYSTEM)+";");
				writer.println(dim.getType().equals(DimensionType.DIMENSION_USERINFO));
			}
		}
	}

	public StringWriter getDimensions(String database, String mask) throws RuntimeException{
		if ((mask == null) || mask.equals("*")) mask = "1111";
		// Support Dimension Masks before 3.2 with 3 digits, without first digit for type normal.
		if (mask.length()==3)
			mask="1"+mask;		
		if (database == null)
			database = getDatabase();
		StringWriter out = new StringWriter();
		PrintWriter writer = new PrintWriter(out);
		try {
			IDatabase db = open().getDatabaseByName(database);
			if (db==null) {
				throw new RuntimeException("Database "+database+" not found");
			}
			IDimension[] dims = db.getDimensions();
			printDimensions(writer,dims,mask);
			close();
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get OLAP-Dimensions for database "+database+": "+e.getMessage());
		}
	}

	public StringWriter getCubeDimensions(String database, String cube, String mask) throws RuntimeException{
		if ((mask == null) || mask.equals("*")) mask = "1111";
		if (database == null)
			database = getDatabase();
		StringWriter out = new StringWriter();
		PrintWriter writer = new PrintWriter(out);
		try {
			IDatabase db = open().getDatabaseByName(database);
			if (db==null) {
				throw new RuntimeException("Database "+database+" not found");
			}
			ICube c = db.getCubeByName(cube);
			if(c==null){
				throw new RuntimeException("Cube " + cube + " is not found in database " +database);
			}
			IDimension[] dims = c.getDimensions();
			printDimensions(writer,dims,mask);
			close();
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
		
		ArrayList<MetadataCriteria> criterias = new ArrayList<MetadataCriteria>();		
		for (MetadataSelectors s : MetadataSelectors.values()) {
			MetadataCriteria c = new MetadataCriteria(s.toString());
			switch (s) {
			case database: {c.setFilters(databaseFilters); break; }
			case cube: { c.setFilters(cubeFilters); break; }
			case dimension: { c.setFilters(dimensionFilters); } 
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
		default: return null;
		}
	}
	
	protected void checkSSL() throws RuntimeException {
		try {
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.verify.toString()));
			if (sslMode.equals(SSLUtil.SSLModes.trust)) {
//				String urlString = getHost() + (getPort() != null ? ":"+getPort() : "");
				String urlString = getHost();
				try {
					SSLUtil util = new SSLUtil();
					URL url = new URL(urlString);
					util.addCertToKeyStore(url);
				}
				catch (UnknownHostException e) {
					throw new RuntimeException("Host "+urlString+" is unknown.");
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
	
	public void init() throws InitializationException {
		super.init();
	}
	

}
