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
package com.jedox.etl.components.transform;

import com.jedox.etl.components.config.transform.TableViewConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.source.processor.UnionProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;

public class TableView extends TableSource implements ITransform {
	
	private class FilterProcessor extends Processor {
		
		private RowFilter filter;
		
		public FilterProcessor(IProcessor sourceProcessor, RowFilter filter) throws RuntimeException {
			setSourceProcessor(sourceProcessor);
			this.filter = filter;
		}
		
		protected boolean evaluateFilters() throws RuntimeException {
			if (filter != null) {
				if (!filter.evaluate(getRow())) return false;
			}
			return true;
		}
		
		public void close() {
			super.close();
		}

		@Override
		protected boolean fillRow(Row row) throws Exception {
			do { 
				Row r = getSourceProcessor().next();
				if (r == null) return false;
			} while (!evaluateFilters());
			return true;
		}

		@Override
		protected Row getRow() throws RuntimeException {
			return getSourceProcessor().current();
		}

		@Override
		protected void init() throws RuntimeException {
			this.filter.init(getSourceProcessor().current());
		}
		
	}
	
	protected RowFilter getFilter() throws RuntimeException {
		try {
			return getConfigurator().getFilter(); //get fresh filter instance from XML
		}
		catch (ConfigurationException e) {
			throw new RuntimeException(e);
		}
	}
	
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(UnionProcessor.getInstance(getSourceManager().getProcessors()),Facets.INPUT);
		RowFilter filter = getFilter();
		if (filter != null) processor = initProcessor(new FilterProcessor(processor,filter),Facets.OUTPUT);
		if (!isCached()) {//cschw: if cached lines will be limited by internal query
			processor.setFirstRow(getFirstRow());
			size = (size == 0) ? getLastRow(): size;
			processor.setLastRow(Math.min(getLastRow(),getFirstRow()+size));
		}
		return processor;
	}

	public TableView() {
		setConfigurator(new TableViewConfigurator());
	}

	public TableViewConfigurator getConfigurator() {
		return (TableViewConfigurator)super.getConfigurator();
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}
	
	protected int getFirstRow() throws RuntimeException {
		try {
			int startLine =  Math.max(Integer.parseInt(getParameter("start","1"))-1,0);
			return startLine;
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to parse number of start line in Transform "+ getName() + ": "+e.getMessage());
		}
	}


	protected int getLastRow() throws RuntimeException {
		try {
			int lastLine = Integer.parseInt(getParameter("end","0"));
			if(lastLine<getFirstRow() && lastLine != 0){
				throw new RuntimeException("In Transform " + getName() + " end line number can be either 0 or smaller than start line.");
			}
			else{
				return (lastLine == 0) ? Integer.MAX_VALUE: lastLine;
			}
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to parse number of end line: "+e.getMessage());
		}
	}

	protected void postInit() throws InitializationException {
		super.postInit();
		try {
			String sorter = getConfigurator().getSorter();
			if (!sorter.isEmpty()) { 	//have to build a view on top of a source
				//ensure internal storage.
				if (!isCached()) setCaching(getDefaultCacheType());
				//set view query on internal storage
				String query = "select * from "+getLocator().getPersistentName()+sorter;
				query += (getLastRow() != Integer.MAX_VALUE) ? " LIMIT "+(getLastRow()-getFirstRow()) : " LIMIT -1";
				if (getFirstRow() != 0) query += " OFFSET "+getFirstRow();
				setQueryInternal(query);
			}
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
		
	}

	public void init() throws InitializationException {
		super.init();
		try {
			SourceManager manager = new SourceManager();
			manager.addAll(getConfigurator().getSources());
			addManager(manager);
			getConfigurator().getFilter(); //just test if filter is OK. We need fresh instances per processor, because optimization needs to set row / column info
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}	
	}

}
