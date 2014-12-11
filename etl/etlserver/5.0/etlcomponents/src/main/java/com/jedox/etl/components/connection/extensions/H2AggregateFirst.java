package com.jedox.etl.components.connection.extensions;

import java.sql.Connection;
import java.sql.SQLException;
import org.h2.api.AggregateFunction;

public class H2AggregateFirst implements AggregateFunction {
	
	private Object result = null;

	@Override
	public void add(Object object) throws SQLException {
		if (result == null) result = object;
	}

	@Override
	public Object getResult() throws SQLException {
		return result;
	}

	@Override
	public int getType(int[] types) throws SQLException {
		return (types.length > 0) ? types[0] : java.sql.Types.VARCHAR;
	}

	@Override
	public void init(Connection arg0) throws SQLException {
		// do nothing here
	}
	
}

