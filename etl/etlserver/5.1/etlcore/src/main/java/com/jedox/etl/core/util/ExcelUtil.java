/**
 * 
 */
package com.jedox.etl.core.util;

import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

/**
 * @author khaddadin
 *
 */
public class ExcelUtil {
	
	
	private static final Log log = LogFactory.getLog(ExcelUtil.class);
	
	public static Object ConvertToColumnType(Object cellValue, IColumn column) {
		if(cellValue==null)
			return null;

		if(column.getValueType().equals(String.class)){
			return cellValue.toString();
		}
		if(column.getValueType().equals(Double.class)){
			try {
				return Double.valueOf(cellValue.toString());
			} catch (NumberFormatException e) {
				log.warn(cellValue  + " in column "+ column.getName()  +	" is not a double value although double is the column type.");
				return null;
			}
		}
		if(column.getValueType().equals(Date.class)){
			if(cellValue instanceof Date){
				return cellValue;
			}else{
				log.warn(cellValue  + " in column  "+ column.getName()  +	" is not a Date value although Date is the column type.");
				return null;
			}
		}
		log.warn("Cell type " + column.getValueType().getSimpleName() + " is not supported and will be mapped to String.");
		return cellValue.toString();
	}
	
	public static void logRowTypes(Row row){
		StringBuilder logMessage = new StringBuilder();
		logMessage.append("Column types: ");
		for(int i=0;i<row.getColumns().size();i++){
			logMessage.append(row.getColumn(i).getName() +"="+row.getColumn(i).getValueType().getSimpleName() + ",");
		}

		log.debug(logMessage.deleteCharAt(logMessage.length()-1));
	}

}
