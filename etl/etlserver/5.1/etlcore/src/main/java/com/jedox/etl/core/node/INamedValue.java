package com.jedox.etl.core.node;

public interface INamedValue<T extends Object> {
	
	/**
	 * Gets the name of this column.
	 * @return the name of this column
	 */
	public String getName();
	/**
	 * Set the name of this column
	 * @param name the name of this column
	 */
	public void setName(String name);
	/**
	 * Return the value of this column as an Object. How this value is calculated, is left to the implementation.
	 * @return the value of this column.
	 */
	public T getValue();
	/**
	 * Sets the value of this column externally.
	 * @param value the value of this column.
	 */
	public void setValue(T value);
	
	/**
	 * Return the value of this column as a String. Useful convenience method.
	 * @return the value of this column as String. If the value is null, null is returned.
	 */
	public String getValueAsString();

}
