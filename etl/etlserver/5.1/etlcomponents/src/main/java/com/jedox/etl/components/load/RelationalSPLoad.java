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
 *   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
 *   rights has proclos OG, Wien. Exclusive worldwide exploitation right
 *   (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.components.load;

import java.io.StringWriter;
import java.sql.CallableStatement;
import java.sql.SQLException;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.load.RelationalSPConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IFileConnection;
import com.jedox.etl.core.connection.IRelationalConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.source.processor.IProcessor;

public class RelationalSPLoad extends Load {

	private static final Log log = LogFactory.getLog(RelationalSPLoad.class);
	private String schemaName;
	private String spName;
	private Row row;
	private boolean needSource;
	private CallableStatement callStmt;

	public RelationalSPLoad() {
		setConfigurator(new RelationalSPConfigurator());
	}

	public RelationalSPConfigurator getConfigurator() {
		return (RelationalSPConfigurator)super.getConfigurator();
	}

	public IRelationalConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IRelationalConnection && !(connection instanceof IFileConnection)))
			return (IRelationalConnection) connection;
		throw new RuntimeException("Relational connection is needed for source "+getName()+".");
	}

	public void executeLoad() {

		log.info("Starting load "+getName()+" for stored procedure "+spName);

		try {
			checkProcedureName();
			if(callStmt==null){
				setCallStmt();
			}

			if(needSource){	
				IProcessor processor = getProcessor();
				Row inputRow = processor.next();
				StringBuilder buffer = new StringBuilder();
				while ((inputRow != null) && (isExecutable())) {	
					for(int i=0;i<row.getColumns().size();i++){
						CoordinateNode node =((CoordinateNode)row.getColumn(i));
						if(node.getValue()==null)
							node.setInput(inputRow.getColumn(node.getInputName()));
						String paramName = row.getColumn(i).getName();
						Object paramValue = row.getColumn(i).getValue();
						callStmt.setObject(paramName,paramValue);
						buffer.append(paramName + "=\"" + paramValue.toString() + "\",");
					}
					if(buffer.length()>0)
						buffer.deleteCharAt(buffer.length()-1);
					log.info("Running the procedure " + spName + " with these parameters:" + buffer.toString());
					buffer.delete(0, buffer.length());
					boolean isResultSet = callStmt.execute(); 
					if(!isResultSet)
						log.info("Number of rows effected is " + callStmt.getUpdateCount());
					inputRow = processor.next();
				}
			}else{
				for(IColumn col: row.getColumns()){
					callStmt.setObject(col.getName(), col.getValue());
				}
				callStmt.executeUpdate();
				
			}
			getConnection().open().commit();

		}
		catch (Exception e) {
			if(e.getMessage()!=null)
				log.error(e.getMessage().replaceAll("\n", ""));
		}
		log.info("Finished load "+getName());
		
	}

	/**
	 * @throws RuntimeException
	 */
	private void checkProcedureName() throws RuntimeException {
		StringWriter writer = getConnection().getProcedures(getConnection().getDatabase(), schemaName, spName );
		String[] str = writer.toString().split("\n");
		if(!(str.length==2 && !str[1].trim().isEmpty())){
			throw new RuntimeException("Procedure " + spName + " is not found in schema " + schemaName);
		}
		writer = getConnection().getProcedureParameters(getConnection().getDatabase(), schemaName, spName, null);
		str = writer.toString().split("\n");
	}

	/**
	 * @throws SQLException
	 * @throws RuntimeException 
	 */
	private void setCallStmt() throws SQLException, RuntimeException {
		String params = "";
		for(int i=0;i<row.getColumns().size();i++)
			params += "?,";

		if(!params.isEmpty())
			params = "(" + params.substring(0, params.length()-1) + ")";
		callStmt = getConnection().open().prepareCall("{ call " + spName + params + "}");
	}

	public void init() throws InitializationException {
		try {
			super.init();
			
			if(!getConnection().allowInRelationalSP()){
				throw new InitializationException("This connection type is not allowed in RelationalSP load");
			}
			
			spName = getConfigurator().getStoredProcedureName();
			schemaName = getConfigurator().getSchemaName();

			// check if database connection supports Schemas
			if (schemaName!=null && !getConnection().isSchemaSupported()) {
				throw new InitializationException("Schema cannot be set in load "+getName()+" for connection "+getConnection().getName()+". It has to be set in the connection.");
			}
			row = getConfigurator().getCoordinates();
			
			needSource = getConfigurator().needSource();
			boolean hasSource = getConfigurator().hasSource();
			if(needSource){
				if(!hasSource)
					throw new ConfigurationException("Load " + getName() + " needs a source since some input are dynamic.");
			}else{
				if(hasSource)
					throw new ConfigurationException("Load " + getName() + " does not need a source since all input are constant or no inputs are needed.");
			}

		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
