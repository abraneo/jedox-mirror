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

import java.util.List;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.component.RuntimeException;

/**
 * Interface for Processors.
 * A Processor is responsible for the processing the data provided by a {@link com.jedox.etl.core.source.ISource Source} or Pipeline.
 * Data is made accessible by a single {@link com.jedox.etl.core.node.Row} at every processing step.
 * A Processor provides the possibility of adding Filters to ignore Rows with data conforming to certain filter criteria.
 * A Processor also is able to convert the character encoding of the delivered data.   
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IProcessor {
	
	public static enum Facets {
		INPUT, HIDDEN, OUTPUT, CONNECTION
	}
	
	/**
	 * Gets the name of the processor, which by default should be the name of the underlying source.
	 * @return the name of the processor.
	 */
	public String getName();
	
	/**
	 * Sets the number of rows, which are silently dropped before delivering the first row in processing. Default is 0. Rows dropped by filter operations are not counted. 
	 * @param start the number of rows to drop before delivering the first processing result.
	 */
	public void setFirstRow(int start);
	/**
	 * Sets the last row in processing. When this row is delivered the processor terminates further processing. Rows dropped by filter operations are not counted. The number of rows deliver a calculated by {@link #setLastRow(int)} - {@link #setFirstRow(int)}
	 * @param end the number of the last row in processing.
	 */
	public void setLastRow(int end);
	/**
	 * gets the number of the first row in processing
	 * @return the number of the first row
	 */
	public int getFirstRow();
	/**
	 * gets the number of the last row in processing
	 * @return the number of the last row in processing
	 */
	public int getLastRow();
	/**
	 * Gets the current row. When the processor is initialized, this row does not contain any data before {@link #next()} is called. 
	 * @return the current row.
	 */
	public Row current() throws RuntimeException;
	/**
	 * Gets the next row of data from this processor.
	 * @return the current row or null if the processor does not provide any more rows.
	 */
	public Row next() throws RuntimeException;
	/**
	 * Adds a Persistor to this Processor. A Persistor persists all data delivered by this processor to an external Datastore.
	 * @param definition the definition of the external Datastore to persist to.
	 */
	public Datastore addPersistor(PersistorDefinition definition) throws RuntimeException;
	
	/**
	 * remove a Persistor to this Processor. This does not delete the table of the datastore but only the reference for it in the DatastoreManager.
	 * @param definition the definition of the external Datastore to persist to.
	 * @throws RuntimeException 
	 */
	public void removePersistor(PersistorDefinition definition) throws RuntimeException;
	/**
	 * Closes the processor and disposes all underlying resources 
	 */
	public void close();
	/**
	 * Runs the processor. This is useful when having set a Persistor via {@link #addPersistor(Datastore)} and there is no interest in the resulting data for further processing.
	 */
	public void run() throws RuntimeException;
	/**
	 * Gets the format description of this processors output. This is optional and has to supported by the underlying Source. 
	 * @return
	 */
	public Views getFormat();
	/**
	 * gets an IColumn holding the row-count of this processor in its value.
	 * @return
	 */
	public IColumn getRowCountColumn();
	/**
	 * gets the previous processor (holding the input for this processor) in the processor chain.   
	 * @return the source processor or null, if this processor is the first in a chain.
	 */
	public IProcessor getSourceProcessor();
	/**
	 * gets the processor chain of all previous processors delivering data to this processor.  
	 * @return the processor chain.
	 */
	public List<IProcessor> getProcessorChain();
	
	/**
	 * Gets the component output specification as Row
	 * @return the Row holding the output columns
	 */
	public Row getOutputDescription() throws RuntimeException;
	
	/**
	 * determines if this processor is finished and provides no more data.
	 * @return true if finished
	 */
	public boolean isClosed();
	
	/**
	 * gets the number of accepted / processed rows by this processor 
	 * @return
	 */
	public int getRowsAccepted();
	/**
	 * gets the time (ms) needed for processing 
	 * @return
	 */
	public long getOwnProcessingTime();
	
	public long getOverallProcessingTime();
	
	public void setOwner(IComponent owner);
	
	public IComponent getOwner();
	
	public void setFacet(Facets facet);
	
	public Facets getFacet();
	
	public String getLogDisplayType();
	
	public void setLogBlockSize(int logBlockSize);
	
	public void setSourceProcessor(IProcessor sourceProcessor);
	
	public boolean isInitialized();
	
	public void initialize() throws RuntimeException;

}
