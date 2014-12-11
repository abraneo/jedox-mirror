package com.jedox.etl.core.node;

public class NamedValue<T extends Object> implements INamedValue<T> {
	
	private T value = null;
	private String name = null;
	
	public NamedValue() {
	}
	
	public NamedValue(String name) {
		this.name = name;
	}
	
	public NamedValue(String name, T value) {
		this.name = name;
		this.value = value;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name.trim();
	}

	@Override
	public T getValue() {
		return value;
	}
	
	public void setValue(T value) {
		this.value = value;
	}
	
	public final String getValueAsString() {
		T value = getValue();
		if (value == null)
			return "";
		if (value instanceof Number) {
			String v = value.toString().trim();
			if (v.endsWith(".0"))//sometimes it is added to numeric values
				v = v.substring(0, v.length()-2);
			return v;
		}
		String trimmedValueAsString = value.toString().trim();
		if (trimmedValueAsString.isEmpty() && !value.toString().isEmpty())
			return " ";
		else
			return trimmedValueAsString;
	}
	
	public String toString() {
		return name;
	}
}
