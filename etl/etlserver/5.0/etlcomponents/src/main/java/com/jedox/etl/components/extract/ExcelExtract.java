package com.jedox.etl.components.extract;

import java.util.Date;

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
import com.jedox.etl.core.source.processor.Processor;

public class ExcelExtract extends TableSource implements IExtract {
	
	private class ExcelProcessor extends Processor {
		
		private Sheet sheet;
		private CellReference start;
		private CellReference end;
		private FormulaEvaluator evaluator;
		private com.jedox.etl.core.node.Row processorRow = new com.jedox.etl.core.node.Row();
		private int currentRow;
		
		public ExcelProcessor(Workbook workbook, String query) throws RuntimeException {
			String[] def = query.split("\\.(?=([^\"]*\"[^\"]*\")*[^\"]*$)");
			String area;
			String sheetName = def[0].replace("\"", "");
			if (def.length == 2) {
				sheet = workbook.getSheet(sheetName);
				area = def[1];
			} else {
				sheet = workbook.getSheetAt(0);
				area = def[0];
			}
			if (sheet == null) {
				throw new RuntimeException("Excel workbook does not contain a sheet named "+sheetName+".");
			}
			String[] cells = area.split(":");
			if (cells.length != 2) {
				throw new RuntimeException("Cell area needs to be in a form like $A$1:$C$3");
			}
			start = new CellReference(cells[0]);
			end = new CellReference(cells[1]);
			evaluator = workbook.getCreationHelper().createFormulaEvaluator();
			currentRow = start.getRow();
			if (sheet.getLastRowNum() < start.getRow()) {
				throw new RuntimeException("Starting row "+(start.getRow()+1)+" exeeds available rows: "+(sheet.getLastRowNum()+1));
			}
			if (sheet.getLastRowNum() < end.getRow()) {
				log.warn("End row "+(end.getRow()+1)+" exeeds available rows: "+(sheet.getLastRowNum()+1));
			}
			for (int i=start.getCol(); i<=end.getCol(); i++) {
				CellReference r = new CellReference(start.getRow(),i);
				Row row = sheet.getRow(start.getRow());
				if(row==null && header){
					throw new RuntimeException("The header row can not be empty.");
				}
				Cell cell = null;
				if(row!=null)
					cell = row.getCell(i);
				
				if(cell==null  && header){
					throw new RuntimeException("Cells in the header can not be empty.");
				}
				Object cellValue = getCellValue(cell);
				Row typeRowObj = null;
				if(typeRow>0){
					int typeRowIndex = start.getRow()+typeRow;
					if(!header){
						typeRowIndex--;
					}
					
					typeRowObj = sheet.getRow(typeRowIndex);
		 
					if(typeRowObj== null || typeRowObj.getCell(i)== null){
						log.warn("Type row at position " + r.getCellRefParts()[2] + "$" +(typeRowIndex+1) + " is empty and therefor can not be used as a type cell, String is used instead.");
					}
				}
				
				Class<?> valueType = String.class;
				if(typeRowObj!= null && typeRowObj.getCell(i)!= null){
					valueType = getValueType(typeRowObj.getCell(i));
				}
				String name = (header && cellValue != null) ? cellValue.toString() : r.getCellRefParts()[2];
				if(processorRow.getColumn(name)!=null){
					throw new RuntimeException("The header can not have 2 columns with the same name.");
				}
				Column c = new Column(name);
				c.setValueType(valueType.getCanonicalName());
				processorRow.addColumn(c);
			}
			if (header) currentRow++;
		}
		
		protected Object getCellValue(Cell cell) {
			if (cell == null) return null;
			switch (cell.getCellType()) {
	            case Cell.CELL_TYPE_STRING: return cell.getRichStringCellValue().getString();
	            case Cell.CELL_TYPE_NUMERIC:
	                if (DateUtil.isCellDateFormatted(cell)) {
	                	return cell.getDateCellValue();
	                } else {
	                	return cell.getNumericCellValue();
	                }
	            case Cell.CELL_TYPE_BOOLEAN: return cell.getBooleanCellValue();
	            case Cell.CELL_TYPE_FORMULA: {
	            	CellValue cellValue = evaluator.evaluate(cell);
	            	switch (cellValue.getCellType()) {
	            	 	case Cell.CELL_TYPE_STRING: return cellValue.getStringValue();
		                case Cell.CELL_TYPE_NUMERIC: return cellValue.getNumberValue();
		                case Cell.CELL_TYPE_BOOLEAN: return cell.getBooleanCellValue();
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
	                	return Date.class;
	                } else {
	                	return Double.class;
	                }
	            case Cell.CELL_TYPE_BOOLEAN: return Boolean.class;
	            case Cell.CELL_TYPE_FORMULA: {
	            	CellValue cellValue = evaluator.evaluate(cell);
	            	switch (cellValue.getCellType()) {
	            	 	case Cell.CELL_TYPE_STRING: return String.class;
		                case Cell.CELL_TYPE_NUMERIC: return Double.class;
		                case Cell.CELL_TYPE_BOOLEAN: return Boolean.class;
		                default: String.class.getCanonicalName();
	            	}
	            }
	            default: return String.class;
	        }
		}

		@Override
		protected boolean fillRow(com.jedox.etl.core.node.Row row) throws Exception {
			if (currentRow > sheet.getLastRowNum() || currentRow > end.getRow()) return false;
			Row sheetRow = sheet.getRow(currentRow);
			if (sheetRow != null) {
				for (int j=start.getCol(); j<Math.min(end.getCol()+1,sheetRow.getLastCellNum()); j++) {
					Cell cell = sheetRow.getCell(j);
					IColumn column = row.getColumn(j-start.getCol());
					column.setValue(getCellValue(cell));
		        }
				for (int i=sheetRow.getLastCellNum(); i<=end.getCol(); i++) { //set all columns greater currently available cells to null
					IColumn column = processorRow.getColumn(i-start.getCol());
                	column.setValue(null);
				}
			} else { //set all columns to null if sheetRow is null
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
		
	}
	
	
	private static final Log log = LogFactory.getLog(ExcelExtract.class);
	private boolean header;
	private int typeRow;
	
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
		IProcessor processor = new ExcelProcessor(getConnection().open(), getQuerySource());
		processor.current().setAliases(getAliasMap());
		processor.setLastRow(size);
		processor.setName(getName());
		return processor;
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			header = getParameter("header","false").equalsIgnoreCase("true");
			typeRow = Integer.parseInt(getParameter("typeRow","1"));
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}


}
