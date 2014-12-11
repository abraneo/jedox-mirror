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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.poi.openxml4j.opc.OPCPackage;
import com.jedox.etl.components.connection.ExcelSAXConnection;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.ExcelUtil;
import com.jedox.etl.core.util.PersistenceUtil;

public class ExcelSAXExtract extends ExcelExtract implements IExtract {
		
	private class ExcelProcessor extends Processor {
		
		private Row row;
		private Iterator<ArrayList<Object>> dataIterator;
		private ArrayList<Object> headerRow;				
		private OPCPackage pkg;
				
		public ExcelProcessor(OPCPackage pkg) {
			this.pkg = pkg;
		}
				
		protected boolean fillRow(Row row) throws Exception {
			if (dataIterator != null && dataIterator.hasNext()) {
				ArrayList<Object> dataRow = (ArrayList<Object>)dataIterator.next();
				try {
					for (int j=0; j<dataRow.size() && j<headerRow.size(); j++) {
						row.getColumn(j).setValue(ExcelUtil.ConvertToColumnType(dataRow.get(j),row.getColumn(j)));
					}
					for (int j=dataRow.size(); j<headerRow.size(); j++) {
						row.getColumn(j).setValue(null);
					}				
				}
				catch (Exception e) {
					throw new RuntimeException("Error in filling row of Excel extract "+e.getMessage());
				}
				return true;
			}
			return false;
		}

		protected Row getRow() {
			return row;
		}
		
		public void close() {
			dataIterator = null;
//			data = null;
			row = null;
		}
				
		@Override
		protected void init() throws RuntimeException {
			if (parser==null) {
				parser = new ExcelSheetParser(pkg, range, sdf);
				parser.process();
			}	
			dataIterator = parser.getData().iterator();
			// set header row
			if (parser.getData().size()==0)
				throw new RuntimeException("Excel file is empty");			
			headerRow= new ArrayList<Object>();	
			if (header) {
				headerRow = dataIterator.next();
				HashSet<Object> map = new HashSet<Object>();
				for(int i=0;i<headerRow.size();i++) {
					Object column=headerRow.get(i);
					if (column==null || column.toString().isEmpty())
						throw new RuntimeException("Header contains empty value at position "+(i+1)+".");
					String columnName=column.toString();					
					if(map.contains(columnName))
						throw new RuntimeException("Column " + columnName + " exists in the header row more than once.");
					map.add(columnName);
				}
			} else {
				for (int i=0; i<parser.getData().get(0).size(); i++)
					headerRow.add("column"+(i+1));				
			}	
							
			row = PersistenceUtil.getColumnDefinition(getAliasMap(),getColumns(headerRow));
			if(typeRow>0){
				ArrayList<Object> firstRow = dataIterator.next();
				for(int i=0;i<firstRow.size();i++){
					if(firstRow.get(i) instanceof String)
						row.getColumn(i).setValueType(String.class);
					else if(firstRow.get(i) instanceof Double)
						row.getColumn(i).setValueType(Double.class);
					else if(firstRow.get(i) instanceof Date)
						row.getColumn(i).setValueType(Date.class);
					else
						row.getColumn(i).setValueType(String.class);
				}
				
				//reset the iterator
				dataIterator = parser.getData().iterator();
				if(header){
					dataIterator.next();	
				}
				ExcelUtil.logRowTypes(row);
			}
			row.setAliases(getAliasMap());			
		}	

		private String[] getColumns(ArrayList<Object> header) {
			String [] names = new String[header.size()];
			for (int i=0; i<names.length; i++) {
				names[i] = header.get(i).toString();
			}
			return names;
		}		
						
	}
		
		
	private ExcelSheetParser parser;
		
	private static final Log log = LogFactory.getLog(ExcelSAXExtract.class);
	
	public ExcelSAXExtract() {
		setConfigurator(new TableSourceConfigurator());
	}

	public ExcelSAXConnection getConnection() throws RuntimeException {
		return (ExcelSAXConnection)super.getConnection();
	}
	
	public void invalidate() {
		super.invalidate();
		parser = null;
	}

	@Override
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		// you have to try to open it before asking for isMemoryOptimised(), since this value may change 
		Object connObj = getConnection().open();
		if (!getConnection().isMemoryOptimised()) {
			return super.getSourceProcessor(size);
		}
		if (size>0)
			range.setRowSize(size+(header?1:0)); // read one additional row if first data row is header
		IProcessor processor = initProcessor(new ExcelProcessor((OPCPackage)connObj),Facets.OUTPUT);
		processor.current().setAliases(getAliasMap());
		processor.setLastRow(size);
		return processor;
	}
	

}
