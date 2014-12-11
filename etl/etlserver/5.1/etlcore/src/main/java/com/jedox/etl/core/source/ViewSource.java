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
package com.jedox.etl.core.source;

import java.util.ArrayList;

import org.jdom.Element;

import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.component.ComponentDescriptor;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.ViewSourceConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.ISource.CacheTypes;
import com.jedox.etl.core.source.processor.*;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
/**
 * A Source proxy class implementing the view interface to unify the access to table and tree based sources. Uses its own cache for sorting the results of the underlying source, if specified so. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ViewSource extends TreeSource implements IView {
	
	//private static final Log log = LogFactory.getLog(ViewSource.class);
	private ISource baseSource;
	
	public ViewSource() {
		notifyRetrieval = false;
		setConfigurator(new ViewSourceConfigurator());
	}
	
	/**
	 * returns a minimal static ComponentDescriptor for this view. Since Views are not defined in any component.xml files this is necessary for using factories to create this source with a given xml snipped from a project definition.
	 * @return the Component Descriptor describing this Source for Factories. 
	 */
	public static ComponentDescriptor getViewDescriptor() {
		return new ComponentDescriptor("view",ViewSource.class.getCanonicalName(),ITypes.Sources,false,null,null,null,null);
	}
	
	/**
	 * Constructor to directly create a ViewSource without the help of a factory and a xml-configuration
	 * @param locator the locator of the underlying base source
	 * @param context the context of this source
	 * @param format the view format to use to render the output
	 * @throws InitializationException
	 */
	public ViewSource(Locator locator, IContext context, String format) throws InitializationException {
		notifyRetrieval = false;
		Element xml = new Element("source");
		xml.setAttribute("nameref", locator.getName());
		if (format != null)
			xml.setAttribute("format", format);
		ViewSourceConfigurator configurator = new ViewSourceConfigurator();
		configurator.setLocator(locator.clone().reduce(), context);
		configurator.setXML(xml,new ArrayList<String>());
		setConfigurator(configurator);
		init();
	}
	
	public ViewSourceConfigurator getConfigurator() {
		return (ViewSourceConfigurator)super.getConfigurator();
	}
	
	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}
	
	public IConnection getConnection() {
		//view source has no connection;
		return null;
	}
	
	public Views getFormat() {
		return (super.getFormat().equals(Views.NONE)) ? getBaseSource().getFormat() : super.getFormat();
	}
	
	public Row getAttributes() throws RuntimeException {
		if (getBaseSource() instanceof ITreeSource)
			return ((ITreeSource)getBaseSource()).getAttributes();
		return new Row();
	}
	
	public String getPersistentView() {
		if (isOptimizePersistence()) {
			if (isTreeBased()) {
				return super.getPersistentView();
			} else {
				if (getCacheType().equals(getBaseSource().getCacheType())) {
					return getBaseSource().getPersistentView();
				} else {
					return super.getPersistentView();
				}
			}
		} else {
			return super.getPersistentView();
		}
	}
	
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		//take the appropriate processor in table format from the source
		//this method is not called, when direct rendering can be made. This is the case with TreeSources 
		//this method is called, when the source format is identical to the target format (direct throughput).
		IProcessor result = null;
		if (getBaseSource() instanceof ITreeSource) {
			//get source processor in view format.
			result = ((ITreeSource)getBaseSource()).getProcessor(getFormat());
			result.setLastRow(size);
		} 
		else {
			result = getBaseSource().getProcessor(size);
		}
		return result;
	}
	
	public ITreeProcessor renderTree(IView.Views format) throws RuntimeException {
		if (isTreeBased()) {
			return generate();
		} else { //do implicit conversion with format, which is disabled in standard generate method
			ITreeProcessor processor = initTreeProcessor(new TreeBuildProcessor(getProcessor(getSampleSize()),format),Facets.OUTPUT);
			processor.run();
			setTreeManager(processor.getManager());
			return processor;
		}
	}

	
	public ITreeProcessor buildTree() throws RuntimeException {
		if (isTreeBased()) {
			ITreeSource source = (ITreeSource) getBaseSource();
			//Directly take tree. Do not call getSourceProcessor to convert to a table.
			//Root node is a property of this sources and overrides the the root of the underlying source
			ITreeProcessor sourceProcessor = source.generate();
			ITreeManager manager = sourceProcessor.getManager();
			setTreeManager(manager);
			return sourceProcessor;
		}
		else { 
			/* this is no longer allowed for standard processing. Use appropriate Pipelines to create trees.
			//generate the corresponding view as tree
			if (getSource().getFormat().equals(Views.NONE)) //use view format to build tree from
				new ViewGenerator(getTreeManager()).generate(getProcessor(getSampleSize()), getFormat());
			else //use source format to build tree from
				new ViewGenerator(getTreeManager()).generate(getProcessor(getSampleSize()), getSource().getFormat());
			return getTreeManager();
			*/
			throw new RuntimeException("Source "+getBaseSource().getName()+" does not support views, since it implements no tree representation.");
		} 
	}

	
	public ISource getBaseSource() {
		return baseSource;
	}
	
	
	public IProcessor getProcessor() throws RuntimeException {
		return getProcessor(getFormat());
	}
	
	
	public IProcessor getProcessor(Views view) throws RuntimeException {
		IProcessor processor = null;
		if (isTreeBased()) {
			//processor will call the generate method of this view to return an appropriate tree. The generate method holds all logic necessary
			processor = initProcessor(new TreeViewProcessor(this,view),Facets.OUTPUT);
			processor.setLastRow(getSampleSize());
		}
		else {//deliver implicit format as is 
			//use standard variant with callback an getSourceProcessor and caching mechanism if needed.
			processor = getProcessor(getSampleSize());
		}
		return processor;
	}
	
	public Row getOutputDescription() throws RuntimeException {
		if (isTreeBased()) //get converted outputs
			return new TreeViewProcessor(this,getFormat(),true).getOutputDescription(); //optimization avoids full initialization if possible
		else
			return getBaseSource().getOutputDescription(); //pass through from source
	}

	public boolean isTreeBased() {
		//source is a tree source 
		return ((getBaseSource() instanceof ITreeSource)); 
	}
	
	protected void postInit() throws InitializationException {
		super.postInit();
		if (isOptimizePersistence()) {
			if (isTreeBased()) {//cschw only tweak locator if treebased and multiple view may exists on tree. if tablebased base source persistence can be used thus!
				getLocator().setPersistentTable(getLocator().getPersistentTable()+"View"+getFormat().toString());
			}
		} else {
			getLocator().setPersistentSchema(getLocator().getRootName()+"_"+getLocator().getContext()+"View"+getFormat().toString());
		}
		
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			addManager(new SourceManager());
			baseSource = getConfigurator().getSource();
			//reset locator to source locator
			setLocator(baseSource.getLocator().clone(),getContext());
			//setAliasMap(baseSource.getAliasMap());
			getSourceManager().add(baseSource);
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
