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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source.processor;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.DatastoreManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.persistence.PersistorDefinition;

public abstract class Processor implements IProcessor {
	
	private ArrayList<Datastore> persistences = new ArrayList<Datastore>();
	private int start = 0;
	private int end = Integer.MAX_VALUE;
	private int rowsAccepted = 0;
	private static final Log log = LogFactory.getLog(Processor.class);
	private int logBlockSize = 0;
	private boolean stopped = false;
	private Column rowCountColumn;
	private IProcessor sourceProcessor;
	private long processingTime = 0;
	private IComponent owner;
	private Facets facet;
	private boolean logInfo = true;
	private boolean initialized = false;
	
	public String getName() {
		return owner != null ? owner.getName() : "";
	}
	
	protected ExecutionState getState() {
		return (owner != null && owner.getContext()!= null) ? owner.getContext().getState() : null;
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
	
	public void setSourceProcessor(IProcessor sourceProcessor) {
		this.sourceProcessor = sourceProcessor;
	}
	
	protected boolean isExecutable() {
		return (!stopped && (getState() != null) ? getState().isExecutable() : !stopped);
	}

	protected abstract boolean fillRow(Row row) throws Exception;
	
	protected abstract Row getRow() throws RuntimeException;
	
	protected abstract void init() throws RuntimeException;
	
	public void initialize() throws RuntimeException {
		if (!initialized) {
			IProcessor sourceProcessor = getSourceProcessor();
			long start = System.nanoTime();
			init();
			long end = System.nanoTime();
			processingTime += end-start;
			//also add init time of source processor to own init time, when it has been already initialized outside of init phase of this processor.
			if (sourceProcessor!= null) processingTime += sourceProcessor.getOverallProcessingTime(); 
		}
		initialized = true;
	}
	
	public boolean isInitialized() {
		return initialized;
	}
	
	private boolean hasNext() throws Exception {
		return (rowsAccepted < end) && fillRow(getRow());
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
	
	public Row current() throws RuntimeException {
		if (!initialized) {
			throw new RuntimeException("Processor has not been initialized.");
		}
		return getRow();
	}
	
	protected String getFinishedText() {
		return "Lines read from "+getLogDisplayType()+" "+getName()+": "+rowsAccepted;
	}
	
	
	private void log(String text) {
		if ((getFacet().equals(Facets.INPUT) || getFacet().equals(Facets.OUTPUT)) && logInfo)
			log.info(text);
		else
			log.debug(text);
				
	}
	
	protected boolean logProgress() {
		//only apply for last processor in chain of owning component
		List<IProcessor> processors = owner.getContext().getProcessorsByOwner(owner);
		if (processors.size() > 0 && this.equals(processors.get(processors.size()-1))) {
			log.info(rowsAccepted+" rows processed from "+getLogDisplayType()+" "+getOwnerName()+".");
			return true;
		}
		return false;
	}
	
	public Row next() throws RuntimeException {
		if (!initialized) {
			throw new RuntimeException("Processor has not been initialized.");
		}
		if (logBlockSize != 0 && rowsAccepted == 0) {
			log("Retrieving data from "+getLogDisplayType()+" "+getName());
		}
		try {
			if (isExecutable()) {
				long start = System.nanoTime();
				if (hasNext()) {
					rowsAccepted++;
					if (rowCountColumn != null) {
						rowCountColumn.setValue(rowsAccepted);
					}
					if (logBlockSize != 0) {
						if (owner != null && (rowsAccepted % logBlockSize == 0 || rowsAccepted==1)) logProgress();
					}	
					
					Row current = current();
					persist(current);
					long end = System.nanoTime();
					processingTime += end-start;
					return current; //return row if there is one 
				}
				else {
					//else we are finished
					boolean logged = false;
					if (logBlockSize != 0 && owner != null) {
						logged = logProgress();
					}
					if (!logged) log(getFinishedText());
					close();
					commit();
					long end = System.nanoTime();
					processingTime += end-start;
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
	
	public int getRowsAccepted() {
		return rowsAccepted;
	}
	
	public long getOwnProcessingTime() {
		return (sourceProcessor != null) ? getOverallProcessingTime() - sourceProcessor.getOverallProcessingTime() : getOverallProcessingTime();
	}
	
	public long getOverallProcessingTime() {
		return processingTime;
	}
	
	protected void addProcessingTime(long processingTime) {
		this.processingTime += processingTime;
	}
	
	public IColumn getRowCountColumn() {
		if (rowCountColumn == null) {
			rowCountColumn = new Column("#_#RowCount#_#");
			rowCountColumn.setValueType(Integer.class);
			rowCountColumn.setValue(rowsAccepted);
		}
		return rowCountColumn;
	}
	
	public Row getOutputDescription() throws RuntimeException {
		return getRow().clone();
	}
	
	public void setOwner(IComponent owner) {
		this.owner = owner;
	}
	
	public IComponent getOwner() {
		return owner;
	}
	
	public void setFacet(Facets facet) {
		this.facet = facet;
	}
	
	protected void setLogInfo(boolean logInfo) {
		this.logInfo = logInfo;
	}
	
	public Facets getFacet() {
		return facet;
	}
	
	public String getLogDisplayType() {
		return owner != null ? owner.getLocator().getManager().substring(0, owner.getLocator().getManager().length()-1) : "processor";
	}
	
	protected String getOwnerName() {
		return owner != null ? owner.getName() : getName();
	}
	
	public void setLogBlockSize(int logBlockSize) {
		this.logBlockSize = logBlockSize;
	}
	
	protected IProcessor initProcessor(IProcessor processor, Facets facet) throws RuntimeException {
		return getOwner().initProcessor(processor, facet);
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
