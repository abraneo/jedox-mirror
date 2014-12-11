/**
*   @brief <Description of Class>
*
*   @file
*
*   Copyright (C) 2008-2014 Jedox AG
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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.components.config.extract.SalesforceExtractConfigurator;
import com.jedox.etl.components.connection.SalesforceConnection;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.*;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.sforce.soap.partner.DebugLevel;
import com.sforce.soap.partner.DescribeSObjectResult;
import com.sforce.soap.partner.PartnerConnection;
import com.sforce.soap.partner.QueryResult;
import com.sforce.soap.partner.sobject.SObject;
import com.sforce.ws.ConnectionException;
import com.sforce.ws.bind.XmlObject;


public class SalesforceExtract extends TableSource implements IExtract  {
	
	private static final Log log = LogFactory.getLog(SalesforceExtract.class);
	private String tablename = null;
	private String [] fields = null;
	private String soqlqueryCondition = null;
	private String whereClause = null;

	private class SalesforceProcessor extends Processor {

		private Row row;
		private QueryResult query;
		private int inBulkCount = 0;
		private int allCount = 0;
		private boolean hasNext = true;
		private PartnerConnection connection;
		
		/**
		 * read rules from the cube and set the columns names
		 * @throws RuntimeException
		 */
		public SalesforceProcessor() {
			
		}

		protected boolean fillRow(Row row) throws Exception {
			
			 while (hasNext) {
	                SObject[] records = query.getRecords();
	                if(records.length == 0)
	                	hasNext = false;
	                else if(inBulkCount < records.length) {
	                    Iterator<XmlObject> child = records[inBulkCount].getChildren();
	                    child.next();
	                    child.next();
	                    int j=0;
		                while(child.hasNext()){
		                	row.getColumn(j).setValue(child.next().getValue());
		                	j++;
		                }
	                	inBulkCount++;
	                	allCount++;
	                	
	                	if(records.length!=0 && inBulkCount == records.length) {
	                		 if (query.isDone()) {
	     	                	hasNext = false;
	     	                	inBulkCount = 0;
	     	                	log.info("Number of row read: " + allCount);
	     	                } else {
	     	                	query = connection.queryMore(query.getQueryLocator());
	     	                	inBulkCount = 0;
	     	                }
	                	}
	                	
		                return true;
	                }
	        
	            }
			 return false;
		}

		protected Row getRow() {
			return row;
		}
				
		@Override
		protected void init() throws RuntimeException {
			try {
				String buildQuery = null;
				connection = (PartnerConnection)getConnection().open();
				ArrayList<String> columnsList = new ArrayList<String>();
			
				StringBuilder fieldsStr = new StringBuilder();
					for(String field:fields){
						fieldsStr.append(field + ",");
						columnsList.add(field);
					}
					//no columns were given
					if(fieldsStr.length()==0){
						if(tablename==null){
							throw new InitializationException("At least query or table should be given.");
						}else{
							// get the fields from the connection
							DescribeSObjectResult dsor = connection.describeSObject(tablename);
							for(int i=0;i<dsor.getFields().length;i++){
								fieldsStr.append(dsor.getFields()[i].getName() + ",");
								columnsList.add(dsor.getFields()[i].getName());
							}
							fieldsStr.deleteCharAt(fieldsStr.length()-1);
						}
					}else{
						fieldsStr.deleteCharAt(fieldsStr.length()-1);
					}
					
					buildQuery = "select " + fieldsStr.toString() + " from " + tablename;
					
					if(!whereClause.isEmpty()){
						buildQuery += " " + whereClause;
					}
					
					if(soqlqueryCondition!=null && !soqlqueryCondition.isEmpty()){
						buildQuery += " " + soqlqueryCondition;
					}
				
		
				connection.setQueryOptions(2000);
				connection.setDebuggingHeader(DebugLevel.Db);
				log.debug(buildQuery);
				
				try {
					query = connection.query(buildQuery);
				} 
				catch (ConnectionException e2) {
					int indexFrom=e2.toString().indexOf("exceptionCode='");
					throw new RuntimeException("Error in Salesforce query: "+e2.toString().substring(indexFrom==-1?0:indexFrom));
				}
				
				row = PersistenceUtil.getColumnDefinition(getAliasMap(),columnsList.toArray(new String[0]));
			}
			catch (Exception e) {
				throw new RuntimeException(e.getMessage());
			}
		}
	}



	/**
	 * get the connection
	 */
	public SalesforceConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if (connection != null || !(connection instanceof SalesforceConnection))
			return (SalesforceConnection)connection;
		throw new RuntimeException("Salesforce connection is needed for this source: "+getName());
	}
	
	public SalesforceExtract() {
		setConfigurator( new SalesforceExtractConfigurator());
	}
	
	public SalesforceExtractConfigurator getConfigurator() {
		return (SalesforceExtractConfigurator)super.getConfigurator();
	}
	
	/**
	 * get a specific number of rows
	 */
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor result = initProcessor(new SalesforceProcessor(),Facets.OUTPUT);	
		result.setLastRow(size);
		return result;
	}	
	
	public Row getOutputDescription() throws RuntimeException {
		return PersistenceUtil.getColumnDefinition(getAliasMap(), fields);
	}		
	
	public void init() throws InitializationException {
		try {
			super.init();
			tablename = getConfigurator().getTableName();
			fields = getConfigurator().getColumns();
			whereClause = getConfigurator().getWhereClause();
			soqlqueryCondition = getConfigurator().getQuery();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
	
}
