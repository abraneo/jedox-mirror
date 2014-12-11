package com.jedox.etl.core.node.tree;

import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IElement;

public class UniqueConsolidation extends Consolidation {

	public UniqueConsolidation(IElement parent, IElement child, double weight) {
		super(parent, child, weight);
	}
	
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (getChild() == null ? 0 : getChild().getName().hashCode());
		return result;
	}
	
	public boolean equals(Object obj) {
		if (obj instanceof IConsolidation) {
			IConsolidation c = (IConsolidation)obj;
			return equals(getChild(),c.getChild());
		}
		return false;
	}

}
