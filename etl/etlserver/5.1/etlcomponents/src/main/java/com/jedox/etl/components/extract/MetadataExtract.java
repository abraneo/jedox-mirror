/**
 *   @brief
 *
 *   @file
 *
 *   Copyright (C) 2008-2013 Jedox AG
 *
 *   @author Andreas Froehlich
 */
package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.extract.MetadataExtractConfigurator;
import com.jedox.etl.core.component.*;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.util.MetadataWriter;
import com.jedox.etl.core.util.PersistenceUtil;

public class MetadataExtract extends TableSource implements IExtract {

	private Properties properties;
	
	private static final Log log = LogFactory.getLog(MetadataExtract.class);
	
	
	public MetadataExtract() {
		setConfigurator(new MetadataExtractConfigurator());
	}

	public MetadataExtractConfigurator getConfigurator() {
		return (MetadataExtractConfigurator) super.getConfigurator();
	}
	
	private class MetadataProcessor extends Processor {
		private int maxrows;
		private int count;
		private Row row;
		private ArrayList<String> resultList = new ArrayList<String>();
		private int size;
		
		public MetadataProcessor(String result, int size) {
			this.size = size;
			for (String r : result.split("\n"))
				resultList.add(r.trim());// to remove \r 
		}
						
		protected boolean fillRow(Row row) throws Exception {
			if (count<=maxrows) {
				// String r = resultList.get(count);
				String[] cols = MetadataWriter.parseLine(resultList.get(count));

				for (int j=0; j<cols.length; j++) {
					if (j<row.size())
						row.getColumn(j).setValue(cols[j]);
					else
						log.warn("Invalid data in row "+count+" starting with "+row.getColumn(0).getValueAsString()+": Column index exceeds "+row.size());
				}
				count++;
				return true;				
			}
			else
				return false;
		}
		
		protected Row getRow() {
			return row;
		}

		@Override
		protected void init() throws RuntimeException {
			if (resultList.isEmpty())
				throw new RuntimeException("Empty result");
			
			String[] headerColumns = MetadataWriter.parseLine(resultList.get(0));
			
 			row = PersistenceUtil.getColumnDefinition(getAliasMap(),headerColumns);
			if(row.getColumn(0).getName().equalsIgnoreCase("id")){
				row.getColumn(0).setValueType(Integer.class);
			}
			maxrows = resultList.size()-1;
			if (size>0 && size<maxrows)
				maxrows=size;
			count = 1;
		}			
	}
		
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(new MetadataProcessor(getConnection().getMetadata(properties),size),Facets.OUTPUT);
		return processor;
		
	}
		
	public void init() throws InitializationException {
		try {
			super.init();
			properties = getConfigurator().getProperties();
			String selector=properties.getProperty("selector");			
			MetadataCriteria criteria=getConnection().getMetadataCriteria(selector);
			if (criteria==null)
				throw new InitializationException("Given selector "+selector+" is not valid");			
			criteria.checkFilters(properties);			
		}
		catch (Exception e) {throw new InitializationException(e);}
	}
}
