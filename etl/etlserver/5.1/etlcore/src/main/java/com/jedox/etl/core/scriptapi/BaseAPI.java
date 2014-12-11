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
package com.jedox.etl.core.scriptapi;

import java.util.HashMap;

import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.ViewSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.MailUtil;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;

/**
 * Common API for all functions for convenience methods and interaction with ETL-Server functionality. Especially useful for scripting functions.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class BaseAPI extends ScriptAPI {

	private HashMap<String, IProcessor> processors = new HashMap<String, IProcessor>();
	private HashMap<String, Source> sources = new HashMap<String, Source>();
	private MailUtil mailer;
	
	private class APIProcessor extends Processor
	{

		public APIProcessor(IProcessor processor) throws RuntimeException
		{
			setSourceProcessor(processor);
			setOwner(processor.getOwner());
			setFacet(Facets.HIDDEN);
			initialize();
		}

		@Override
		protected boolean fillRow(Row row) throws Exception
		{
			return getSourceProcessor().next() != null;
		}

		@Override
		protected Row getRow() throws RuntimeException
		{
			return getSourceProcessor().current();
		}

		@Override
		protected void init() throws RuntimeException
		{
			// TODO Auto-generated method stub
		}

		public void close()
		{
			super.close();
			getOwner().getContext().clear();
		}

	}
	

	/**
	 * fast test if a given input data is numeric or not
	 * @param inputData the data to be tested for being numeric
	 * @return true, if numeric, false otherwise.
	 */
	public boolean isNumeric(String inputData) {
		 return TypeConversionUtil.isNumeric(TypeConversionUtil.convertToNumericString(inputData));
	}
	
	protected Locator getContextLocator(String type, String name) {
		Locator locator = new Locator().add(getProjectName()).add(type).add(name);
		locator.setContext(getComponentContext().getName());
		return locator;
	}

	/**
	 * sets up a new {@link IProcessor Processor} for access to a {@link ISource Source}. Variables for usage within the source can be set via {@link #setProperty(String, String)}
	 * @param source the name of the source
	 * @param format the format of the processor for tree sources. Ignored for table sources.
	 * @param size the size of the processor
	 * @return a new processor for this source
	 * @throws RuntimeException
	 */
	public IProcessor setupProcessor(@Scanable(type=ITypes.Managers.sources) String name, String format, int size) throws RuntimeException {
		try {
			IContext processorContext = ContextManager.getInstance().provide(getComponentContext());
			processorContext.addVariables(getApiProperties());
			ISource source = new ViewSource(getContextLocator(ITypes.Sources,name),processorContext,format);
			IProcessor processor = new APIProcessor(source.getProcessor(size));
			processors.put(name, processor);
			return processor;
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}

	/**
	 * sets up a new {@link IProcessor Processor} for access to a {@link ISource Source}. Variables for usage within the source can be set via {@link #setProperty(String, String)}
	 * @param source the name of the source
	 * @return a new processor for this source
	 * @throws RuntimeException
	 */
	public IProcessor setupProcessor(@Scanable(type=ITypes.Managers.sources) String name) throws RuntimeException {
		return setupProcessor(name,null,0);
	}

	/**
	 * gets a {@link IProcessor Processor} for access to a {@link ISource Source}. If no processor exists for this source, a new one is created via {@link #setupProcessor(String)}. Else the existing processor is returned.
	 * @param source the name of the source
	 * @return the processor processing this source
	 * @throws RuntimeException
	 */
	public IProcessor getProcessor(@Scanable(type=ITypes.Managers.sources) String name) throws RuntimeException {
		IProcessor processor = processors.get(name);
		if (processor == null)
			processor = setupProcessor(name);
		return processor;
	}

	public Source initSource (@Scanable(type=ITypes.Managers.sources) String name, String format, int size) {
		Source source;
		try {
			source = new Source(setupProcessor(name,format,size));
		}
		catch (Exception e) {
			log.error("Could not initialise source "+name+": "+e.getMessage());
			return null;
		}
		sources.put(name, source);
		return source;	
	}
	
	public Source initSource (@Scanable(type=ITypes.Managers.sources) String name) {
		return initSource(name,null,0);
	}
	
	public Source getSource (@Scanable(type=ITypes.Managers.sources) String name) {
		Source source = sources.get(name);
		if (source == null)
			source = initSource(name);
		return source;		
	}
	
	/** closes all previously opened processors
 		@deprecated this should be handled by Excecution if dependencies are properly specified. However, it should not hurt here, as long as not invoked explicitly by script. 
	 */
	public void close() {
		for (IProcessor p : processors.values())
			p.close();
		sources.clear();
		processors.clear();
		super.close();
	}
	
	public MailUtil getMailer() {
		if (mailer == null) mailer = new MailUtil();
		return mailer;
	}

	public String getLocalFilesDir() {
		return Settings.getInstance().getDataDir();
	}
	
	
}
