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
package com.jedox.etl.core.node.treecompat;


import java.util.HashMap;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.HashSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnManager;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.util.TypeConversionUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;


/**
 * Class for {@link IColumn} and {@link TreeNode} Management.
 * A TreeNodeManager holds and manipulates the intermediate representation of all tree based data (e.g. used for olap dimensions) generated by various datasources.
 * To form a tree this class provides exactly one internal root node. All nodes are descending from this root node.
 * <br>
 * {@link IColumn IColumns} represent the unique elements of a Tree and are registered by their technical name. {@link TreeNode TreeNodes} are used to form a hierarchy of IColumns and allow multiple consoldiations of TreeNodes by specifying parent-child relationships.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TreeManager {

	private class AttributeWrapper {
		private IColumn element;
		private IColumn definition;
		private Object value;
		
		public AttributeWrapper(IColumn element, IColumn definition) {
			this.element = element;
			this.definition = definition;
		}

		public void setValue(Object value) {
			this.value = value;
		}

		public Object getValue() {
			return value;
		}
		
		public String getId() {
			return element.getName()+ "." + definition.getName();
			//return element.hashCode()+ "." + definition.hashCode();
		}
		
		public IColumn getAttribute() {
			Column column = new Column();
			column.mimic(definition);
			column.setValue(value);
			return column;
		}
		
	}
	
	private HashMap<String, IColumn> elements = new HashMap<String, IColumn>();
	private HashMap<IColumn, Row> parents = new HashMap<IColumn, Row> ();
	private HashMap<IColumn, Row> children = new HashMap<IColumn, Row> ();
	private ColumnManager attributeDefinitions = new ColumnManager();
	private HashMap<String,AttributeWrapper> attributeLookup = new HashMap<String,AttributeWrapper>();
	private IColumn root = null;
	private String name ="unknown";
	private static final Log log = new MessageHandler(LogFactory.getLog(TreeManager.class));

	/**
	 * Constructor
	 * @param rootname the name of the root node to create
	 */
	public TreeManager(String sourcename, String rootname) {
		this.name = sourcename;
		setRoot(new Column(rootname));
	}

	/**
	 * gets the name of this TreeManager, which is the name of the underlying datasource.
	 * @return
	 */
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * gets a specific parent of a IColumn by its name
	 * @param element the base IColumn
	 * @param parent the name of the parent to get.
	 * @return the parent as TreeNode or null, if no such parent exists.
	 */
	public TreeNode getParent(IColumn element, String parent) {
		Row p = getParents(element);
		return (p != null) ? (TreeNode) p.getColumn(parent) : null;
	}

	/**
	 * gets all parents of a IColumn
	 * @param element the base IColumn to get its parent.
	 * @return a Row containing the parents as TreeNodes or an empty Row, if no parents exists.
	 */
	public Row getParents(IColumn element) {
		Row p = parents.get(element);
		return (p != null) ? p : new Row(true);
	}

	/**
	 * gets a specific child of a IColumn by its name
	 * @param element the base IColumn
	 * @param child the name of the child to get
	 * @return the child as TreeNode or null, if no such child exists
	 */
	public TreeNode getChild(IColumn element, String child) {
		Row c = children.get(element);
		return (c != null) ? (TreeNode) c.getColumn(child) : null;
	}

	/**
	 * gets all children of a IColumn. As a special case all unassigned IColumns are returned as children of the internal root node.
	 * @param element the base IColumn
	 * @return a Row containing the children as TreeNodes or an empty Row, if no children exists.
	 */
	public Row getChildren(IColumn element) {
		Row c = children.get(element);
		if (c == null)
			c = new Row(true);
		if (isRoot(element.getName())) { //add unassigned nodes on root level
			Row overlay = new Row(true);
			overlay.addColumns(c);
			for (String nodeName : elements.keySet()) {
				IColumn e = getElement(nodeName);
				if ((getParents(e).size() == 0) && !isRoot(nodeName)) {
					TreeNode consolidation = new TreeNode(element, e, 1);
					//consolidation.mask(true);
					overlay.addColumn(consolidation);
				}
			}
			return overlay;
		}
		return c;
	}

	private boolean isCyclic(TreeNode consolidation) {
		if (consolidation.getName().equals(consolidation.getParent()))
			return true;
		ArrayList<TreeNode> ancestors = getAncestors(this,new ArrayList<TreeNode>(),consolidation.getParent());
		for (TreeNode ancestor : ancestors) {
			if (ancestor.getName().equals(consolidation.getName())) {
				if (consolidation.getName().equals(consolidation.getParent().getName())) {
					log.warn("Node "+consolidation.getName()+" cannot be consolidated under itself in source "+getName()+". The consolidation is ignored.");
				} else {
					log.warn("Failed to consolidate node "+consolidation.getName()+" with parent "+consolidation.getParent().getName()+" due to a circular reference in source "+getName()+".");
				}

				return true;
			}
		}
		return false;
	}

	/**
	 * adds a consolidation in form of a TreeNode specifying a parent-child relationship inducing a hierarchy.
	 * @param consolidation the relation to add.
	 */
	public void addConsolidation(TreeNode consolidation) {
		if (!isCyclic(consolidation)) {
			Row p = parents.get(consolidation.getElement());
			Row c = children.get(consolidation.getParent());
			if (p == null) {
				p = new Row(true);
				parents.put(consolidation.getElement(), p);
			}
			if (c == null) {
				c = new Row(true);
				children.put(consolidation.getParent(), c);
			}
			p.addColumn(consolidation,consolidation.getParentName());
			c.addColumn(consolidation);
			if(consolidation.getParentName() != getRoot().getName() && p.containsColumn(getRoot().getName())){
				// Remove an existing consolidation of the element to the root node
				p.removeColumn(getRoot().getName());
			    (children.get(getRoot())).removeColumn(consolidation.getElement().getName());			    
				// getChildren(getRoot()).removeColumn(consolidation.getElement().getName());
			}
		}
	}

	/**
	 * adds a consolidation in form of a TreeNode specifying a parent-child relationship inducing a hierarchy.
	 * @param node the name of the base/child IColumn
	 * @param parent the name of the parent IColumn
	 * @param weight the weight of the relationship.
	 */
	public void addConsolidation(String node, String parent, double weight) {
		TreeNode consolidation = new TreeNode(getElement(parent), getElement(node), weight);
		addConsolidation(consolidation);
	}

	/**
	 * removes a consolidation in form of a TreeNode removing a parent-child relationship from the hierarchy.
	 * @param consolidation the relation to remove
	 */
	public void removeConsolidation(TreeNode consolidation) {
		if (consolidation != null) {
			Row p = parents.get(consolidation.getElement());
			Row c = children.get(consolidation.getParent());
			if (p != null)
				p.removeColumn(consolidation);
			if (c != null)
				c.removeColumn(consolidation);
		}
	}

	/**
	 * renames a IColumn
	 * @param oldName the current name of the IColumn
	 * @param newName the new name of the IColumn
	 */
	public void renameElement(String oldName, String newName) {
		IColumn element = elements.remove(oldName);
		if (element != null) {
			element.setName(newName);
			elements.put(newName, element);
		}
	}

	/**
	 * Sets an externally provided {@link IColumn} as root node. All children of the original root node are preserved as children of the new root node.
	 * @param root the root node to set
	 * @return the original root node
	 */
	public IColumn setRoot(IColumn root) {
		IColumn original = getRoot();
		this.root = root;
		addNodeInternal(this,root,true);
		if (original != null) {
			Row c = getChildren(original);
			for (TreeNode n : getNodesFromRow(c)) {
				n.setParent(root);
			}
			children.remove(original);
			children.put(root, c);
			elements.remove(original);
		}
		return original;
	}

	/**
	 * gets the IColumn serving as internal tree root.
	 * @return the IColumn serving as internal tree root.
	 */
	public IColumn getRoot() {
		return root;
	}

	/**
	 * determines if a IColumn is the internal root of this manager.
	 * @param name the name of the IColumn
	 * @return true, if IColumn is the internal root, false otherwise.
	 */
	public boolean isRoot(String name) {
		return root.getName().equals(name);
	}

	/**
	 * gets the IColumn serving as internal root node as TreeNode relation with no parent. Convenience method when working on TreeNode level.
	 * @return the IColumn serving as internal tree root in TreeNode representation
	 */
	public TreeNode getRootNode() {
		return new TreeNode(null, getRoot(), 1);
	}

	/**
	 * gets the number of registered {@Link TreeNode}s of this manager
	 * @return the number of registered {@Link TreeNode}s of this manager
	 */
	public int getSize() {
		return elements.size();
	}

	private IColumn addNodeInternal(TreeManager manager, IColumn element, boolean withAttributes) {
		//use Element of this tree, if already present
		IColumn presentElement = getElement(element.getName());
		if (presentElement == null) {
			elements.put(element.getName(), element);
		}
		if (withAttributes) { //add foreign node attributes to present element
			for (IColumn definition : manager.getAttributeDefinition().getColumns()) {
				Object attributeValue = manager.getAttributeValue(element, definition);
				addAttribute(element.getName(),definition.getName(),attributeValue,definition.getColumnType(),definition.getElementType());
			}
		}
		return getElement(element.getName());
	}

	protected void addDescendants(TreeManager manager, List<TreeNode> nodes) {
		for (int i=0; i<nodes.size();i++) {
			TreeNode d = nodes.get(i);
			addNodeInternal(manager, manager.getElement(d.getName()),true);
			addConsolidation(d.getName(),d.getParentName(),d.getWeight());
		}
	}

	/**
	 * Adds a full subtree of an external manager to this manager.
	 * @param manager the external manager providing the subtree to add
	 * @param node the IColumn serving as root of the subtree of the external manager
	 * @param parent the IColumn serving as parent / hook in this manager for the subtree of the external manager.
	 * @param weight the weight for the new consolidation between the internal parent node and the external subtree to add.
	 * @return the consolidation created between the internal and the external node.
	 */
	public TreeNode addSubtree(TreeManager manager, IColumn node, IColumn parent, double weight) {
		TreeNode result = null;
		if (node != null) {
			if (parent == null) parent = getRoot();
			IColumn element = addNodeInternal(manager, node, true);
			result = new TreeNode(parent,element,weight);
			addConsolidation(result);
			addDescendants(manager,getDescendants(manager,new ArrayList<TreeNode>(), node));
		}
		return result;
	}

	private TreeNode addNode(IColumn node, IColumn parent, double weight) {
		return addSubtree(this,node,parent,weight);
	}

	/**
	 * Builds a new TreeNode consolidation. If the child IColumn does not yet exist, it is created. The parent IColumn has to already exist within this manager.
	 * @param name the name of the IColumn to use as child in the relation.
	 * @param parent the name of the (already registered) IColumn serving as parent in the new relation. if null the internal root is used as parent.
	 * @param weight the weight of this new parent-child relation.
	 * @return the TreeNode defining the new consolidation or null, if the parent does not exist.
	 */
	public TreeNode createNode(String name, String parent, double weight) {
		return createNode(name,getElement(parent),weight);
	}

	/**
	 * Builds a new TreeNode consolidation. If the child IColumn does not yet exist, it is created. As parent the internal root is used.
	 * @param name the name of the IColumn to use as child in the relation.
	 * @param weight the weight of this new parent-child relation.
	 * @return the TreeNode defining the new consolidation.
	 */
	public TreeNode createNode(String name, double weight) {
		return createNode(name,getRoot(),weight);
	}

	/**
	 * Builds a new TreeNode consolidation. If the child IColumn does not yet exist, it is created. The parent IColumn has to already exist within this manager.
	 * @param name name the name of the IColumn to use as child in the relation.
	 * @param parent the (already registered) IColumn serving as parent in the new relation. if null the internal root is used as parent.
	 * @param weight the weight of this new parent-child relation.
	 * @return
	 */
	public TreeNode createNode(String name, IColumn parent, double weight) {
		if (name != null) {
			IColumn n = getElement(name);
			if (n == null)
				n = createElement(name);
			return addNode(n,parent,weight);
		}
		return null;
	}

	/**
	 * creates a new IColumn without assigning it to the internal hierarchy. Until assigned otherwise, this IColumn is considered as foster child of the internal root. If a IColumn with this name already exists, no action is taken.
	 * @param name the name of the new IColumn to create.
	 * @return the newly created IColumn.
	 */
	public IColumn createElement(String name) {
		IColumn t = getElement(name);
		if (t == null) {
			t = new Column(name);
			addNodeInternal(this,t,false);
		}
		return t;
	}

	/**
	 * removes a IColumn and all its consolidations from this manager. Orphaned IColumns are are considered as foster children of the internal root node until assigned otherwise. The internal root node cannot be removed but only renamed.
	 * @param name the name of the IColumn to remove
	 * @return the removed IColumn or null, if no such IColumn exists.
	 */
	public IColumn removeElement(String name) {
		if (!isRoot(name)) {
			IColumn result = elements.remove(name);
			if (result != null) {
				ArrayList<IColumn> myChildren = new ArrayList<IColumn>();
				ArrayList<IColumn> myParents = new ArrayList<IColumn>();
				myChildren.addAll(getChildren(result).getColumns());
				myParents.addAll(getParents(result).getColumns());
				for (IColumn c : myChildren) {
					TreeNode tn = (TreeNode)c;
					removeConsolidation(tn);
				}
				for (IColumn c : myParents) {
					TreeNode tn = (TreeNode)c;
					removeConsolidation(tn);
				}
				parents.remove(result);
				children.remove(result);
			}
			return result;
		}
		return null;
	}


	/**
	 * gets a IColumn by its name
	 * @param name the name of the IColumn
	 * @return the IColumn or null, if no such IColumn exists.
	 */
	public IColumn getElement(String name) {
		if (name == null) return null;
		IColumn n = elements.get(name);
		return n;
	}

	private ArrayList<TreeNode> getAncestors(TreeManager manager, ArrayList<TreeNode> nodes, IColumn n) {
		Row row = manager.getParents(n);
		if (row.size() > 0) {
			for (IColumn c : row.getColumns()) {
				TreeNode node = (TreeNode)c;
				nodes.add(node);
				getAncestors(manager,nodes, node.getParent());
			}
		}
		//root node is always an ancestor
		nodes.add(getRootNode());
		return nodes;
	}

	/**
	 * Recursivly gets all descendants of a node
	 * @param nodes the list to be populated
	 * @param n the actual node to be processed
	 * @return all descendant nodes
	 */
	private ArrayList<TreeNode> getDescendants(TreeManager manager, ArrayList<TreeNode> nodes, IColumn n) {
		Row row = manager.getChildren(n);
		if (row.size() > 0) {
			for (IColumn c : row.getColumns()) {
				TreeNode node = (TreeNode)c;
				nodes.add(node);
				getDescendants(manager,nodes, node.getElement());
			}
		}
		return nodes;
	}

	/**
	 * Gets an ordered subtree as list of TreeNodes. The order is determinded by the tree (depth first). Siblings are in the order as inserted.
	 * @param n the node to use as subtree root
	 * @param ignoreRoot if true the subtree root itself is ommitted in the result.
	 * @return the list of ordered TreeNodes of the subtree.
	 */
	public ArrayList<TreeNode> getOrderedSubtree(TreeNode n, boolean ignoreRoot) {
		ArrayList<TreeNode> result = getDescendants(this,new ArrayList<TreeNode>(), n.getElement());
		if (!ignoreRoot) result.add(0,n);
		return result;
	}

	/**
	 * Gets the full ordered tree as list of TreeNodes. The order is determinded by the tree (depth first). Siblings are in the order as inserted.
	 * @param ignoreRoot if true the internal root itself is ommitted in the result.
	 * @return the list of ordered TreeNodes of the full tree
	 */
	public ArrayList<TreeNode> getOrderedTree(boolean ignoreRoot) {
		return getOrderedSubtree(getRootNode(),ignoreRoot);
	}

	/**
	 * Gets all unique TreeNodes from a given list of TreeNodes possible containing duplicates. Convenience method.
	 * @param nodes the list of TreeNodes to consider.
	 * @return a list of TreeNodes, where each TreeNode is unique.
	 */
	public ArrayList<TreeNode> getUniqueNodes(List<TreeNode> nodes) {
		ArrayList<TreeNode> result = new ArrayList<TreeNode>();
		//only add nodes with the same name once
		HashSet<String> added = new HashSet<String>();
		for (TreeNode t: nodes) {
			if (!added.contains(t.getName())) {
				result.add(t);
				added.add(t.getName());
			}
		}
		return result;
	}

	/**
	 * gets all TreeNodes, which act as leafs in the tree.
	 * @param ignoreRoot ignore the internal root node in the result (only affects the result, when tree is empty)
	 * @return the list of leaf nodes.
	 */
	public ArrayList<TreeNode> getLeafNodes(boolean ignoreRoot) {
		ArrayList<TreeNode> result = new ArrayList<TreeNode>();
		for (TreeNode t: getOrderedTree(ignoreRoot)) {
			if (getChildren(t.getElement()).size() == 0) {
				result.add(t);
			}
		}
		return result;
	}

	/**
	 * gets all TreeNodes, which do not act as leafs in the tree
	 * @param ignoreRoot ignore the internal root node in the result
	 * @return the list of branch nodes (all nodes, which are not leafs)
	 */
	public LinkedList<TreeNode> getBranchNodes(boolean ignoreRoot) {
		LinkedList<TreeNode> result = new LinkedList<TreeNode>();
		//only add nodes with the same name once
		HashSet<String> added = new HashSet<String>();
		for (TreeNode t: getOrderedTree(ignoreRoot)) {
			if ((getChildren(t.getElement()).size() > 0) && !added.contains(t.getName())) {
				//add nodes in reverse order, so that the root appears last.
				result.add(0,t);
				added.add(t.getName());
			}
		}
		return result;
	}

	/**
	 * gets all TreeNodes, which have a given attribute value set.
	 * @param attributeName the name of the attribute
	 * @param attributeValue the value of the attribute
	 * @return the list of TreeNodes having this attribute value
	 */
	public LinkedList<TreeNode> getNodesByAttribute(String attributeName, String attributeValue) {
		LinkedList<TreeNode> result = new LinkedList<TreeNode>();
		//only add nodes with the same name once
		HashSet<String> added = new HashSet<String>();
		for (TreeNode t: getOrderedTree(true)) {
			IColumn attribute = getAttribute(t.getElement(),attributeDefinitions.getColumn(attributeName));
			if (attribute != null && attribute.getValueAsString().equals(attributeValue) && !added.contains(t.getName())) {
				//add nodes in reverse order, so that the root appears last.
				result.add(0,t);
				added.add(t.getName());
			}
		}
		return result;
	}

	/**
	 * gets the TreeNodes stored in a Row as list. Convenience method for accessing the TreeNodes, since Row only gives access to the generic {@link IColumn} interface.
	 * @param row the Row containing the TreeNodes.
	 * @return the list of TreeNodes
	 */
	public ArrayList<TreeNode> getNodesFromRow(Row row) {
		ArrayList<TreeNode> result = new ArrayList<TreeNode>();
		for (IColumn node : row.getColumns())
			if (node instanceof TreeNode)
				result.add((TreeNode)node);
		return result;
	}

	/**
	 * gets the underlying IColumns from a list of TreeNodes. The IColumns returned are the elements acting as child in the parent-child relation established by the TreeNodes. Convenience method.
	 * @param nodes the list of TreeNodes
	 * @return the list of the underlying IColumns.
	 */
	public ArrayList<IColumn> getElementsFromNodes(List<TreeNode> nodes) {
		ArrayList<IColumn> result = new ArrayList<IColumn>();
		HashSet<String> added = new HashSet<String>();
		for (IColumn node : nodes) {
			if (!added.contains(node.getName())) {
				result.add(getElement(node.getName()));
				added.add(node.getName());
			}
		}
		return result;
	}

	/**
	 * Recursivly gets all nodes from a specified level (distance to the root node) in the tree
	 * @param nodes to list to be populated.
	 * @param n the current node to be inspected.
	 * @param currentLevel the level the current nodes resides in.
	 * @param targetLevel the level where nodes should be delivered.
	 * @return a list of nodes of a given level (distance) with respect to the root node.
	 */
	private ArrayList<TreeNode> getNodesFromLevel(ArrayList<TreeNode> nodes, TreeNode n, int currentLevel, int targetLevel) {
		if (currentLevel < targetLevel-1) {
			for (IColumn c: getChildren(n.getElement()).getColumns()) {
				getNodesFromLevel(nodes, (TreeNode)c, currentLevel+1,targetLevel);
			}
		}
		if (currentLevel == targetLevel-1) {
			for (IColumn c : getChildren(n.getElement()).getColumns()) {
				nodes.add((TreeNode)c);
			}
		}
		if (targetLevel == 0) nodes.add(getRootNode());
		return nodes;
	}

	/**
	 * Gets all TreeNodes from a specified level (distance to the root node) in the tree
	 * @param level the distance to the root node (root node itself is on level 0)
	 * @return a list of TreeNodes of a given level (distance) with respect to the root node.
	 */
	public ArrayList<TreeNode> getNodesFromLevel(int level) {
		return getNodesFromLevel(new ArrayList<TreeNode>(),getRootNode(),0,level);
	}

	/**
	 * Gets all defined attributes of all TreeNodes in a subtree.
	 * @return a ColumnManager holding the attributes
	 */
	public ColumnManager getAttributeDefinition() {
		return attributeDefinitions;
	}
	
	public Object getAttributeValue(IColumn element, IColumn definition) {
		if (definition != null) {
			AttributeWrapper wrapper = new AttributeWrapper(element,definition);
			wrapper = attributeLookup.get(wrapper.getId());
			if (wrapper != null) {
				return wrapper.getValue();
			}
		}
		return null;
	}
	
	public IColumn getAttribute(IColumn element, IColumn definition) {
		if (definition != null) {
			AttributeWrapper wrapper = new AttributeWrapper(element,definition);
			wrapper = attributeLookup.get(wrapper.getId());
			if (wrapper != null) {
				return wrapper.getAttribute();			}
		}
		return null;
	}
	
	public void removeAttribute(IColumn element, IColumn definition) {
		if (definition != null) {
			AttributeWrapper wrapper = new AttributeWrapper(element,definition);
			wrapper = attributeLookup.remove(wrapper.getId());
		}
	}

	/**
	 * adds an attribute to a IColumn
	 * @param elementName the name of the IColumn
	 * @param key the name of the attribute
	 * @param value the value of the attribute to set
	 * @param ctype the type of attribute (alias, data, attribute, etc)
	 * @param etype the element type of attribute (text, numeric)
	 */
	public void addAttribute(String elementName, String key, Object value, IColumn.ColumnTypes ctype, ElementType etype) {
		if (key != null) {
			if (etype.equals(ElementType.ELEMENT_NUMERIC) && value != null) {
				boolean numeric = new TypeConversionUtil().isNumeric(value.toString());
				if (!numeric) {
					log.warn("Attribut "+key+" of Element "+elementName+" is of type numeric, but attribut value is not numeric: "+value.toString());
					//set value to null to ignore it
					value = null;
				}
			}
			IColumn element = getElement(elementName);
			if (element != null) {
				IColumn definition = attributeDefinitions.getColumn(key);
				if (definition != null && (!definition.getColumnType().equals(ctype) || !definition.getElementType().equals(etype))) {
					log.warn("Attribute "+definition.getName()+" has occurrences with incompatible types in tree. Using "+definition.getColumnType().toString()+","+definition.getElementType().toString()); 
				}
				if (definition == null) {
					Column column = new Column(key);
					column.setColumnType(ctype);
					column.setElementType(etype.toString());
					attributeDefinitions.addColumn(column);
					definition = column;
				}
				AttributeWrapper wrapper = new AttributeWrapper(element,definition);
				wrapper.setValue(value);
				attributeLookup.put(wrapper.getId(), wrapper);
			}
		}
	}

	/**
	 * adds a text attribute to a IColumn
	 * @param elementName the name of the IColumn
	 * @param key the name of the attribute
	 * @param value the value of the attribute to set
	 * @param ctype the type of attribute (alias, data, attribute, etc)
	 */
	public void addAttribute(String elementName, String key, Object value, IColumn.ColumnTypes ctype) {
		addAttribute(elementName, key, value, ctype, ElementType.ELEMENT_STRING);
	}

	private ArrayList<TreeNode> filter(TreeNode n, ArrayList<TreeNode> toRemove) {
		for (IColumn c: getChildren(n.getElement()).getColumns()) {
			//System.err.println(n.getName() +": "+ c.getName());
			filter((TreeNode)c, toRemove);
		}
		//add all non masked leaf nodes which are not root.
		if (!n.isMasked() && getChildren(n.getElement()).size() == 0 && n.getParent() != null)
			toRemove.add(n);
		return toRemove;
	}

	public void filterTree() {
		ArrayList<TreeNode> toRemove = filter(getRootNode(), new ArrayList<TreeNode>());
		while (toRemove.size() > 0) {
			for (TreeNode n : toRemove) {
				removeConsolidation(n);
				if (getParents(n.getElement()).size() == 0)
					removeElement(n.getElement().getName());
			}
			toRemove = filter(getRootNode(), new ArrayList<TreeNode>());
		}
	}
	
	public int hasCaseSensitiveElements() {
		HashSet<String> namesInsensitive = new HashSet<String>();
		for (String s : elements.keySet()) {
			namesInsensitive.add(s.toLowerCase());
		}
		return elements.keySet().size() - namesInsensitive.size();
	}
	
	public void clearConsolidations() {
		children.clear();
		parents.clear();
	}

	/**
	 * clears this manager removing all IColumns (except for the internal root) and TreeNodes.
	 */
	public void clear() {
		elements.clear();
		children.clear();
		parents.clear();
		attributeDefinitions.clear();
		attributeLookup.clear();
		addNodeInternal(this,getRoot(),false);
	}

}