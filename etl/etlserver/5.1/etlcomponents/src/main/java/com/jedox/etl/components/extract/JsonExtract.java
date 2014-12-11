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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.extract;

import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jayway.jsonpath.JsonPath;
import com.jedox.etl.components.config.extract.JsonExtractConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IJsonConnection;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;



public class JsonExtract extends TableSource implements IExtract {

	private static final Log log = LogFactory.getLog(JsonExtract.class);
	private String level;
	private ArrayList<String> fields;

	private class JsonProcessor extends Processor {

		private Row row;
		private int count;
		private ArrayList<String> header;
		private ArrayList<ArrayList<String>> data;

		private Iterator<ArrayList<String>> dataIterator;

		public JsonProcessor(int size) throws RuntimeException {
			this.setLastRow(size);
		}

		/**
		 * read ALL the data and store it in the processor. All the data is stored once because when doing
		 * the first step (searching the level nodes), we retrieve all the elements that we need to read.
		 *
		 */
		@SuppressWarnings("unchecked")
		protected void init() throws RuntimeException{
			InputStreamReader isr = (InputStreamReader)getConnection().open();

			//the data in the json file
			data = new ArrayList<ArrayList<String>>();
				
			// get the file text in a string, JsonPath can not take inputStreamReader as a parameter
			// only inputStream can be given, but with inputStream no encoding can be specified
			StringBuffer buffer = new StringBuffer();
			int r = -1;
			try{				
				while((r=isr.read()) != -1) {
	            	buffer.append((char)r);
				}
			}  
			catch(Exception e){
				throw new RuntimeException("Error reading json file: "+e.getMessage());
			}

	        List<Object> levelElements;
	        try {
	           	levelElements = JsonPath.read(buffer.toString(), level);
	        } catch(Exception e){
    			throw new RuntimeException("Error reading json file on path "+level+": "+e.getMessage());
	        }

			//NodeList levelElements = (NodeList) xpath.evaluate(level, xmlDoc,XPathConstants.NODESET);

			//set the header
			ArrayList<String> inputRow = new ArrayList<String>();
			for(String field:fields)
				inputRow.add(field);
			header = (ArrayList<String>) inputRow.clone();

			// prepare the JPath queries
			/*JsonPath [] fieldExpressions = new JsonPath[fields.size()];
			for(int i=0;i<fieldExpressions.length;i++){
				JsonPath fieldExp= JsonPath.compile(fields.get(i), new Filter[0]);
				
				fieldExpressions[i] = fieldExp;
			}*/

			//set the data
			for(int i=0;i<levelElements.size();i++){
				inputRow.clear();
				for(int j=0;j<fields.size();j++){
					Object obj;
					try {
						obj = JsonPath.read(levelElements.get(i), fields.get(j));
		           	} catch(Exception e){
		           		// for this element field is not available in json file. Set it to empty
		           		inputRow.add("");
		           		continue;
		           	}
					inputRow.add(obj.toString());
				}
				data.add((ArrayList<String>)inputRow.clone());
				if(getLastRow()!= 0 && i>=getLastRow())
					break;
			}
			
			dataIterator = data.iterator();
			row = PersistenceUtil.getColumnDefinition(getAliasMap(),getColumns(header));
			row.setAliases(getAliasMap());
		}
		
		protected boolean fillRow(Row row) throws Exception {
			if (data != null && dataIterator.hasNext()) {
				count++;
				ArrayList<String> dataRow = (ArrayList<String>)dataIterator.next();
				if (dataRow != null) {
					try {
						for (int j=0; j<dataRow.size(); j++) {
							row.getColumn(j).setValue(dataRow.get(j));
						}
					}
					catch (Exception e) {
						log.error("Error in export cell: "+count+" "+e.getMessage());
					}
				}
				return true;
			}
			else { //finished ... do some cleanup
				dataIterator = null;
				data = null;
				count = 0;
				row = null;
				getConnection().close();
				return false;
			}
		}

		protected Row getRow() {
			return row;
		}

	}

	public JsonExtract() {
		setConfigurator(new JsonExtractConfigurator());
	}

	public JsonExtractConfigurator getConfigurator() {
		return (JsonExtractConfigurator) super.getConfigurator();
	}


	private String[] getColumns(ArrayList<String> header) {

		String [] names = new String[header.size()];
		for (int i=0; i<names.length; i++) {
			names[i] = header.get(i);
		}
		return names;
	}

	public IJsonConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IJsonConnection))
			return (IJsonConnection) connection;
		throw new RuntimeException("Json connection is needed for extract "+getName()+".");
	}


	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(new JsonProcessor(size),Facets.OUTPUT);
		return processor;
	}
	
	public Row getOutputDescription() throws RuntimeException {			
		Row row = PersistenceUtil.getColumnDefinition(fields);
		row.setAliases(getAliasMap());
		return row;
	}

	public void init() throws InitializationException {
		super.init();
		try {
			level = getConfigurator().getLevel();
			fields = getConfigurator().getFields();
		} catch (ConfigurationException e) {
			throw new InitializationException("Can not initialise extract "+getName()+":" + e.getMessage());
		}
	}

}
