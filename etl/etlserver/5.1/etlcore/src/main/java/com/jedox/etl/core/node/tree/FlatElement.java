package com.jedox.etl.core.node.tree;

import java.util.HashMap;
import java.util.Map;

import com.jedox.etl.core.node.NamedValue;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class FlatElement extends NamedValue<ElementType> implements IElement, Cloneable {
	
	private HashMap<String,Object> attributes;
	
	protected FlatElement(String name, ElementType type) {
		setName(name);
		setValue(type);
	}

	@Override
	public Object getAttributeValue(String attributeName) throws PaloJException, PaloException {
		return (attributes != null) ? attributes.get(attributeName) : null;
	}

	@Override
	public ElementType getType() {
		return getValue();
	}
	
	@Override
	public void rename(String name) throws PaloException, PaloJException {
		setName(name);
	}
	
	public FlatElement clone() {
		FlatElement e = new FlatElement(getName(),getType());
		e.attributes = attributes;
		return e;
	}
	
	protected HashMap<String,Object> getAttributes() {
		return attributes;
	}
	
	protected void setAttributes(HashMap<String,Object> attributes) {
		this.attributes = attributes;
	}

	@Override
	public int getChildCount() throws PaloException, PaloJException {
		return 0;
	}

	@Override
	public IElement[] getChildren() throws PaloException, PaloJException {
		return null;
	}

	@Override
	public int getParentCount() throws PaloException, PaloJException {
		return 0;
	}

	@Override
	public IElement[] getParents() throws PaloException, PaloJException {
		return null;
	}

	@Override
	public ElementPermission getPermission() {
		return null;
	}

	@Override
	public int getPosition() {
		return 0;
	}

	@Override
	public Map<String, IElement[]> getSubTree() throws PaloException,
			PaloJException {
		return null;
	}

	@Override
	public Map<String, HashMap<String, Object>> getSubTreeAttributes()
			throws PaloException, PaloJException {
		return null;
	}

	@Override
	public double getWeight(IElement arg0) throws PaloException, PaloJException {
		return 1;
	}

	@Override
	public void move(int arg0) throws PaloException, PaloJException {
			
	}

}
