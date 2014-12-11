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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source.processor;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.DatastoreManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.etl.core.util.Recoder;
import com.jedox.etl.core.persistence.PersistorDefinition;

public abstract class Processor implements IProcessor {
	
	private ArrayList<RowFilter> filters = new ArrayList<RowFilter>();
	private ArrayList<Datastore> persistences = new ArrayList<Datastore>();
	private int start = 0;
	private int end = Integer.MAX_VALUE;
	private int rowsAccepted = 0;
	private static final Log log = LogFactory.getLog(Processor.class);
	private String name = "";
	private boolean info = false;
	private String displayType = "processor"; 
	private String[][] result = null;
	private Recoder recoder = new Recoder();
	private ExecutionState state = null;
	private boolean stopped = false;
	private Column rowCountColumn;
	private IProcessor sourceProcessor;
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getName() {
		return name;
	}
	
	public void setState(ExecutionState state) {
		this.state = state;
	}
	
	protected ExecutionState getState() {
		return state;
	}
	
	public void addFilter(RowFilter filter) {
		if (filter != null)
			filters.add(filter);
	}
	
	public void removeFilters() {
		filters.clear();
	}
	
	public Datastore addPersistor(PersistorDefinition definition) throws RuntimeException {
		definition.setInput(current());
		Datastore persistor = DatastoreManager.getInstance().provide(definition,true);
		if (persistor != null && definition.getDirectSource()==null) {
			persistences.add(persistor);
		}
		return persistor;
	}
	
	public void removePersistor(PersistorDefinition definition) throws RuntimeException {
		Datastore persistor = DatastoreManager.getInstance().get(definition.getLocator().toString());
		if (persistor != null) {
			DatastoreManager.getInstance().remove(persistor);
		}
	}
	
	public String getOrigin() {
		return (sourceProcessor == null) ? name : sourceProcessor.getOrigin();
	}
	
	public IProcessor getSourceProcessor() {
		return sourceProcessor;
	}
	
	public List<IProcessor> getProcessorChain() {
		LinkedList<IProcessor> result = new LinkedList<IProcessor>(); 
		IProcessor p = sourceProcessor;
		while (p != null) {
			result.add(0, p);
			p = p.getSourceProcessor();
		}
		//add self at last position
		result.add(this);
		return result;
	}
	
	protected void setSourceProcessor(IProcessor sourceProcessor) {
		this.sourceProcessor = sourceProcessor;
	}
	
	protected boolean isExecutable() {
		return (!stopped && (state != null) ? state.isExecutable() : !stopped);
	}

	public void setInfo(boolean info, String displayType) {
		this.info = info;
		this.displayType = displayType;
	}
	
	public String[][] getResultMatrix() throws RuntimeException {
		//see if the result is cached.
		if (result == null) {
			Row row = current();
			ArrayList<ArrayList<String>> columns = new ArrayList<ArrayList<String>>();
			for (int i=0; i<row.size(); i++)
				columns.add(new ArrayList<String>());
			row = next();
			while (row != null) {
				for (int i=0; i<row.size(); i++) {
					IColumn c = row.getColumn(i);
					columns.get(i).add(c.getValueAsString());
				}
				row = next();
			}
			result = new String[columns.size()][];
			for (int i=0; i<columns.size(); i++) {
				result[i] = columns.get(i).toArray(new String[columns.get(i).size()]);
			}
		}
		return result;
	}

	protected abstract boolean fillRow(Row row) throws Exception;
	
	protected abstract Row getRow();
	
	protected boolean evaluateFilters() {
		for (RowFilter filter : filters) {
			if (!filter.evaluate(getRow())) return false;
		}
		return true;
	}
	
	private boolean hasNext() throws Exception {
		boolean hasData;
		do { //loop until filter matches or no more data
			hasData = (rowsAccepted < end) && fillRow(getRow());
		} while ((hasData) && !evaluateFilters());
		return hasData;
	}

	protected void persist(Row row) throws RuntimeException {
		for (Datastore def : persistences) {
			def.write();
		}
	}
	
	protected void commit() throws RuntimeException {
		for (Datastore def : persistences) {
			log.debug("Committing datastore "+def.getLocator());
			def.commit();
		}
	}
	
	public Row current() {
		if (recoder.isRecoding())
			return getRow().recode(recoder);
		else return getRow();
	}
	
	public Row next() throws RuntimeException {
		try {
			if (isExecutable()) {
				if (hasNext()) {
					rowsAccepted++;
					if (rowCountColumn != null) {
						rowCountColumn.setValue(rowsAccepted);
					}
					Row current = current();
					persist(current);
					return current; //return row if there is one 
				}
				else {
					//else we are finished
					String infotext = "Lines read from "+displayType+" "+name+": "+rowsAccepted;
					if (info)
						log.info(infotext);
					else
						log.debug(infotext);
					close();
					commit();
				}
			}
			return null; 
		}
		catch (Exception e) {
			if (!stopped)
				commit();
			log.debug("Can't get next row in source "+getName()+": "+e.getMessage());
			throw new RuntimeException(e.getMessage());
		}
	}

	public void setFirstRow(int start) {
		start = Math.max(start,0);
		this.start = start;
		boolean hasData = true;
		try {
			while ((rowsAccepted < start) && hasData) {
				hasData = hasNext();
				if (hasData) 
					rowsAccepted++;
				else close();
			} 
		}
		catch (Exception e) {
			log.error("Can't get next row in source "+getName()+": "+e.getMessage());
			log.debug("",e);
		}
		
	}
	
	public void setLastRow(int end) {
		if (end <= 0) end = Integer.MAX_VALUE; 
		this.end = end;
	}
	
	public int getFirstRow() {
		return start;
	}
	
	public int getLastRow() {
		return end;
	}
	
	public void setEncoding(String sourceEncoding, String targetEncoding) {
		recoder.setRecoding(sourceEncoding, targetEncoding);
	}
	
	public void run() throws RuntimeException {
		Row row = current();
		while (row != null) 
			row = next();
	}
	
	public void close() {
		//indicate that we are finished
		stopped = true;
		for (IProcessor p : getProcessorChain()) {
			if (!p.isClosed()) p.close();
		}
	}
	
	public boolean isClosed() {
		return stopped == true;
	}
	
	public Views getFormat() {
		return Views.NONE;
	}
	
	protected int getRowsAccepted() {
		return rowsAccepted;
	}
	
	public IColumn getRowCountColumn() {
		if (rowCountColumn == null) {
			rowCountColumn = new Column("#_#RowCount#_#");
			rowCountColumn.setValueType(Integer.class.getCanonicalName());
			rowCountColumn.setValue(rowsAccepted);
			rowCountColumn.setDefaultValue("0");
		}
		return rowCountColumn;
	}
	
	public Row getOutputDescription() throws RuntimeException {
		return getRow().clone();
	}
	
/*	
	public void handleTrigger(Trigger trigger) {
		switch (trigger.getAction()) {
		case Stop : stopped = true; break;
		default: 
		}
	}
*/
}
