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
package com.jedox.etl.core.source.processor;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.node.ColumnNode;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;


import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class LoopProcessor extends Processor {
	
	private static final Log log = LogFactory.getLog(LoopProcessor.class);
	private IProcessor parameterProcessor;
	private IProcessor sourceProcessor;
	private boolean hasParameters = false;
	private SourceManager baseSources;
	private IContext parentContext;
	private Row row = new Row();
	private IContext contextToClean;
	private List<IProcessor> sourceProcessors = new ArrayList<IProcessor>();

	public LoopProcessor(IProcessor parameterProcessor, SourceManager baseSources) {
		this.parameterProcessor = parameterProcessor;
		this.baseSources = baseSources;
	}
	
/*	
	private void checkDB() {
		try {
			ResultSet rs = ConfigManager.getInstance().getInternalConnection().open().createStatement().executeQuery("SELECT * FROM INFORMATION_SCHEMA.TABLES where Table_Schema != 'INFORMATION_SCHEMA'");
			boolean hasNext = rs.next();
			while (hasNext) {
				for (int i = 0; i < rs.getMetaData().getColumnCount(); i++) {
					String name = rs.getString(i+1);
					System.out.print(name+",");
				}
				System.out.println();
				hasNext = rs.next();
			}
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (RuntimeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ConfigurationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.flush();
	}
*/
	
	private void cleanUp() {
		if (contextToClean != null) {
			/* context details are added via Context.clear() now
			for (IProcessor p : contextToClean.getProcessorsByOwner(null)) {
				parentContext.registerProcessor(p);
			}
			*/
			contextToClean.clear();
		}
	}
	
	public void setSourceProcessor(IProcessor processor) {
		super.setSourceProcessor(processor);
		sourceProcessors.add(processor);
	}
	
	private boolean setParameters() throws RuntimeException {
		try {
			Row row = parameterProcessor.next();
			//clear / invalidate  the source in its old context to clean up persistent structures
			cleanUp();
			if (row != null) {
				Properties properties = new Properties();
				String message = "Set loop variables to ";
				for (IColumn c : row.getColumns()) {
					message=message.concat(c.getName()+": "+c.getValueAsString()+" ");
					properties.setProperty(c.getName(), c.getValueAsString());
				}
				log.info(message);
				//create a new temporary runtime context for the new variables.
				IContext context = ContextManager.getInstance().provide(parentContext);
				// throw an exception if the columns names are not defined as variables
				for (IColumn c : row.getColumns()) {
					if(context.getVariables().get(c.getName()) == null)
						throw new RuntimeException("Column " + c.getName() + " in " + parameterProcessor.getName() + " is not defined as a variable.");
				}
				context.getVariables().putAll(properties);
				SourceManager contextSources = new SourceManager();
				for (ISource source : baseSources.getAll()) {
					//reinitialize the source in new context and re-set state.
					ISource s = (ISource)context.getComponent(source.getLocator());
					contextSources.add(s);
				}
				sourceProcessor = initProcessor(UnionProcessor.getInstance(contextSources.getProcessors()),Facets.INPUT);
				for (int i=0; i < this.row.size() && i < sourceProcessor.current().size(); i++) {
			    	ColumnNode node = (ColumnNode)this.row.getColumn(i);
			    	node.setInput(sourceProcessor.current().getColumn(i));
			    }
				this.setSourceProcessor(sourceProcessor);
				//set the context to clean
				contextToClean = context;
				return true;
			}
			else {
				// source processor necessary to get component output even if loop processor is empty; CSCHW: this uses default value of variables as parameter and potentially will throw an error if default value is invalid for underlying sources! 
				if (sourceProcessor == null) {
					sourceProcessor = initProcessor(UnionProcessor.getInstance(baseSources.getProcessors()),Facets.INPUT);
					this.setSourceProcessor(sourceProcessor);
				}
				return false;
			}
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	protected boolean fillRow(Row row) throws Exception {
		if (!hasParameters) return false;
		Row r = sourceProcessor.next();
		while (r == null) {
			if (setParameters())
				r = sourceProcessor.next();
			else
				return false;
		}
		return r != null;
	}

	protected Row getRow() {
		return row;
	}
	
	public void close() {
		super.close();
		cleanUp();
	}

	@Override
	protected void init() throws RuntimeException {
		if (baseSources.isEmpty()) {
			throw new RuntimeException("No base sources given to loop upon.");
		}
		parentContext = baseSources.getFirst().getContext();
	    hasParameters = setParameters();
	    if (!hasParameters) {
	    	log.info("Loop source "+parameterProcessor.getName()+" is empty. Result will be empty!");
	    } 
	    for (IColumn c : sourceProcessor.current().getColumns()) {
	    	CoordinateNode node = ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(),c);
	    	row.addColumn(node);
	    }
	}

	public long getOwnProcessingTime() {
		long time = 0;
		for (IProcessor p: sourceProcessors) {
			time += p.getOverallProcessingTime();
		}
		time += parameterProcessor.getOverallProcessingTime();
		return super.getOverallProcessingTime()-time;
	}
	

}
