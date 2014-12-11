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

import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.source.TreeSourceConfigurator;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.TreeProcessor;
import com.jedox.palojlib.interfaces.IAttribute;


/**
 * Abstract base class for all sources having an internal tree representation, which can be rendered in multiple column-oriented ways using views. All concrete tree based sources should inherit from this class. 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class TreeSource extends TableSource implements ITreeSource {
	
	private ITreeManager nodemanager = new TreeManagerNG("default");
	private Date timestamp = null;
	private int size = 0;
	private static Log log = LogFactory.getLog(TreeSource.class);
	

	public TreeSource() {
		setConfigurator(new TreeSourceConfigurator());
	}
	
	public TreeSourceConfigurator getConfigurator() {
		return (TreeSourceConfigurator)super.getConfigurator();
	}
		
	protected void invalidateTreeCache() {
		timestamp = null;
	}
	
	protected void stampTreeCache() {
		timestamp = new Date();
	}
	
	protected boolean isTreeCacheDirty() {
		return ((timestamp == null));
	}
	
	/**
	 * Does the actual tree building
	 * @return the built tree
	 * @throws RuntimeException
	 */
	protected ITreeManager buildTree() throws RuntimeException {
		return getTreeManager();
	}
	
	/**
	 * generates the internal tree representation, unless it it already built on valid (cache not expired) 
	 */
	public ITreeManager generate() throws RuntimeException { 
		if (isExecutable() && isTreeCacheDirty()) {
			boolean autocommit = getTreeManager().isAutoCommit();
			getTreeManager().setAutoCommit(false);
			buildTree();
			getTreeManager().setAutoCommit(autocommit);
			stampTreeCache();
		}
		return getTreeManager();
	}
	
	protected Views getFormat(Views format, Views fallback) {
		return format.equals(Views.NONE) ? fallback : format;
	}
	
	public IProcessor getProcessor(Views view) throws RuntimeException {
		return new TreeProcessor(this,getFormat(view,Views.PC));
	}

	public ITreeManager getTreeManager() {
		return nodemanager;
	}
	
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		//Tree sources should not be cached. But if they are cached, the standard processor is returned
		setSize(size);
		IProcessor processor =  getProcessor(getFormat());
		processor.setLastRow(size);
		return processor;
	}
	
	protected void setTreeManager(ITreeManager manager) {
		if (manager != null) {
			nodemanager = manager;
			invalidateTreeCache();
		}
	}
	
	public Row getAttributes() throws RuntimeException {
		IAttribute[] attributes = generate().getAttributes();
		Row result = new Row();
		for (IAttribute a : attributes) {
			Column c = new Column(a.getName());
			c.setElementType(a.getType().toString());
			result.addColumn(c);
		}
		return result;
	}
	
	public Row getOutputDescription() throws RuntimeException {
		return getProcessor(getFormat()).getOutputDescription();
	}
	
	public void test() throws RuntimeException {
		generate();
	}
	
	public void invalidate() {
		super.invalidate();
		invalidateTreeCache();
		getTreeManager().clear();
	}
	
	/**
	 * sets the size requested from the Source Processor in order to optionally generate the tree only partially 
	 */
	public void setSize(int size) {
		this.size = size;
	}
	
	/**
	 * gets the size requested from the Source Processor 
	 */
	public int getSize() {
		return size;
	}
	
	
	public void init() throws InitializationException {
		try {
			super.init();
			setTreeManager(getConfigurator().getNodeManager());
			setCaching(false);	
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
