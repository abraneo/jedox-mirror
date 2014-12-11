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
package com.jedox.etl.core.load;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.load.LoadConfigurator;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.RecodingProcessor;
import com.jedox.etl.core.util.PersistenceUtil;
import com.jedox.etl.core.util.Recoder;
import com.jedox.etl.core.node.RelationalNode.UpdateModes;
import com.jedox.etl.core.node.Row;

/**
 * Abstract Implementation class for {@link ILoad} Interface. All Loads should inherit from this class.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Load extends Component implements ILoad {

	private static final Log log = LogFactory.getLog(Load.class);
	private Modes mode;
	private IConnection connection;
	private int startLine;
	private int lastLine;
	private long processingTime = 0;
	private int logBlockSize;
	
	public abstract void executeLoad();

	public void execute() {
		long start = System.nanoTime();
		if (isExecutable()) {
			getContext().registerLoad(this); //register as processable
			executeLoad();
		}
		long end = System.nanoTime();
		processingTime = end - start;
	}
	
	public boolean isExecutable() {
		return getContext().isExecutable();
	}

	public LoadConfigurator getConfigurator() {
		return (LoadConfigurator)super.getConfigurator();
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}


	public Modes getMode() {
		return mode;
	}

	/**
	 * sets the load mode externally
	 * @param mode the load mode
	 */
	public void setMode(Modes mode) {
		this.mode = mode;
	}

	protected String getEncoding() throws RuntimeException {
		if (hasConnection()) {
			return getConnection().getEncoding();
		}
		return null;
	}

	public IConnection getConnection() throws RuntimeException {
		return connection;
	}

	public boolean hasConnection() {
		return connection != null;
	}

	protected IView getView() throws RuntimeException {
		IComponent view = getSourceManager().getFirst();
		if (view instanceof IView) {
			return ((IView)view);
		}
		else
			throw new RuntimeException("Load needs to have set a view as source.");
	}

	// This setting will get obsolete // 
	protected int getFirstRow() {
		try {
			startLine =  Math.max(Integer.parseInt(getParameter("start","1"))-1,0);
			if (startLine!=0)
				log.info("Parameter start line in load "+getName()+" is obsolete, use transform TableView for this setting.");
			return startLine;
		}
		catch (Exception e) {
			log.error("Failed to parse number of start line: "+e.getMessage());
		}
		return 0;
	}

	// This setting will get obsolete // 
	protected int getLastRow() {
		try {
			lastLine = Math.min(Integer.parseInt(getParameter("end",String.valueOf(Integer.MAX_VALUE-1))),Integer.MAX_VALUE-1);
			if(lastLine -1  < startLine){
				throw new ConfigurationException("Failure in load " + getName() + ", parameter end can not be smaller than start parameter");
			}
			if (lastLine!=Integer.MAX_VALUE-1)	
				log.info("Parameter end line in load "+getName()+" is obsolete, use transform TableView for this setting.");
			return lastLine;
		}
		catch (Exception e) {
			log.error("Failed to parse number of end line: "+e.getMessage());
		}
		return 0;
	}
	
	private IProcessor getProcessorInternal(IProcessor processor) throws RuntimeException {
		int lastRow=getLastRow();
		if(lastRow>= Integer.MAX_VALUE-1)
			processor.setLastRow(processor.getLastRow());
		else
			processor.setLastRow(Math.min(processor.getLastRow(),processor.getFirstRow()+lastRow));
		processor.setFirstRow(processor.getFirstRow()+ getFirstRow());
		return initProcessor(new RecodingProcessor(processor,Recoder.internalCoding,getEncoding()),Facets.INPUT);
	}

	protected IProcessor getProcessor() throws RuntimeException {
		IProcessor rows = getView().getProcessor();
		return getProcessorInternal(rows);
	}

	protected IProcessor getProcessor(IView.Views view) throws RuntimeException {
		IProcessor rows = getView().getProcessor(view);
		return getProcessorInternal(rows);
	}

	public Row getOutputDescription() throws RuntimeException {
		return getSourceManager().getFirst().getOutputDescription();
	}
	
	public long getProcessingTime() {
		return processingTime;
	}

	public void test() throws RuntimeException {
		super.test();
		if (hasConnection())
			getConnection().open();
		IProcessor processor = getView().isTreeBased() ? getProcessor(Views.PCWA) : getProcessor();
		processor.setLastRow(10);
		processor.run();
	}

	protected UpdateModes getDataPersistenceMode() {
		return PersistenceUtil.getDataPersistenceMode(getMode());
	}
	
	protected int getLogBlockSize() {
		return logBlockSize;
	}

	public void init() throws InitializationException {
		try {
			super.init();
			if (getConfigurator().hasSource()) {
				addManager(new SourceManager());
				getSourceManager().add(getConfigurator().getSource());
			}
			mode = getConfigurator().getMode();
			connection = getConfigurator().getConnection();
			logBlockSize = Integer.parseInt(getParameter().getProperty("logBlockSize", "0"));
			if (connection != null) {
				ConnectionManager manager = new ConnectionManager();
				manager.add(connection);
				addManager(manager);
			}			
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
