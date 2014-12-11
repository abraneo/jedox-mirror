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
package com.jedox.etl.components.extract;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.poi.hssf.util.CellReference;
import org.apache.poi.ss.usermodel.CellValue;
import org.apache.poi.ss.usermodel.DateUtil;
import org.apache.poi.ss.usermodel.FormulaEvaluator;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.usermodel.Cell;

import com.jedox.etl.components.connection.ExcelConnection;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.ExcelUtil;

public class ExcelExtract extends TableSource implements IExtract {
	
	public class Range {
		
		private String sheetName=null;
		private int minColumn=1;
		private int maxColumn=Integer.MAX_VALUE;
		private int minRow=1;
		private int maxRow=Integer.MAX_VALUE;
		private String startCell;
		private String endCell;		
		private boolean isDefault=true;
		private ArrayList<Integer> inclusionSet;	
		
		public String getSheetName() {
			return sheetName;
		}		
		public int getMinRow() {
			return minRow;
		}
		public int getMaxRow() {
			return maxRow;
		}
		public boolean isDefault() {
			return isDefault;
		}
		public String getStartCell() {
			return startCell;
		}
		public String getEndCell() {
			return endCell;
		}

		public void setSheetName(String sheetName) {
			this.sheetName=sheetName;
		}
				
		public void setInclusionSet(ArrayList<Integer> inclusionSet) {
			this.inclusionSet = inclusionSet;
		}
		
		public ArrayList<Integer> getInclusionSet() {
			return inclusionSet;
		}
		
		public boolean isIncludedColumn (int index) {
			if (index<minColumn || index>maxColumn)
				return false;
			return (inclusionSet==null) || (inclusionSet.isEmpty()) || (inclusionSet.contains(index-minColumn+1));
		}
					
		private void checkCell(String cell, String type) throws RuntimeException {
			if (!cell.matches("\\$?[a-zA-Z]{1,}\\$?[0-9]{1,}") && 
				!cell.matches("\\$?[a-zA-Z]{1,}"))
					throw new RuntimeException(type+" cell needs to be in a form like $B$3 or $B");						
		}
		
		private int getFirstDigit(String name) {
            for (int c = 0; c < name.length(); c++) {
                if (Character.isDigit(name.charAt(c))) {
                    return c;
                }
            }
            return -1;
		}    
				
		public int getColumnIndex(String cell) {
			// support cells in form $AC$12 and AC12
			String name=cell.replaceAll("\\$", "");
            int firstDigit = getFirstDigit(name);
            name=name.substring(0, firstDigit);                
            int column = 0;
            for (int i=0; i<name.length(); i++) {
                int c = name.charAt(i);
                column = column*26 + c - 'A' + 1;
            }
            return column;
        }			

		public int getRowIndex(String cell, int defaultValue) {
			// Cell in form $AC$12 -> Column 28
			// Cell in form $AC -> defaultValue		
            int firstDigit = getFirstDigit(cell);
            String name=cell.substring(firstDigit);
            if (firstDigit==-1)
            	return defaultValue;
            else 
            	return Integer.parseInt(name);
		}  
			
		private void check() throws RuntimeException {
			if (!(minRow<=maxRow && minColumn<=maxColumn)) {
				throw new RuntimeException("End cell can not exist in the upper-left side of the start cell.");
			}			
		}
		
		public void setRowSize(int size) {
			maxRow=Math.min(maxRow,size+minRow-1);
		}
		
		public Range (String query) throws RuntimeException {
			if (query!=null && !query.trim().isEmpty()) {
				
				int index = query.lastIndexOf('!');
				if(index==-1){
					log.debug("No '!' was found, so the query is only the sheet name.");
					sheetName = query;
					return;
				}
				if(index==0){
					throw new RuntimeException("No sheet is given.");
				}
				if(index==query.length()-1){
					throw new RuntimeException("No cells are given.");
				}
				sheetName = query.substring(0, index);
				
				String[] cells = query.substring(index+1).split(":");
				if (cells.length != 2) {
					throw new RuntimeException("Start and end cells must be defined");
				}
				startCell=cells[0];
				checkCell(startCell,"Start");
				minRow=getRowIndex(startCell,1);
				minColumn=getColumnIndex(startCell);
				
				endCell=cells[1];
				checkCell(endCell,"End");
				maxRow=getRowIndex(endCell,Integer.MAX_VALUE);
				maxColumn=getColumnIndex(endCell);
				check();
				isDefault=false;
			}			
		}	
	}
	
	
	private class ExcelProcessor extends Processor {
		
		private Sheet sheet;
		private CellReference start;
		private CellReference end;
		private FormulaEvaluator evaluator;
		private com.jedox.etl.core.node.Row processorRow = new com.jedox.etl.core.node.Row();
		private int currentRow;
		private Workbook workbook;
		private Range range;
		
		public ExcelProcessor(Workbook workbook, Range range) {
			this.workbook = workbook;
			this.range = range;			
		}
		
		protected Object getCellValue(Cell cell) {
			if (cell == null) return null;
			switch (cell.getCellType()) {
	            case Cell.CELL_TYPE_STRING: return cell.getRichStringCellValue().getString();
	            case Cell.CELL_TYPE_NUMERIC:
	                if (DateUtil.isCellDateFormatted(cell)) {
	                	Date date = cell.getDateCellValue();
	                	if(sdf!=null)
	                		return sdf.format(date);
	                	else
	                		return date;
	                } else {
	                	return cell.getNumericCellValue();
	                }
	            case Cell.CELL_TYPE_BOOLEAN: return String.valueOf(cell.getBooleanCellValue());
	            case Cell.CELL_TYPE_FORMULA: {
	            	CellValue cellValue = evaluator.evaluate(cell);
	            	switch (cellValue.getCellType()) {
	            	 	case Cell.CELL_TYPE_STRING: return cellValue.getStringValue();
		                case Cell.CELL_TYPE_NUMERIC:
		                	{
		                		if (DateUtil.isCellDateFormatted(cell)) {
		    	                	Date date = cell.getDateCellValue();
		    	                	if(sdf!=null)
		    	                		return sdf.format(date);
		    	                	else
		    	                		return date;
		    	                } else {
		    	                	return cell.getNumericCellValue();
		    	                }
		                	
		                	}
		                case Cell.CELL_TYPE_BOOLEAN: return String.valueOf(cell.getBooleanCellValue());
		                default: return null;
	            	}
	            }
	            default: return null;
	        }
		}
		
		
		
		protected Class<?> getValueType(Cell cell) {
			if (cell==null) return String.class;
			switch (cell.getCellType()) {
	            case Cell.CELL_TYPE_STRING: return String.class;
	            case Cell.CELL_TYPE_NUMERIC:
	                if (DateUtil.isCellDateFormatted(cell)) {
	                		return (sdf==null?Date.class:String.class);
	                } else {
	                	return Double.class;
	                }
	            case Cell.CELL_TYPE_BOOLEAN: return String.class;
	            case Cell.CELL_TYPE_FORMULA: {
	            	CellValue cellValue = evaluator.evaluate(cell);
	            	switch (cellValue.getCellType()) {
	            	 	case Cell.CELL_TYPE_STRING: return String.class;
		                case Cell.CELL_TYPE_NUMERIC: return Double.class;
		                case Cell.CELL_TYPE_BOOLEAN: return String.class;
		                default: String.class.getCanonicalName();
	            	}
	            }
	            default: return String.class;
	        }
		}

		private boolean checkIfWholeRowIsEmpty(Row sheetRow){
			for (int j=start.getCol(); j<=end.getCol(); j++) {
				Cell cell = sheetRow.getCell(j);
				if(cell!=null){
					return false;
				}
			}
			
			return true;
		}
		
		@Override
		protected boolean fillRow(com.jedox.etl.core.node.Row row) throws Exception {
			if (currentRow > sheet.getLastRowNum() || currentRow > end.getRow()) return false;
			Row sheetRow = sheet.getRow(currentRow);
			if (sheetRow != null) {
				boolean emptyRow = true;
				if(range.getInclusionSet().size()==0){
					for (int j=start.getCol(); j<=end.getCol(); j++) {
						IColumn column = row.getColumn(j-start.getCol());
						Cell cell = sheetRow.getCell(j);
						if(emptyRow && cell!=null){		
							emptyRow = false;
						}
						column.setValue(ExcelUtil.ConvertToColumnType(getCellValue(cell),column));	
			        }
				}else{
					Iterator<Integer> iter = range.getInclusionSet().iterator();
					int i=0;
					while (iter.hasNext()) {
						int index = iter.next() - 1;
						IColumn column = row.getColumn(i);
						Cell cell = sheetRow.getCell(index+start.getCol());
						if(emptyRow && cell!=null){
							emptyRow = false;
						}							
						column.setValue(ExcelUtil.ConvertToColumnType(getCellValue(cell),column));
						i++;	
						}
					if(end.getRow()==Integer.MAX_VALUE && emptyRow)
						emptyRow = checkIfWholeRowIsEmpty(sheetRow);
			        
				}
				if(end.getRow()==Integer.MAX_VALUE && emptyRow){
					return false;
				}
			} else { 
				if(end.getRow()==Integer.MAX_VALUE){
					return false;
				}
				//set all columns to null if sheetRow is null
				for (IColumn c : row.getColumns()) {
					c.setValue(null);
				}
			}
			currentRow++;
			return true;
		}

		@Override
		protected com.jedox.etl.core.node.Row getRow() {
			return processorRow;
		}
		
		public void close() {
			try {
				// Huge memory consumption if Excel Workbook data is reused
				getConnection().close();
				sheet=null;
				start=null;
				end=null;
				evaluator=null;
				processorRow=null;
				workbook=null;
				range=null;
			} catch (RuntimeException e) {
				log.error("Error in closing connection for "+getName()+": "+e.getMessage());
			}
		}

		
		
		private CellReference getCellReference(String cell, int defaultRows, String type) throws RuntimeException {
			try{
				if (cell.matches("\\$?[a-zA-Z]{1,}\\$?[0-9]{1,}"))
					return new CellReference(cell);
				else if (cell.matches("\\$?[a-zA-Z]{1,}"))
					return new CellReference(cell+"$"+defaultRows);
				else
					throw new RuntimeException(type+" cell needs to be in a form like $B$3 or $B");						
			}catch(Exception e){
				throw new RuntimeException(type+" cell can not be definded, maybe the cell index is too big for this excel version.");
			}
			
		}
				
		@Override
		protected void init() throws RuntimeException {
			if(!range.isDefault){
				sheet = workbook.getSheet(range.getSheetName());	
				if (sheet == null) {
					throw new RuntimeException("Excel workbook does not contain a sheet named "+range.getSheetName()+".");
				}				
				start = getCellReference(range.getStartCell(), sheet.getFirstRowNum()+1, "Start");
				end = getCellReference(range.getEndCell(), sheet.getLastRowNum()+1, "End");
								
				if(!(start.getRow()<=end.getRow() && start.getCol()<=end.getCol())){
					throw new RuntimeException("End cell can not exist in the upper-left side of the start cell.");
				}	
				if (sheet.getLastRowNum() < start.getRow()) {
					throw new RuntimeException("Starting row "+(start.getRow()+1)+" exeeds available rows: "+(sheet.getLastRowNum()+1));
				}
			}else{
				// get the default area
				if(range.getSheetName()!=null){
					sheet = workbook.getSheet(range.getSheetName());
					if (sheet == null)
						throw new RuntimeException("Excel workbook does not contain a sheet " + range.getSheetName());
				}else{
					sheet = workbook.getSheetAt(0);
					if (sheet == null)
						throw new RuntimeException("Excel workbook does not contain any sheet.");
				}
				log.debug("Using \"" + sheet.getSheetName() + "\" sheet.");
				int firstRowNum = sheet.getFirstRowNum();
				if(sheet.getRow(firstRowNum)==null){
					throw new RuntimeException("Sheet named " + sheet.getSheetName() + " is empty.");
				}
				int firstColIndex = -1;
				int lastColIndex = -1;
				int maxLastColIndex=1000;
				int count = 0;
				while(count<maxLastColIndex){
					Cell cell = sheet.getRow(firstRowNum).getCell(count);
					if(cell==null){
						if(firstColIndex!=-1){
							lastColIndex=count-1;
							break;
						}
					}
					Object cellValue = getCellValue(cell);
					if(cellValue!=null && firstColIndex==-1){
						firstColIndex = count;
					}else if(cellValue==null && firstColIndex!=-1){
						lastColIndex=count-1;
						break;
					}
					count++;
				}
				if(lastColIndex==-1 && firstColIndex!=-1){
					lastColIndex=maxLastColIndex-1;
				}
				
				if(lastColIndex==-1 || firstColIndex==-1){
					throw new RuntimeException("The columns first and last index could not be detected.");
				}
				
				start = new CellReference(firstRowNum, firstColIndex);				
				end = new CellReference(Integer.MAX_VALUE, lastColIndex);			
				log.info("Excel cell range starting at "+sheet.getSheetName()+"!"+
						"$"+CellReference.convertNumToColString(firstColIndex)+"$"+(firstRowNum+1));
//						":$"+CellReference.convertNumToColString(lastColIndex)+"$"+(firstRowNum+1));
			}
			
			evaluator = workbook.getCreationHelper().createFormulaEvaluator();
			currentRow = start.getRow();
			
			Row startRow = sheet.getRow(start.getRow());
			if(startRow==null && header){
				throw new RuntimeException("The header row is empty.");
			}
			
			Row typeRowObj = null;
			int typeRowIndex = 0;
			if(typeRow>0){
				typeRowIndex = start.getRow()+typeRow;
				if(!header){
					typeRowIndex--;
				}
				
				typeRowObj = sheet.getRow(typeRowIndex);
			}
			
			for (int i=start.getCol(); i<=end.getCol(); i++) {
				if(range.getInclusionSet().size()==0 || range.getInclusionSet().contains(i-start.getCol()+1)){
					CellReference r = new CellReference(start.getRow(),i);
					
					Cell cell = null;
					if(startRow!=null)
						cell = startRow.getCell(i);
					
					if(cell==null  && header){
						throw new RuntimeException("Cells in the header must not be empty.");
					}
					Object cellValue = getCellValue(cell);					
					Class<?> valueType = String.class;
					
					if(typeRowObj== null || typeRowObj.getCell(i)== null){
						log.debug("Type row at position " + r.getCellRefParts()[2] + "$" +(typeRowIndex+1) + " is empty and therefor can not be used as a type cell, String is used instead.");
					}
					
					if(typeRowObj!= null && typeRowObj.getCell(i)!= null){
						valueType = getValueType(typeRowObj.getCell(i));
					}
					String name = (header && cellValue != null) ? cellValue.toString() : r.getCellRefParts()[2];
					if(header){
						if(processorRow.getColumn(name)!=null){
							throw new RuntimeException("Column " + name + " exists in the header row more than once.");
						}
						if (name.trim().isEmpty()) {
							throw new RuntimeException("A cell value in the header is empty.");
						}	
					}
					Column c = new Column(name);
					c.setValueType(valueType);
					processorRow.addColumn(c);
				}
			}
			if(typeRow>0)
				ExcelUtil.logRowTypes(processorRow);
			if (header) currentRow++;
		}
		
	}
	
	
	private static final Log log = LogFactory.getLog(ExcelExtract.class);
	
	protected boolean header;	
	protected int typeRow;
	protected SimpleDateFormat sdf = null;	
	protected Range range;
	
	public ExcelExtract() {
		setConfigurator(new TableSourceConfigurator());
	}

	public TableSourceConfigurator getConfigurator() {
		return (TableSourceConfigurator) super.getConfigurator();
	}


	public ExcelConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof ExcelConnection))
			return (ExcelConnection) connection;
		throw new RuntimeException("Excel connection is needed for extract "+getName()+".");
	}

	@Override
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(new ExcelProcessor((Workbook)getConnection().open(), range),Facets.OUTPUT);
		processor.current().setAliases(getAliasMap());
		processor.setLastRow(size);
		return processor;
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			header = getParameter("header","false").equalsIgnoreCase("true");

			if(Boolean.parseBoolean(getParameter("firstRowType","false")))
				typeRow = 1;
			else
				typeRow = -1;
			String format = getParameter("dateTargetFormat", "");
			if(!format.isEmpty()){
				try{
					sdf = new SimpleDateFormat(format);
				}catch(IllegalArgumentException ie){
					throw new InitializationException("Invalid date format " + format);
				}
			}
			
			String set = getParameter("rangeColumnSubset", "");
			ArrayList<Integer> inclusionSet = new ArrayList<Integer>();
			if(!set.isEmpty()){
				String[] ss = set.split(",");
				for(String s:ss){
					try {
						inclusionSet.add(Integer.parseInt(s));
					} catch (java.lang.NumberFormatException nfe)  { 
						throw new InitializationException("Column index has to be numeric: "+set);
					}
				}
				Collections.sort(inclusionSet);
			}
			range = new Range(getQuerySource());	
			range.setInclusionSet(inclusionSet);			
			
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}


}
