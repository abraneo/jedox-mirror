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
package com.jedox.etl.components.function;

import java.util.HashMap;
import java.util.Map;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.components.config.function.LookupConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.NamingUtil;

public class Lookup extends Function {

	private ISource source;
	private String defaultValue;
	private HashMap<String, Object> lookup;
	private String[] froms = null;
	private Views treeformat;
	private static final Log log = LogFactory.getLog(Lookup.class);	

	public Lookup() {
		setConfigurator(new LookupConfigurator());
	}

	public LookupConfigurator getConfigurator() {
		return (LookupConfigurator) super.getConfigurator();
	}

	private HashMap<String, Object> initLookup() throws ConfigurationException, RuntimeException {
		HashMap<String, Object> lookup = new HashMap<String,Object>();
		// get Source Processor
		IProcessor processor;
		if (source instanceof IView && ((IView)source).isTreeBased()) {
			processor = ((IView)source).getProcessor(treeformat);
			}
		else
			processor = source.getProcessor();
		if (processor != null) {
			getUsedProcessorList().add(processor);
			Map<String, String> map = getConfigurator().getMap();
			Row row = processor.next();
			if (row==null) {
				log.warn("Lookup table in function "+getName()+" on source "+processor.getName()+" is empty.");
				return lookup;
			}
			String fromName = map.keySet().iterator().next();
			int[] keyCols = new int[froms.length];;
			int valueCol = 1;
			String toName = map.get(fromName);
			for(int i=0;i<froms.length;i++){
				IColumn key = row.getColumn(froms[i]);
				if (key == null) {
					throw new ConfigurationException("Column "+froms[i]+" not found in Lookup-source "+processor.getName());
				}
				keyCols[i] = row.indexOf(key);
			}
			
			IColumn valueColumn = row.getColumn(toName);
			if (valueColumn == null) {
				throw new ConfigurationException("Column "+toName+" not found in Lookup-source "+processor.getName());
			}
			valueCol = row.indexOf(valueColumn);
			
			while (row != null) {
				StringBuilder keyBuilder = new StringBuilder();
				
				keyBuilder.append(row.getColumn(keyCols[0]).getValueAsString());
				for(int i=1;i<keyCols.length;i++){
					keyBuilder.append(getConfigurator().getSeparator() + row.getColumn(keyCols[i]).getValueAsString());
				}
				Object value = row.getColumn(valueCol).getValue();
				lookup.put(keyBuilder.toString(), value);
				row = processor.next();
			}

		}
		return lookup;
	}

	protected Object transform(Row values) throws FunctionException {
		try {
			if (lookup == null)
				lookup = initLookup();
		}
		catch (Exception e) {
			// Avoid repeated calls of initLookup and Raise Error Message
			lookup = new HashMap<String,Object>();
			String message = "Error in Initialisation of Lookup function "+getName();
			if (e.getMessage()!=null)
				message=message+": "+e.getMessage();				
			log.error(message);
			throw new FunctionException(message);
		}
		
		String key = null;
		StringBuilder keyBuilder = new StringBuilder();
		keyBuilder.append(values.getColumn(0).getValueAsString());
		for(int i=1;i<froms.length;i++){
			keyBuilder.append(getConfigurator().getSeparator() + values.getColumn(i).getValueAsString());
		}
		key=keyBuilder.toString();
		
		Object result = lookup.get(key);
		if (result == null)
			return (defaultValue == null) ? key : defaultValue;
		else
			return result;
	}

	protected void validateInputs() throws ConfigurationException {
		checkInputSize(getConfigurator().getFromColumns().length,false);
	}

	protected void validateParameters() throws ConfigurationException {
		String sourceName = getConfigurator().getSourceName();
		if (sourceName == null) {
			throw new ConfigurationException("Parameter source must be specified.");
		}
	}
	
	public void close() {
		if(lookup!=null){
			lookup.clear();
			lookup=null;
		}
	}

	public void init() throws InitializationException {
		try {
			super.init();
			defaultValue = getConfigurator().getDefault();
			if(defaultValue!=null)
				defaultValue = defaultValue.replaceAll(NamingUtil.spaceValue(), " ");
			froms = getConfigurator().getFromColumns();
			treeformat = Views.valueOf(getConfigurator().getTreeFormat().toUpperCase());			
			// add Source Manager
			source = getConfigurator().getSource();
			SourceManager manager = new SourceManager();
			addManager(manager);
			manager.add(source);			
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
