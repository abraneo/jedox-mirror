package com.jedox.etl.core.persistence.mem;

import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;

import org.h2.engine.Session;
import org.h2.index.BaseIndex;
import org.h2.index.Cursor;
import org.h2.index.IndexType;
import org.h2.message.DbException;
import org.h2.value.DataType;
import org.h2.result.Row;
import org.h2.result.SearchRow;
import org.h2.result.SortOrder;
import org.h2.table.IndexColumn;
import org.h2.table.TableFilter;
import org.h2.value.Value;

/**
 *
 * @author chris
 */
public class Index extends BaseIndex {

    private class CustomFunctionCursor implements Cursor {

        private final Session session;
        private final ResultSet result;
        private final ResultSetMetaData meta;
        private Value[] values;
        private Row row;

        CustomFunctionCursor(Session session, ResultSet result) {
            this.session = session;
            this.result = result;
            try {
                this.meta = result.getMetaData();
            } catch (SQLException e) {
                throw DbException.convert(e);
            }
        }

        @Override
        public Row get() {
            if (values == null) {
                return null;
            }
            if (row == null) {
                row = new Row(values, 1);
            }
            return row;
        }

        @Override
        public SearchRow getSearchRow() {
            return get();
        }

        @Override
        public boolean next() {
            row = null;
            try {
                if (result != null && result.next()) {
                    int columnCount = meta.getColumnCount();
                    values = new Value[columnCount];
                    for (int i = 0; i < columnCount; i++) {
                        int type = DataType.convertSQLTypeToValueType(meta.getColumnType(i + 1));
                        values[i] = DataType.readValue(session, result, i+1, type);
                    }
                } else {
                    values = null;
                }
            } catch (SQLException e) {
                throw DbException.convert(e);
            }
            return values != null;
        }

        @Override
        public boolean previous() {
            throw DbException.throwInternalError();
        }
    }

    private Table table;
    
    public Index(Table table, IndexColumn[] columns) {
        initBaseIndex(table, 0, null, columns, IndexType.createNonUnique(true));
        this.table = table;
    }

    @Override
    public void add(Session sn, Row row) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public boolean canGetFirstOrLast() {
        return false;
    }

    @Override
    public void close(Session sn) {
        //nothing to do
    }

    @Override
    public Cursor find(Session sn, SearchRow sr, SearchRow sr1) {
        return new CustomFunctionCursor(sn,table.getResultSet());
    }

    @Override
    public Cursor findFirstOrLast(Session sn, boolean bln) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public long getDiskSpaceUsed() {
        return 0;
    }

    @Override
    public long getRowCount(Session sn) {
        return table.getRowCount(sn);
    }

    @Override
    public long getRowCountApproximation() {
        return table.getRowCountApproximation();
    }

    @Override
    public boolean needRebuild() {
        return false;
    }

    @Override
    public void remove(Session sn, Row row) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void remove(Session sn) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void truncate(Session sn) {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public void checkRename() {
        throw DbException.getUnsupportedException("ALIAS");
    }

    @Override
    public String getPlanSQL() {
        return "function";
    }

    @Override
    public boolean canScan() {
        return false;
    }

	@Override
	public double getCost(Session session, int[] masks, TableFilter arg2,
			SortOrder arg3) {
		if (masks != null) {
            throw DbException.getUnsupportedException("ALIAS");
        }
		return table.getRowCountApproximation() * 10;
	}



}

