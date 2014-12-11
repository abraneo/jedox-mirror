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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.connection;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.poi.openxml4j.opc.OPCPackage;
import org.apache.poi.poifs.crypt.Decryptor;
import org.apache.poi.poifs.crypt.EncryptionInfo;
import org.apache.poi.poifs.filesystem.NPOIFSFileSystem;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.usermodel.WorkbookFactory;

import com.jedox.etl.components.extract.ExcelSheetParser;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.MetadataWriter;
import com.jedox.etl.core.util.PersistenceUtil;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;

public class ExcelConnection extends Connection {
	
	
	protected SSLModes sslMode = null;
	
	public enum MetadataSelectors {
		worksheet
	}
	
	protected class ExcelMetaDataProcessor extends Processor {

		private Row row;
		int index = 0;
		private ArrayList<String> sheetNames = new ArrayList<String>();

		public ExcelMetaDataProcessor(int size) {
			this.setLastRow(size);
		}

		protected void init() throws RuntimeException{
			String[] columnsNames = new String[]{"SheetName"};
			row = PersistenceUtil.getColumnDefinition(columnsNames);
			Object connObj = open();
			if(connObj instanceof org.apache.poi.ss.usermodel.Workbook){
				workbook = (Workbook)open();
				String sheetName = null;
				int i=0;
				try {
					sheetName = workbook.getSheetName(i);
				} catch (Exception e) {}
				while(sheetName!=null){
					sheetNames.add(sheetName);
					i++;
					try {
						sheetName = workbook.getSheetName(i);
					} catch (Exception e) {sheetName = null;}
				}
			}else{
				ExcelSheetParser parser = new ExcelSheetParser((OPCPackage) open(), null, null);
				sheetNames = parser.getWorkSheets();
			}
			
		}


		protected boolean fillRow(Row row) throws Exception {
			if (sheetNames.size()>=(index+1)) {
				row.getColumn(0).setValue(sheetNames.get(index));
				index++;
				return true;
			}
			else { //finished ... do some cleanup
				index = 0;
				return false;
			}
		}

		protected Row getRow() {
			return row;
		}
		
		public void close(){
			index = 0;
			row = null;
			sheetNames.clear();
		}

	}

	
	private Workbook workbook = null;
	private NPOIFSFileSystem fs = null;
	private static final Log log = LogFactory.getLog(ExcelConnection.class);
	protected OPCPackage pkg = null;
	
	
	public String getDatabase() {
		String database = super.getDatabase();
		if (getHost() == null && FileUtil.isRelativ(database))
			return FileUtil.convertPath(Settings.getInstance().getDataDir() + "/" + database);
		else	
			return database;
	}

	@Override
	public Object open() throws RuntimeException {
		if (workbook == null) {
			try {				
				// Creating Workbook would be faster via File than via InputStrem, but not possible with ssl-trust
				InputStream inp = null;
				try{
					pkg = OPCPackage.open(new File(getDatabase()));
					workbook =  WorkbookFactory.create(pkg);
				}catch(Exception e){
					try {
						inp = FileUtil.getInputStream(getDatabase(), false, sslMode);
						String password = getParameter("password","");
						if(!password.isEmpty()){
							NPOIFSFileSystem fs = new NPOIFSFileSystem(inp);
							EncryptionInfo info = new EncryptionInfo(fs);
							Decryptor d = Decryptor.getInstance(info);
							if (!d.verifyPassword(password)) {
					            throw new RuntimeException("password is incorrect.");
					        }
							inp = d.getDataStream(fs);
						}
						pkg = OPCPackage.open(inp);
						workbook =  WorkbookFactory.create(pkg);
					} catch (Exception e1) {
						String password = getParameter("password","");
						if(!password.isEmpty()){
							org.apache.poi.hssf.record.crypto.Biff8EncryptionKey.setCurrentUserPassword(password);
						}
						try {
							if(inp!=null) inp.close();
							fs = new NPOIFSFileSystem(new File(getDatabase()));
							workbook =  WorkbookFactory.create(fs);
						} catch (Exception e2) {
							inp = FileUtil.getInputStream(getDatabase(), false, sslMode);
							fs = new NPOIFSFileSystem(inp);
							workbook =  WorkbookFactory.create(fs);
						}
					}
				}				
			}
			catch (Exception e) {	
				throw new RuntimeException("Failed to open workbook: "+e.getMessage());
			}
		}
		return workbook;
	}

	@Override
	public void close() {
		if(pkg!=null){		
			try {
				pkg.close();
			} catch (IOException e) {}
			pkg=null;		
		}
		
		if(fs!=null){			
			try {
				fs.close();
			} catch (IOException e) {}
			fs=null;		
		}
		workbook = null;
		org.apache.poi.hssf.record.crypto.Biff8EncryptionKey.setCurrentUserPassword(null);
	}

	@Override
	public String getMetadata(Properties properties) throws RuntimeException {
		String selector = properties.getProperty("selector");
		
		MetadataSelectors sel;
		try {
			sel = MetadataSelectors.valueOf(selector);
		}
		catch (Exception e) {
			throw new RuntimeException("Property 'selector' must be one of: "+getMetadataSelectorValues());
		}
		switch (sel) {
		case worksheet: return getWorksheets().toString();
		default: return null;
		}
	}

	@Override
	public MetadataCriteria[] getMetadataCriterias() {
		ArrayList<MetadataCriteria> criterias = new ArrayList<MetadataCriteria>();		
		for (MetadataSelectors s : MetadataSelectors.values()) {
			MetadataCriteria c = new MetadataCriteria(s.toString());
			criterias.add(c);
		}
		return criterias.toArray(new MetadataCriteria[criterias.size()]);
	}
	
	protected StringWriter getWorksheets() throws RuntimeException{
		log.info("getting worksheets from connection: "+getName());
		
		StringWriter out = new StringWriter();		
		MetadataWriter writer = new MetadataWriter(out);
		try {
			ExcelMetaDataProcessor processor = new ExcelMetaDataProcessor(0);
			writer.write(initProcessor(processor, Facets.OUTPUT ));
			return out;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get worksheet for connection "+getName()+": "+e.getMessage());
		}
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.verify.toString()));
		} catch (ConfigurationException e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
