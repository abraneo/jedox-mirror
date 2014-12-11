package com.jedox.etl.core.node.treecompat;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;

public class PaloElement implements IElement {
	
	private IColumn column;
	private TreeManager manager;
	
	public PaloElement(IColumn column, TreeManager manager) {
		this.column = column;
		this.manager = manager;
	}
	
	private IElement[] toElements(Row row) {
		IElement[] result = new IElement[row.size()];
		if (row != null) for (int i=0; i<row.size(); i++) {
			PaloElement element = new PaloElement(((TreeNode)row.getColumn(i)).getElement(),manager);
			result[i] = element;
		}
		return result;
	}
	
	private IElement[] toParentElements(Row row) {
		List<IElement> result = new ArrayList<IElement>();
		if (row != null) for (int i=0; i<row.size(); i++) {
			IColumn parent = ((TreeNode)row.getColumn(i)).getParent();
			if (!manager.getRoot().equals(parent)) {
				PaloElement element = new PaloElement(parent,manager);
				result.add(element);
			}
		}
		return result.toArray(new IElement[result.size()]);
	}

	@Override
	public Object getAttributeValue(String attributeName) throws PaloJException, PaloException {
		return manager.getAttributeValue(column, manager.getAttributeDefinition().getColumn(attributeName));
	}

	@Override
	public int getChildCount() throws PaloException, PaloJException {
		Row row = manager.getChildren(column);
		return (row != null) ? row.size() : 0;
	}

	@Override
	public IElement[] getChildren() throws PaloException, PaloJException {
		return toElements(manager.getChildren(column));
	}

	@Override
	public int getParentCount() throws PaloException, PaloJException {
		return getParents().length;
	}

	@Override
	public IElement[] getParents() throws PaloException, PaloJException {
		return toParentElements(manager.getParents(column));
	}

	@Override
	public HashMap<String, IElement[]> getSubTree() throws PaloException,
			PaloJException {
		List<TreeNode> nodeList = manager.getUniqueNodes(manager.getOrderedSubtree(new TreeNode(column), false));
		HashMap<String,IElement[]> result = new HashMap<String,IElement[]>();
		for (TreeNode node : nodeList) {
			result.put(node.getName(), toElements(manager.getChildren(node.getElement())));
		}
		return result;
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getSubTreeAttributes()
			throws PaloException, PaloJException {
		List<TreeNode> nodeList = manager.getUniqueNodes(manager.getOrderedSubtree(new TreeNode(column), false));
		HashMap<String,HashMap<String,Object>> result = new HashMap<String,HashMap<String, Object>>();
		for (TreeNode node : nodeList) {
			HashMap<String,Object> attributeValues = new HashMap<String,Object>();
			for (IColumn attribute : manager.getAttributeDefinition().getColumns()) {
				Object value = manager.getAttributeValue(node.getElement(), attribute);
				if (value != null) {
					attributeValues.put(attribute.getName(), value);
				}
			}
			result.put(node.getName(), attributeValues);
		}
		return result;
	}

	@Override
	public ElementType getType() {
		return column.getElementType();
	}

	@Override
	public double getWeight(IElement parent) throws PaloException, PaloJException {
		if (parent == null) return 1;
		Row parents = manager.getParents(column);
		IColumn parentColumn = parents.getColumn(parent.getName());
		if (parentColumn instanceof TreeNode) {
			TreeNode consolidation = (TreeNode) parentColumn;
			return consolidation.getWeight();
		}
		return 1;
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		manager.renameElement(column.getName(), name);
		
	}
	@Override
	public String getName() {
		return column.getName();
	}

}
