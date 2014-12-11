package com.jedox.etl.core.node.xml;

import com.jedox.etl.core.node.Column;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class XMLAttribute extends Column implements IAttribute {

	@Override
	public ElementType getType() {
		return ElementType.ELEMENT_STRING;
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		setName(name);
	}

}
