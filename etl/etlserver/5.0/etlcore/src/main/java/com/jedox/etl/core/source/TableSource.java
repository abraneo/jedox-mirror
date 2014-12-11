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
package com.jedox.etl.core.source;

import java.io.StringWriter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.source.TableSourceConfigurator;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.persistence.IStore;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.CSVWriter;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.SQLUtil;
import com.jedox.etl.core.load.ILoad.Modes;

/**
 * Abstract Base Class for DataSources. Implements base caching and execution logic. All concrete sources should inherit from this class.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class TableSource extends Component implements ISource, IExecutable {

	private IAliasMap aliasMap;
	private static final Log log = LogFactory.getLog(TableSource.class);
	private String queryInternal;
	protected boolean notifyRetrieval = true;
	private boolean isCached = false;
	private IConnection connection;
	private Views format;
//	private FetchModes fetchMode;
	private String query;
	private ExecutionState state;
	private IStore tempStore;
    // Maximum lines for Data Preview. toDo: constant in config
	private static final int maxLinesPreview = 50000;

	public TableSourceConfigurator getConfigurator() {
		return (TableSourceConfigurator)super.getConfigurator();
	}


	protected String escapeName(String name) {
		try {
		return SQLUtil.quoteName(name,getInternalConnection().getIdentifierQuote());
		}
		catch (RuntimeException e) { //fallback to standard quote.
			return NamingUtil.escape(name);
		}
	}

	public Views getFormat() {
		return format;
	}

//	protected FetchModes getFetchMode() {
//		return fetchMode;
//	}

	protected String getQueryInternal() {
		if (queryInternal == null) return "select * from "+getLocator().getPersistentName();
		return queryInternal;
	}

	protected int getSampleSize() throws RuntimeException {
		try {
			return Integer.parseInt(getParameter("sample","0"));
		}
		catch (Exception e) {
			throw new RuntimeException("Parameter sample of source "+getName()+" needs to be numeric.");
		}
	}

	protected void setQueryInternal(String queryInternal) {
		this.queryInternal = queryInternal;
	}

	protected void setQuerySource(String query) {
		this.query = query;
	}

	protected String getQuerySource() {
		return query;
	}

	public IAliasMap getAliasMap() {
		return aliasMap;
	}

	public void setAliasMap(IAliasMap map) {
		if (map != null) aliasMap = map;
	}

	/**
	 * gets a processor for the source data of this source.
	 * @param size the size of the data sample
	 * @return the processor for the source data.
	 * @throws RuntimeException
	 */
	protected abstract IProcessor getSourceProcessor(int size) throws RuntimeException;

	/**
	 * gets a processor on the cached data of this source
	 * @param query the query to execute
	 * @param size the sample size
	 * @return the processor for the cached data.
	 * @throws RuntimeException
	 */
	protected IProcessor getCachedProcessor(String query, int size) throws RuntimeException {
		if (isExecutable()) {
			log.debug(getName()+": Starting querying cached data via query: "+query);
			IProcessor result = getInternalConnection().getProcessor(query, false, size);
			//reset alias map properties such as default value to processor
			result.setName(getName());
			result.current().setAliases(getAliasMap());
			log.debug(getName()+": Finished querying cached data.");
			return result;
		}
		throw new RuntimeException("Execution of source "+getName()+" was aborted due to errors.");
	}
	
	/**
	 * gets a processor on the cached data using the internal query definition
	 * @param size the data sample size
	 * @return the processor for the cached data
	 * @throws RuntimeException
	 */
	protected IProcessor getCachedProcessor(int size) throws RuntimeException {
		return getCachedProcessor(getQueryInternal(),size);
	}

	/**
	 * caches the data of a given processor in the internal cache
	 * @param processor the processor of the data to cache
	 * @throws RuntimeException
	 */
	protected void cache(IProcessor processor) throws RuntimeException {
		if (processor != null) {
			tempStore = processor.addPersistor(new PersistorDefinition(getLocator(),getInternalConnection()));
			processor.run();
		}
		else throw new RuntimeException("Internal processor to cache is null.");
	}
	
	protected boolean isPersisted() {
		if  (tempStore == null) {
			try {
				String query = "select * from "+ getLocator().getPersistentName(getInternalConnection().getIdentifierQuote());
				getInternalConnection().getProcessor(query, null, true, false, 1);
				log.info("Reusing existing persistence for source "+getName());
				return true;
			}
			catch (Exception e) {
				return false;
			}
		}
		else return true;
	}

	public boolean isCached() {
		return isCached;
	}
	
	public void setCaching(boolean isCached) {
		this.isCached = isCached;
	}

	protected void cache() throws RuntimeException {
		setCaching(true);
		IProcessor processor = getSourceProcessor(0);
		if (!isPersisted())
			cache(processor);
	}

	
	public IProcessor getProcessor(int size) throws RuntimeException {
		if (isExecutable()) {
			if (notifyRetrieval)
				log.info("Data retrieval from "+getLocator().getType()+" "+getName());
			else
				log.debug("Data retrieval from "+getLocator().getType()+" "+getName());
			IProcessor processor = null;
			if (isCached()) {
				if (!isPersisted()) {
					cache();
				}
				processor = getCachedProcessor(size);
			}
			else {
				if (isPersisted()) {
					processor = getCachedProcessor(size);
				}
				else {
					processor = getSourceProcessor(size);
				}
			}
			log.debug("Finishing data retrieval from "+getLocator().getType()+" "+getName());
			processor.setEncoding(getEncoding(), null);
			processor.setState(getState());
			processor.setName(getName());
			return processor;
		}
		return null;
	}

	public IProcessor getProcessor() throws RuntimeException {
		return getProcessor(getSampleSize());
	}

	public IConnection getConnection() throws RuntimeException {
		return connection;
	}

	public boolean hasConnection() {
		return connection != null;
	}

	protected String getEncoding() {
		if (hasConnection())
			try {
				return getConnection().getEncoding();
			} catch (RuntimeException e) {}
		return null;
	}

	public void setState(ExecutionState state) {
		this.state = state;
	}

	public ExecutionState getState() {
		return state;
	}

	public boolean isExecutable() {
		return (state == null) || state.isExecutable();
	}

	public void execute() {
		log.info("Starting source execution: "+getName());
		if (getState() != null) {
			try {
				IProcessor processor = getProcessor();
				getState().setMetadata(processor.current());
				switch (getState().getDataTarget()) {
				case PERSISTENCE: {//create temporary persistent data store, which may be queried later on
					Locator loc = new Locator().add(NamingUtil.internal(getContextName())).add(getState().getType()).add(NamingUtil.internal(getName()));
					loc.setContext(getContextName());
					loc.setPersistentSchema(getContextName());
					PersistorDefinition definition = new PersistorDefinition();
					definition.setConnection(getInternalConnection());
					definition.setLocator(loc);
					definition.setMode(Modes.FILL);
					processor.addPersistor(definition);
					getState().setData(loc.toString());
					log.info("Caching Source "+getName()+" to internal persistence");						
					processor.run();
					break;
				}
				case STDOUT: {//simply write to stdout. Useful only for standalone use (non server)
					CSVWriter writer = new CSVWriter(System.out);
					writer.write(processor);
					break;
				}
				default: {//write inline to state. Be careful for data size
					StringWriter buffer = new StringWriter();
					CSVWriter writer = new CSVWriter(buffer);
					//limit maximum number of lines in inline case to avoid heap space problems
					if (getSampleSize() <= 0 || getSampleSize() > maxLinesPreview) {
						log.info("Big data sample requested. Limiting data to "+String.valueOf(maxLinesPreview)+" lines....");
						processor.setLastRow(processor.getFirstRow()+maxLinesPreview);
					}
					writer.write(processor);
					getState().setData(buffer.toString());
				}
				}
				log.info("Finished retrieving of source "+getName()+".");
			}
			catch (Exception e) {
				log.error("Failed to retrieve data from source "+getName() + " : " +e.getMessage());
			}
		}
	}

	public void test() throws RuntimeException {
		super.test();
		IProcessor processor = getProcessor(10);
		processor.setLastRow(10);
		//processor.run();
		StringWriter buffer = new StringWriter();
		CSVWriter writer = new CSVWriter(buffer);
		writer.write(processor);
		if (getState() != null) getState().setData(buffer.toString());
	}

	public void invalidate() {
		state = null;
		try {
			if (tempStore != null) {
				tempStore.close();
			}	
		}
		catch (RuntimeException e) {
			log.warn("Error removing persistent structures: "+e.getMessage());
		}
	}

	public Row getOutputDescription() throws RuntimeException {
		if (queryInternal == null) { //no change in layout
			IProcessor p = getSourceProcessor(1);
			return p.getOutputDescription();
		}
		return getProcessor(1).getOutputDescription();
	}
	
	/*
	public String getPersistentName() {
		if (queryInternal == null) return getLocator().getPersistentName();
		return "("+queryInternal+")";
	}
	*/

	public void init() throws InitializationException {
		try {
			super.init();
			getLocator().setQuote(ConfigManager.getInstance().getInternalConnection().getIdentifierQuote());
			setAliasMap(getConfigurator().getAliasMap());
			connection = getConfigurator().getConnection();
			if (connection != null) {
				ConnectionManager manager = new ConnectionManager();
				manager.add(connection);
				addManager(manager);
			}
			format = getConfigurator().getFormat();
//			fetchMode = getConfigurator().getFetchMode();
			query = getConfigurator().getQuery();
			getSampleSize();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}



}