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
package com.jedox.etl.components.transform;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import com.jedox.etl.components.config.transform.TreeJoinConfigurator;
import com.jedox.etl.components.config.transform.TreeJoinConfigurator.Hook;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Consolidation;
import com.jedox.etl.core.node.tree.ITreeElement;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeColumn;
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeJoin extends TreeSource implements ITransform {
	
	private class TreeJoinProcessor extends Processor implements ITreeProcessor {
		
		private List<ITreeSource> sources;
		private ITreeManager manager;
		private Row row = new Row();
		private List<IElement> elements;
		private int currentElement = 0;
		private long nodeProcessingTime = 0;
		
		public TreeJoinProcessor(List<ITreeSource> sources, ITreeManager manager) {
			this.sources = sources;
			this.manager = manager;
		}

		@Override
		public ITreeManager getManager() {
			return manager;
		}

		@Override
		protected boolean fillRow(Row row) throws Exception {
			if (currentElement < elements.size()-1) {
				currentElement++;
				TreeColumn c = row.getColumn(0, TreeColumn.class);
				c.setElement(elements.get(currentElement));
				return true;
			}
			return false;
		}

		@Override
		protected Row getRow() throws RuntimeException {
			return row;
		}
		
		public int getRowsAccepted() {
			return elements.size();
		}

		@Override
		protected void init() throws RuntimeException {
			TreeColumn treeColumn = new TreeColumn();
			row.addColumn(treeColumn);
			elements = new ArrayList<IElement>();
			for (int i=0; i<hookList.size(); i++) {
				String hook = hookList.get(i);
				Double weight = hookWeights.get(i);
				//empty is default when no root is specified.
				ITreeElement hookNode = (!hook.isEmpty()) ? manager.getElement(hook) : null;
				ITreeSource generator = sources.get(i);
				ITreeProcessor sourceProcessor = generator.generate();
				nodeProcessingTime += sourceProcessor.getOverallProcessingTime(); //fix own processing time, since we register no source processors 
				elements.addAll(Arrays.asList(sourceProcessor.getManager().getElements(false)));
				if (hookNode != null || hook.isEmpty()) {//hook already present
					ITreeManager m = generator.getTreeManager();
					manager.addAttributes(m.getAttributes(), false);
					for (ITreeElement e : m.getRootElements(true)) {
						manager.addSubtree(e, hook.isEmpty() ? null : new IConsolidation[]{new Consolidation(hookNode, e, weight)});
					}
				}
				else {//add with hook node on root level
					log.debug("Hook node "+hook+" does not exist yet in joint tree. It is created on root level.");
					ITreeManager m = generator.getTreeManager();
					manager.addAttributes(m.getAttributes(), false);
					ITreeElement newRootNode = manager.provideElement(hook, ElementType.ELEMENT_NUMERIC);
					for (ITreeElement e : m.getRootElements(true)) {
						manager.addSubtree(e, new IConsolidation[]{new Consolidation(newRootNode, e, weight)});
					}
				}
			}
		}
		
		public long getOwnProcessingTime() {
			return super.getOverallProcessingTime() - nodeProcessingTime;
		}
		
	}

	private ArrayList<String> hookList = new ArrayList<String>();
	private ArrayList<Double> hookWeights = new ArrayList<Double>();
	private static final Log log = LogFactory.getLog(TreeJoin.class);

	public TreeJoin() {
		setConfigurator(new TreeJoinConfigurator());
	}

	public TreeJoinConfigurator getConfigurator() {
		return (TreeJoinConfigurator) super.getConfigurator();
	}

	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}

	public Row getAttributes() throws RuntimeException {
		Row row = new Row();
		for (ITreeSource s: getSourceManager().getTreeSources()) {
			row.addColumns(s.getAttributes());
		}
		return row;
	}

	public ITreeProcessor buildTree() throws RuntimeException {
		ITreeManager nodemanager = getTreeManager();
		return initTreeProcessor(new TreeJoinProcessor(getSourceManager().getTreeSources(),nodemanager),Facets.INPUT);
	}

	private void addTreeSource(ITreeSource generator, String hookNode, double weight) throws RuntimeException {
		getSourceManager().add(generator);
		hookList.add(hookNode);
		hookWeights.add(weight);
		invalidateTreeCache();
	}

	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			getSourceManager().setAllowDuplicates(true);
			for (Hook hook : getConfigurator().getHooks()) {
				addTreeSource((ITreeSource)hook.source, hook.node, hook.weight);
			}
		}
		catch (Exception e) {
			throw new InitializationException("In transform " + getConfigurator().getName() + ": " + e.getMessage());
		}
	}

}
