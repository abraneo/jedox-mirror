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

import java.util.ArrayList;
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
import com.jedox.etl.core.source.ITreeSource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TreeSource;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeJoin extends TreeSource implements ITransform {

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

	protected ITreeManager buildTree() throws RuntimeException {
		ITreeManager nodemanager = super.buildTree();
		List<ITreeSource> sources = getSourceManager().getTreeSources();
		for (int i=0; i<hookList.size(); i++) {
			String hook = hookList.get(i);
			Double weight = hookWeights.get(i);
			//empty is default when no root is specified.
			ITreeElement hookNode = (!hook.isEmpty()) ? nodemanager.getElement(hook) : null;
			ITreeSource generator = sources.get(i);
			generator.generate();
			if (hookNode != null || hook.isEmpty()) {//hook already present
				ITreeManager m = generator.getTreeManager();
				nodemanager.addAttributes(m.getAttributes(), false);
				for (ITreeElement e : m.getRootElements(true)) {
					nodemanager.addSubtree(e, hook.isEmpty() ? null : new IConsolidation[]{new Consolidation(hookNode, e, weight)});
				}
			}
			else {//add with hook node on root level
				log.debug("Hook node "+hook+" does not exist yet in joint tree. It is created on root level.");
				ITreeManager m = generator.getTreeManager();
				nodemanager.addAttributes(m.getAttributes(), false);
				ITreeElement newRootNode = nodemanager.provideElement(hook, ElementType.ELEMENT_NUMERIC);
				for (ITreeElement e : m.getRootElements(true)) {
					nodemanager.addSubtree(e, new IConsolidation[]{new Consolidation(newRootNode, e, weight)});
				}
			}
		}
		return nodemanager;
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
