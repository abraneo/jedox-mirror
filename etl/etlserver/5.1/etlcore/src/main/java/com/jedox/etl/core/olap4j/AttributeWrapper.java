package com.jedox.etl.core.olap4j;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;

import org.olap4j.metadata.Datatype;
import org.olap4j.metadata.Property;

public class AttributeWrapper implements IAttribute {

	private Property property;
	
	public AttributeWrapper(Property property) {
		this.property = property;
	}
	
	@Override
	public String getName() {
		return property.getName();
	}

	@Override
	public ElementType getType() {
		Datatype type = property.getDatatype();
		switch (type) {
		case ACCP: return ElementType.ELEMENT_STRING;
		case BOOLEAN: return ElementType.ELEMENT_STRING;
		case CHAR: return ElementType.ELEMENT_STRING;
		case CUKY: return ElementType.ELEMENT_STRING;
		case STRING: return ElementType.ELEMENT_STRING;
		case VARIANT: return ElementType.ELEMENT_STRING;
		case DATS: return ElementType.ELEMENT_STRING;
		case FLTP: return ElementType.ELEMENT_STRING;
		case LCHR: return ElementType.ELEMENT_STRING;
		case SSTR: return ElementType.ELEMENT_STRING;
		case STRG: return ElementType.ELEMENT_STRING;
		case TIMS: return ElementType.ELEMENT_STRING;
		case VARC: return ElementType.ELEMENT_STRING;
		case UNIT: return ElementType.ELEMENT_STRING;
		default: return ElementType.ELEMENT_NUMERIC;
		}
	}

	@Override
	public void rename(String newname) throws PaloException {
		throw new PaloException("Renaming attributes not supported by OLAP4j provider.");
	}
	
	public Property getProperty() {
		return property;
	}

}
