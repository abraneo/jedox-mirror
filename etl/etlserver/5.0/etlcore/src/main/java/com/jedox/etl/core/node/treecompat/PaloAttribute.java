package com.jedox.etl.core.node.treecompat;

import com.jedox.etl.core.node.Column;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class PaloAttribute extends Column implements IAttribute {
	
	public PaloAttribute() {
		super();
	}
	
	public PaloAttribute(String name) {
		super(name);
	}
	
	public PaloAttribute(IAttribute attribute) {
		super(attribute.getName());
		setColumnType(ColumnTypes.attribute);
		setElementType(attribute.getType().toString());
	}

	@Override
	public ElementType getType() {
		return getElementType();
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		setName(name);
	}

}
