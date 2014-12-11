package com.jedox.etl.components.connection;

import java.sql.SQLException;
import java.sql.Statement;
import com.jedox.etl.core.component.RuntimeException;

public class PostgresqlConnection extends RelationalConnection {

	
	protected Statement createBufferedStatementInternal() throws SQLException, RuntimeException {
		return open().createStatement(java.sql.ResultSet.TYPE_FORWARD_ONLY,java.sql.ResultSet.CONCUR_READ_ONLY,java.sql.ResultSet.HOLD_CURSORS_OVER_COMMIT);
	}

}
