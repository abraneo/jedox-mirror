package com.jedox.etl.components.connection;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.Properties;

import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.usermodel.WorkbookFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.util.FileUtil;

public class ExcelConnection extends Connection {
	
	private Workbook workbook;
	
	protected String getDataDir() {
		return Settings.getInstance().getDataDir();
	}
	
	public String getDatabase() {
		String database = super.getDatabase();
		if (getHost() == null && FileUtil.isRelativ(database)) {
			String dir = getDataDir();
			database = dir + File.separator + database;
			database = database.replace("/", File.separator);
			database = database.replace("\\", File.separator);
		}
		return database;
	}

	@Override
	public Workbook open() throws RuntimeException {
		if (workbook == null) {
			try {
				InputStream inp = new FileInputStream(getDatabase());
				workbook = WorkbookFactory.create(inp);
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to open workbook: "+e.getMessage());
			}
		}
		return workbook;
	}

	@Override
	public void close() {
		workbook = null;
	}

	@Override
	public String getMetadata(Properties properties) throws RuntimeException {
		throw new RuntimeException("Not implemented in "+this.getClass().getCanonicalName());
	}

	@Override
	public MetadataCriteria[] getMetadataCriterias() {
		// TODO Auto-generated method stub
		return null;
	}

}
