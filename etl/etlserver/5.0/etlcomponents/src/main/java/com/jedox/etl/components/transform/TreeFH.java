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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.transform;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.TreeConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.LevelNode;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.ViewGenerator;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.UnionProcessor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.transform.TransformInputProcessor;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeFH extends TreeSource implements ITransform {

	private ColumnManager columns;
	private RowFilter filter;
	private String defaultElement;
	private String defaultParentElement;
	boolean skipEmptyLevel;
	private static Log log = LogFactory.getLog(TreeFH.class);

	public TreeFH() {
		setConfigurator(new TreeConfigurator());
	}

	public TreeConfigurator getConfigurator() {
		return (TreeConfigurator)super.getConfigurator();
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}

	public ColumnManager getColumnManager() {
		return columns;
	}

	protected FunctionManager getFunctionManager() {
		return (FunctionManager)getManager(ITypes.Functions);
	}

	protected Row getLevels() {
		return getColumnManager().getColumnsOfType(IColumn.ColumnTypes.level);
	}
	
	public Row getAttributes() {
		Row row = new Row();
		for (IColumn c : getLevels().getColumns()) {
			LevelNode level = (LevelNode) c;
			row.addColumns(level.getAttributes());
		}
		return row;
	}

	public IProcessor getProcessor(Views view) throws RuntimeException {
		//check if format is not given, which causes no generation but a pass-through of the input source.
		if (view.equals(Views.NONE))
			return getSourceProcessor(getSampleSize());
		//else generate and Render Tree
		return super.getProcessor(view);
	}

	public IProcessor getSourceProcessor(int size) throws RuntimeException {
		if (isExecutable()) {
			//return native format
			IProcessor sourceProcessor = UnionProcessor.getInstance(getSourceManager().getProcessors());
			IProcessor processor = new TransformInputProcessor(sourceProcessor,getFunctionManager(), getColumnManager(), getName());
			// processor.setName(getName());
			processor.setState(getState());
			processor.addFilter(filter);
			processor.setLastRow(size);
			return processor;
		}
		return null;
	}

	private String getValues(Row row) {
		StringBuffer names = new StringBuffer();
		for (int j=0; j<row.size();j++) {
			IColumn c = row.getColumn(j);
			names.append(c.getValueAsString()+",");
		}
		names.deleteCharAt(names.length()-1);
		return names.toString();
	}

	protected ITreeManager buildTree() throws RuntimeException {
		MessageHandler handler = new MessageHandler(log);
		ITreeManager manager = super.buildTree();
		IProcessor processor = getSourceProcessor(0);
		Row row = processor.next();
		//int countRows = 0;
		while (row != null) {
			try {
				ITreeElement parent = null;
				for (int i=0; i<getLevels().size(); i++) {
					LevelNode cn = (LevelNode) row.getColumn(i);
					//take value from the result set, where the column points to.
					String name = cn.getValueAsString();
					if (name.trim().isEmpty()) {
						if (!skipEmptyLevel) {
							// Check if after empty node a lower level has a filled node
							for(int j=i+1;j<getLevels().size(); j++){
								LevelNode ln = (LevelNode) row.getColumn(j);
								if((ln.getValue()) != null && !ln.getValue().toString().trim().isEmpty() ){
									handler.warn("In transform "+getName()+ " value " + ln.getValueAsString()+ " of "+ln.getName() + " has empty parent and is therefor ignored. Use a default value in the source to fix this.");
									break;
								}
							}
							break;
						}	
					} 
					// Ignore node if equal name as parent node
					else if (parent != null && name.equals(parent.getName())) {
						log.debug("Ignore consolidation of node "+name+" to parent with same name");
					}
					else {
						ITreeElement tn = manager.provideElement(name, cn.getElementType());
						manager.addConsolidation(parent,tn,cn.getWeight());
						Row attributes = cn.getAttributes();
						for (IColumn c : attributes.getColumns()) {
							//check if there is a definition.
							IAttribute a = manager.getAttributeByName(c.getName());
							if (a == null) { //add definition in the fly
								Attribute attribute = manager.addAttribute(c.getName(), c.getElementType(ElementType.ELEMENT_STRING));
								if (c.getColumnType().equals(IColumn.ColumnTypes.alias)) attribute.setMode(AttributeModes.ALIAS);
								a = attribute;
							}
							manager.addAttributeValue(a.getName(), tn.getName(), c.getValue());
						}
						parent = tn;
					}
				}
			}
			catch (Exception e) {
				handler.error("Failed to build tree from data "+getValues(row)+" in transform "+getName()+": "+e.getMessage());
			}
			row = processor.next();
		}

		// add default element
		new ViewGenerator(manager).addDefaultElement(defaultElement, defaultParentElement);
		// add consolidations
		manager.commitConsolidations();
		manager.commitAttributeValues();
		// Check the number of levels in tree
		int levelsCount = manager.getLevelsCount();
		log.debug("Number of Levels: "+levelsCount);
		if (levelsCount > getLevels().size())
			log.warn("Transform "+getName()+" defines "+getLevels().size()+" levels but the resulting tree has a maximum depth of "+levelsCount+". Check the uniqueness of data between the levels.");
		return manager;
	}


	public Row getOutputDescription() throws RuntimeException {
		return getLevels();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			columns = getConfigurator().getColumnManager();
			filter = getConfigurator().getFilter();
			addManager(new SourceManager());
			addManager(new FunctionManager());
			getSourceManager().addAll(getConfigurator().getSources());
			getFunctionManager().addAll(getConfigurator().getFunctions());
			defaultElement = getConfigurator().getDefaultElement();
			defaultParentElement = getConfigurator().getDefaultParentElement();
			skipEmptyLevel = getConfigurator().getSkipEmptyLevel(); 
			if (defaultParentElement!=null && defaultElement==null)
				throw new InitializationException("No default parent element can be defined without a default element");
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}
}
