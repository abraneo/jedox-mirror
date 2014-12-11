package com.jedox.etl.core.node.tree;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import com.jedox.etl.core.node.NamedValue;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class Element extends NamedValue<ElementType> implements ITreeElement, Cloneable {
	
	private Element[] children;
	private Element[] parents;
	private Map<IElement,Double> weights;
	private HashMap<String,Object> attributes;
	
	protected Element(String name, ElementType type) {
		setName(name);
		setValue(type);
	}
	

	@Override
	public Object getAttributeValue(String attributeName) throws PaloJException, PaloException {
		return (attributes != null) ? attributes.get(attributeName) : null;
	}

	@Override
	public int getChildCount() throws PaloException, PaloJException {
		return (children != null) ? children.length : 0;
	}

	@Override
	public Element[] getChildren() throws PaloException, PaloJException {
		return (children != null) ? children : new Element[0];
	}
	
	protected Element getChild(String name) {
		if (children != null) for (Element c : children) {
			if (name.equals(c.getName())) return c;
		}
		return null;
	}
	
	protected void addChild(Element child) {
		if (children != null) {
			Element[] newChildren = Arrays.copyOfRange(children, 0, children.length);
			newChildren[children.length] = child;
			children = null;
			setParents(newChildren);
		} else {
			setParents(new Element[]{child});
		}
	}
	
	protected void removeChild(Element child) {
		if (children != null && children.length > 0) {
			Element[] newChildren = new Element[children.length-1];
			int j = 0;
			for (int i=0; i<children.length; i++) {
				if (!children[i].equals(child)) {
					newChildren[j] = children[i];
					j++;
				}
			}
			children = null;
			setChildren(newChildren);
		}
	}
	
	
	protected void setChildren(Element[] children) {
		this.children = children;
	}

	@Override
	public int getParentCount() throws PaloException, PaloJException {
		return (parents != null) ? parents.length : 0;
	}

	@Override
	public Element[] getParents() throws PaloException, PaloJException {
		return (parents != null) ? parents : new Element[0];
	}
	
	protected void setParents(Element[] parents) {
		this.parents = parents;
	}
	
	protected Element getParent(String name) {
		if (parents != null) for (Element p : parents) {
			if (name.equals(p.getName())) return p;
		}
		return null;
	}
	
	protected void addParent(Element parent) {
		addParent(parent,1);
	}
	
	protected void addParent(Element parent, double weight) {
		if (parents != null) {
			Element[] newParents = Arrays.copyOfRange(parents, 0, parents.length);
			newParents[parents.length] = parent;
			addWeight(parent,weight);
			parents = null;
			setParents(newParents);
		} else {
			setParents(new Element[]{parent});
		}
	}
	
	protected void removeParent(Element parent) {
		if (parents != null && parents.length > 0) {
			Element[] newParents = new Element[parents.length-1];
			int j = 0;
			for (int i=0; i<parents.length; i++) {
				if (!parents[i].equals(parent)) {
					newParents[j] = parents[i];
					j++;
				} else if (weights != null) weights.remove(parent); 
			}
			parents = null;
			setParents(newParents);
		}
	}
	
	private void addWeight(Element parent, double weight) {
		if (weight != 1.0) {
			if (weights == null) weights = new HashMap<IElement,Double>(1,1);
			weights.put(parent, weight);
		}
	}

	@Override
	public HashMap<String, IElement[]> getSubTree() throws PaloException,PaloJException {
		HashMap<String,IElement[]> result = new HashMap<String,IElement[]>();
		Element[] nonNullChildren = (children != null) ? children : new Element[0];
		result.put(getName(), nonNullChildren);
		for (IElement c : nonNullChildren) {
			result.putAll(c.getSubTree());
		}
		return result;
	}
	
	protected HashMap<String,Object> getAttributes() {
		return attributes;
	}
	
	protected void setAttributes(HashMap<String,Object> attributes) {
		this.attributes = attributes;
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getSubTreeAttributes() throws PaloException, PaloJException {
		HashMap<String,HashMap<String,Object>> result = new HashMap<String,HashMap<String, Object>>();
		HashMap<String,Object> nonNullAttributes = (attributes != null) ? attributes : new HashMap<String,Object>(0,1);
		result.put(getName(), nonNullAttributes);
		if (children != null) for (Element c : children) {
			result.putAll(c.getSubTreeAttributes());
		}
		return result;
	}

	// returns the base element type independent of consolidation
	public ElementType getValue() {
		return (super.getValue() != null) ? super.getValue() : ElementType.ELEMENT_NUMERIC;
	}

	@Override
	public ElementType getType() {
		if (getChildCount() > 0) return ElementType.ELEMENT_CONSOLIDATED;
		return getValue();
	}

	@Override
	public double getWeight(IElement parent) throws PaloException, PaloJException {
		if (weights != null) {
			Double result = weights.get(parent);
			//NOTE: for memory efficiency we store only non 1 weights. This requires that the given element MUST be a parent to obtain correct results.
			return (result != null) ? result : 1;
		}
		return 1;
	}
	
	protected Map<IElement,Double> getWeights() {
		return (weights != null) ? weights : new HashMap<IElement,Double>();
	}
	
	protected void setWeights(Map<IElement,Double> weights) {
		this.weights = weights;
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		setName(name);
	}
	
	public Element clone() {
		Element e = new Element(getName(),getType());
		e.attributes = attributes;
		e.children = children;
		e.parents = parents;
		e.weights = weights;
		return e;
	}

}
