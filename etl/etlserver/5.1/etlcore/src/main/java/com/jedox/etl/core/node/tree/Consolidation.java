package com.jedox.etl.core.node.tree;

import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IElement;

public class Consolidation implements IConsolidation {
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
	
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (parent == null ? 0 : parent.getName().hashCode());
		result = prime * result + (child == null ? 0 : child.getName().hashCode());
		return result;
	}
	
	protected boolean equals(IElement source, IElement target) {
		if (source == null && target==null) return true;
		if (source != null && target == null) return false;
		if (source == null && target != null) return false;
		return source.getName().equals(target.getName());
	}
	
	public boolean equals(Object obj) {
		if (obj instanceof IConsolidation) {
			IConsolidation c = (IConsolidation)obj;
			return equals(parent,c.getParent()) && equals(child,c.getChild());
		}
		return false;
	}
	
}
