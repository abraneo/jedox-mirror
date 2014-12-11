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
package com.jedox.etl.components.extract;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.treecompat.PaloTreeManager;
import com.jedox.etl.core.node.treecompat.TreeManager;
import com.jedox.etl.core.node.treecompat.TreeNode;

import java.util.List;

public class TimeToDateExtract extends DateExtract implements IExtract {
	
	private String aggregate;
	
	public TimeToDateExtract() {
		super();
	}
	
	private int getLevel(String format, String aggregate) {
		String s = format.replace(getDelimiter(aggregate), "").toUpperCase();
		return s.indexOf(aggregate.toUpperCase());
	}
	
	private IColumn aggregate(TreeManager manager, IColumn parent, List<TreeNode> children, int start) {
		if (children.size() > 0) {
			for (int i = children.size()-(start+1); i > 0; i--) {
				TreeNode c = children.get(i);
				String name = c.getName();
				//create a new aggregate node
				IColumn aggNode = manager.createNode(aggregate+name, parent,1).getElement();
				//re-map caption alias to aggregate node.
				IColumn alias = manager.getAttribute(c, manager.getAttributeDefinition().getColumnOfType(IColumn.ColumnTypes.alias, "caption"));
				if (alias != null)
					manager.addAttribute(aggNode.getName(),alias.getName(), alias.getValueAsString() + " agg.", IColumn.ColumnTypes.alias);
				//remove data node from manager
				manager.removeConsolidation(c);
				//add data node to newly created aggregate node.
				manager.addSubtree(manager, c.getElement(), aggNode, c.getWeight());
				//aggregate node is now new parent to add nodes.
				parent = aggNode;
			}
			TreeNode c = children.get(0);
			manager.removeConsolidation(c);
			manager.addSubtree(manager, c.getElement(), parent, c.getWeight());
			parent = c.getElement();
		}
		return parent;
	}
	
	protected ITreeManager buildTree() throws RuntimeException {
		TreeManager manager = buildInternalTree();
		if (aggregate.equalsIgnoreCase("All")) {
			//get all now obsolete consolidated nodes.
			List<IColumn> removeNodes = manager.getElementsFromNodes(manager.getBranchNodes(true));
			//consolidate all leaf nodes in aggregate style
			aggregate(manager,manager.getRoot(),manager.getLeafNodes(true),0);
			//remove ex branch nodes, which are now empty. 
			for (IColumn n : removeNodes)
				manager.removeElement(n.getName());
		} 
		else { 
			int level = getLevel(getDateFormat(),aggregate);
			List<IColumn> parents = manager.getElementsFromNodes(manager.getNodesFromLevel(level));
			for (IColumn n : parents) {
				aggregate(manager,n,manager.getNodesFromRow(manager.getChildren(n)),1);
			}
		}
		setTreeManager(new TreeManagerNG(new PaloTreeManager(manager)));
		clearInternalTree();
		return getTreeManager();
	}
	
	public void init() throws InitializationException {
		try {
			super.init();
			aggregate = getParameter("aggregate",getDateFormat().substring(0,1));
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
