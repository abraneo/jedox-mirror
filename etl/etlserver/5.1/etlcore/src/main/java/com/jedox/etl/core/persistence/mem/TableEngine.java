package com.jedox.etl.core.persistence.mem;


import java.util.HashMap;
import java.util.Map;

import org.h2.command.ddl.CreateTableData;
import org.h2.table.TableBase;

import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 *
 * @author chris
 */
public class TableEngine implements org.h2.api.TableEngine {

    private static Map<String,IProcessor> tableProcessor = new HashMap<String,IProcessor>();
    private static Map<String,Table> tables = new HashMap<String,Table>();

    public static void addData(String tableName, IProcessor processor) {
    	tableProcessor.put(tableName, processor);
    }

    @Override
    public TableBase createTable(CreateTableData data) {
    	Locator locator = new Locator();
    	locator.setPersistentSchema(data.schema.getName().toUpperCase());
    	locator.setPersistentTable(data.tableName.toUpperCase());
    	Table table = new Table(data,tableProcessor.remove(locator.getPersistentName()));
    	tables.put(locator.getPersistentName(), table);
        return table;
    }
    
    public static void removeTable(String name) {
    	tables.remove(name);
    }
    
    public static Table getTable(String name) {
    	return tables.get(name);
    }

}
