package com.jedox.etl.core.node.treecompat;


import java.util.ArrayList;
import java.util.List;

import java.util.HashMap;

import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.IColumn.ColumnTypes;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.DimensionInfo;

public class PaloTreeManager implements IDimension {
	
	private TreeManager manager;
	
	private class Consolidation implements IConsolidation {
		private IElement parent;
		private IElement child;
		private double weight;
		
		public Consolidation(IElement parent, IElement child, double weight) {
			this.parent = parent;
			this.child = child;
			this.weight = weight;
		}

		@Override
		public IElement getParent() {
			return parent;
		}

		@Override
		public IElement getChild() {
			return child;
		}

		@Override
		public double getWeight() {
			return weight;
		}
		
	}
	
	public PaloTreeManager(String sourcename, String rootname) {
		manager = new TreeManager(sourcename, rootname);
	}
	
	public PaloTreeManager(TreeManager manager) {
		this.manager = manager;
	}
	
	public PaloTreeManager(IDimension dimension) {
		manager = new TreeManager(dimension.getName(), "#!__ROOT__!#"+dimension.getName());
		IAttribute[] attributes = dimension.getAttributes();
		//add elements and attributes
		for (IElement element : dimension.getElements(true)) {
			manager.createNode(element.getName(), 1);
			//System.out.println(element.getName() + " " + element.getAttributeValue("_origName"));
			//manager.createElement(element.getName());
			for (IAttribute a : attributes) {
				Object value = element.getAttributeValue(a.getName());
				if (value != null && !value.toString().isEmpty()) {
					manager.addAttribute(element.getName(), a.getName(), value, ColumnTypes.attribute, a.getType());
				}
			}
		}
		//add cosolidations
		List<IConsolidation> consolidations = new ArrayList<IConsolidation>();
		for (IElement element : dimension.getRootElements(false)) {
			HashMap<String,IElement[]> subtree = (HashMap<String, IElement[]>) element.getSubTree();
			for (String s : subtree.keySet()) {
				IElement parent = dimension.getElementByName(s, false);
				for (IElement child : subtree.get(s)) {
					Consolidation c = new Consolidation(parent,child,child.getWeight(parent));
					consolidations.add(c);
				}
			}
		}
		this.updateConsolidations(consolidations.toArray(new IConsolidation[consolidations.size()]));
	}
	
	public TreeManager getInternalTree() {
		return manager;
	}
	
	private IElement[] toElements(List<TreeNode> list) {
		IElement[] result = new IElement[list.size()];
		for (int i=0; i<list.size(); i++) {
			PaloElement element = new PaloElement(list.get(i).getElement(),manager);
			result[i] = element;
		}
		return result;
	}
	
	private IAttribute[] toAttributes(Row row) {
		IAttribute[] result = new IAttribute[row.size()];
		for (int i=0; i<row.size(); i++) {
			PaloAttribute attribute = new PaloAttribute();
			attribute.mimic(row.getColumn(i));
			result[i] = attribute;
		}
		return result;
	}
	
	private IColumn findElement(String name) throws PaloJException {
		IColumn elementColumn = manager.getElement(name);
		if (elementColumn == null) throw new PaloJException("Element with name "+name+" not found in "+manager.getClass().getName()+" "+manager.getName());
		return elementColumn;
	}
	
	private IColumn findAttribute(String name) throws PaloJException {
		IColumn attributeColumn = manager.getAttributeDefinition().getColumn(name);
		if (attributeColumn == null) throw new PaloJException("Attribute with name "+name+" not found in "+manager.getClass().getName()+" "+manager.getName());
		return attributeColumn;
	}

	@Override
	public void addAttributeConsolidation(IAttribute arg0, IAttribute arg1)
			throws PaloJException, PaloException {
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");

	}

	@Override
	public void addAttributeValues(IAttribute attribute, IElement [] elements, Object[] values) throws PaloJException, PaloException {
		if (elements.length != values.length) throw new PaloJException("Array elements and values have to be of same size.");
		for (int i = 0; i < elements.length; i++) {
			ElementType type = findAttribute(attribute.getName()).getElementType();
			manager.addAttribute(elements[i].getName(), attribute.getName(), values[i], ColumnTypes.attribute,type);
		}

	}

	@Override
	public void addAttributes(String[] names, ElementType[] types)
			throws PaloJException, PaloException {
		if (names.length != types.length) throw new PaloJException("Array names and types have to be of same size.");
		for (int i = 0; i < names.length; i++) {
			PaloAttribute column = new PaloAttribute();
			column.setName(names[i]);
			column.setColumnType(ColumnTypes.attribute);
			column.setElementType(types[i].toString());
			manager.getAttributeDefinition().addColumn(column);
		}

	}

	@Override
	public IElement addBaseElement(String name, ElementType type)
			throws PaloJException, PaloException {
		IColumn element = manager.createElement(name);
		element.setElementType(type.toString());
		return new PaloElement(element,manager);
	}

	@Override
	public void addElements(String[] names, ElementType[] types)
			throws PaloException, PaloJException {
		if (names.length != types.length) throw new PaloJException("Array names and types have to be of same size.");
		for (int i = 0; i < names.length; i++) {
			IColumn element = manager.createElement(names[i]);
			element.setElementType(types[i].toString());
		}

	}

	@Override
	public IAttribute getAttributeByName(String name) throws PaloException,
			PaloJException {
		IColumn a = findAttribute(name);
		PaloAttribute attribute = new PaloAttribute();
		attribute.mimic(a);
		return attribute;
	}

	@Override
	public DimensionInfo getDimensionInfo() throws PaloException {
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");
	}

	@Override
	public IElement getElementByName(String name,boolean withAttributes)
			throws PaloException, PaloJException {
		return new PaloElement(findElement(name),manager);
	}

	@Override
	public IElement[] getElements(boolean withAttributes) throws PaloException,
			PaloJException {
		return toElements(manager.getUniqueNodes(manager.getOrderedSubtree(manager.getRootNode(), true)));
	}

	@Override
	public int getId() {
		return -1;
	}

	@Override
	public IElement[] getRootElements(boolean withAttributes) throws PaloException,
			PaloJException {
		return toElements(manager.getNodesFromRow(manager.getChildren(manager.getRoot())));
	}

	@Override
	public DimensionType getType() {
		return DimensionType.DIMENSION_NORMAL;
	}

	@Override
	public IConsolidation newConsolidation(IElement parent,IElement child, double weight) {
		return new Consolidation(parent,child,weight);
	}

	@Override
	public void removeAttributeConsolidations(IAttribute arg0)
			throws PaloException, PaloJException {
		throw new PaloException("Not supported by "+manager.getClass().getCanonicalName()+" backend.");

	}

	@Override
	public void removeAttributeValues(IAttribute attribute, IElement [] elements)
			throws PaloJException, PaloException {
		// TODO Auto-generated method stub
		for (IElement element : elements) {
			IColumn c = findElement(element.getName());
			IColumn a = findAttribute(attribute.getName());
			manager.removeAttribute(c, a);
		}

	}

	@Override
	public void removeAttributes(IAttribute[] attributes) throws PaloException,
			PaloJException {
		//NOTE: manager removes only the attribute definitions, not the actual attributes already assigned to elements. However, you will not be able to get them any more, unless you re-add the definition.
		for (IAttribute attribute : attributes) {
			manager.getAttributeDefinition().removeColumn(attribute.getName());
		}

	}

	@Override
	public void removeConsolidations(IElement[] elements) throws PaloException,
			PaloJException {
		for (IElement element : elements) {
			IColumn elementColumn = findElement(element.getName());
			Row myChildren = manager.getChildren(elementColumn);
			for (IColumn c : myChildren.getColumns()) {
				TreeNode tn = (TreeNode)c;
				manager.removeConsolidation(tn);
			}
		}

	}

	@Override
	public void removeElements(IElement[] elements) throws PaloException,
			PaloJException {
		for (IElement element : elements) {
			manager.removeElement(element.getName());
		}

	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		manager.setName(name);
	}

	@Override
	public void updateConsolidations(IConsolidation[] consolidations)
			throws PaloException, PaloJException {
		//delete all consolidations first
		manager.clearConsolidations();
		//add the new consolidations
		for (IConsolidation consolidation : consolidations) {
			IColumn child = findElement(consolidation.getChild().getName());
			IColumn parent = null;
			if (consolidation.getParent() != null) {
				parent = findElement(consolidation.getParent().getName());
				TreeNode c = new TreeNode(parent,child,consolidation.getWeight());
				manager.addConsolidation(c);
			}
		}

	}

	@Override
	public IAttribute[] getAttributes() throws PaloException, PaloJException {
		return toAttributes(manager.getAttributeDefinition());
	}

	@Override
	public String getName() {
		return manager.getName();
	}

	@Override
	public HashMap<String, IElement[]> getChildrenMap() throws PaloException {
		HashMap<String,IElement[]> result = new HashMap<String,IElement[]>();
		for (IElement e : getElements(false)) {
			result.put(e.getName(), e.getChildren());
		}
		return result;
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getAttributesMap()throws PaloException {
		HashMap<String, HashMap<String, Object>> result = new HashMap<String, HashMap<String, Object>>();
		for (IElement e : getElements(false)) {
			HashMap<String, Object> attributes = new HashMap<String, Object>();
			for (IAttribute a : getAttributes()) {
				Object value = e.getAttributeValue(a.getName());
				if (value != null) attributes.put(a.getName(), value);
			}
			if (!attributes.isEmpty()) result.put(e.getName(), attributes);
		}
		return result;
	}

	@Override
	public HashMap<String, HashMap<String, Double>> getWeightsMap() throws PaloException {
		HashMap<String, HashMap<String, Double>> result = new HashMap<String, HashMap<String, Double>>();
		for (IElement e : getElements(false)) {
			HashMap<String, Double> weights = new HashMap<String, Double>();
			for (IElement p : e.getParents()) {
				weights.put(p.getName(), e.getWeight(p));
			}
			if (!weights.isEmpty()) result.put(e.getName(), weights);
		}
		return result;
	}

	@Override
	public void setCacheTrustExpiry(int arg0) {
		//do nothing, since we are not expireing any cache. we ARE the backend;		
	}

	@Override
	public void updateElementsType(IElement[] arg0, ElementType arg1)
			throws PaloException, PaloJException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public IElement[] getBasesElements(boolean arg0) throws PaloException,
			PaloJException {
		// TODO Auto-generated method stub
		return null;
	}

}
