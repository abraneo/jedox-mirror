package com.jedox.etl.core.persistence.mem;

import com.jedox.etl.core.source.processor.IProcessor;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import org.h2.command.ddl.CreateTableData;
import org.h2.engine.Session;
import org.h2.index.IndexType;
import org.h2.message.DbException;
import org.h2.result.Row;
import org.h2.table.Column;
import org.h2.table.IndexColumn;
import org.h2.table.TableBase;
import org.h2.tools.SimpleResultSet;
import org.h2.value.DataType;

/**
 *
 * @author chris
 */
public class Table extends TableBase {

    private final long rowCount = 0;
    private CachedRowSource source;
    private IProcessor processor;

    public Table(CreateTableData data, IProcessor processor) {
        super(data);
        this.processor = processor;
        setColumns(data.columns.toArray(new Column[data.columns.size()]));
    }

    @Override
    public void lock(Session sn, boolean bln, boolean bln1) {
        //nothing to to
    }

    @Override
    public void close(Session sn) {
    	if (source != null) source.clear();
    }

    @Override
    public void unlock(Session sn) {
        //nothing to to
    }

    @Override
    public Index addIndex(Session sn, String string, int i, IndexColumn[] ics, IndexType it, boolean bln, String string1) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void removeRow(Session sn, Row row) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void truncate(Session sn) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void addRow(Session sn, Row row) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void checkSupportAlter() {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public String getTableType() {
        return null;
    }

    @Override
    public Index getScanIndex(Session sn) {
        return new Index(this, IndexColumn.wrap(columns));
    }

    @Override
    public Index getUniqueIndex() {
       return null;
    }

    @Override
    public ArrayList<org.h2.index.Index> getIndexes() {
         return null;
    }

    @Override
    public boolean isLockedExclusively() {
         return false;
    }

    @Override
    public long getMaxDataModificationId() {
        return Long.MAX_VALUE;
    }

    @Override
    public boolean isDeterministic() {
        return true;
    }

    @Override
    public boolean canGetRowCount() {
        return rowCount != Long.MAX_VALUE;
    }

    @Override
    public boolean canDrop() {
    	if (source != null) source.clear();
        //throw DbException.throwInternalError();
    	return true;
    }

    @Override
    public long getRowCount(Session sn) {
        return rowCount;
    }

    @Override
    public long getRowCountApproximation() {
        return rowCount;
    }

    @Override
    public long getDiskSpaceUsed() {
        return 0;
    }

    @Override
    public void checkRename() {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public boolean canReference() {
        return false;
    }

    public ResultSet getResultSet() {
      try {
         SimpleResultSet rs = new SimpleResultSet(getRowSource());
         for (Column c : getColumns()) {
             rs.addColumn(c.getName(), DataType.convertTypeToSQLType(c.getType()), (int)c.getPrecision(), c.getScale());
         }
         return rs;
      }
      catch (Exception e) {
    	  e.printStackTrace();
      }
      return null;
    }
    
    public CachedRowSource getRowSource() {
    	try {
		    if (source == null) {
		   	 	source = new CachedRowSource(new RowSource(processor));
		   	} else {
		   		source = source.newInstance();
		   	}
		}
    	catch (Exception e) {
   			e.printStackTrace();
   		}
    	return source;
    }

}
