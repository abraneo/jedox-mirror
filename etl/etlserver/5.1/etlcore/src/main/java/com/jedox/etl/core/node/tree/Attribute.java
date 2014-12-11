package com.jedox.etl.core.node.tree;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class Attribute implements IAttribute {
	
	public static enum AttributeModes {
		ATTRIBUTE, ALIAS
	}
	
	private AttributeModes mode;
	private ElementType type;
	private String name;
	
	public Attribute() {
		super();
	}
	
	public Attribute(String name) {
		super();
		setName(name);
	}
	
	public Attribute(String name, ElementType type) {
		super();
		setName(name);
		setType(type);
	}
	
	public Attribute(IAttribute attribute) {
		super();
		setName(attribute.getName());
		setType(attribute.getType());
	}

	@Override
	public ElementType getType() {
		return (type != null) ? type : ElementType.ELEMENT_STRING;
	}
	
	public void setType(ElementType type) {
		this.type = type;
	}

	@Override
	public void rename(String name) throws PaloException, PaloJException {
		setName(name);
	}

	public void setMode(AttributeModes mode) {
		this.mode = mode;
	}

	public AttributeModes getMode() {
		return (mode != null) ? mode : AttributeModes.ATTRIBUTE; 
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getName() {
		return name;
	}
	
	

}
