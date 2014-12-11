/**
 * 
 */
package com.jedox.palojlib.interfaces;


/**
 * represent a consolidation relationship between two elements in a dimension
 * @author khaddadin
 *
 */
public interface IConsolidation {
	
	/**
	 * get the parent element
	 * @return the parent
	 */
	public IElement getParent();
	
	/**
	 * get the child element
	 * @return the child element
	 */
	public IElement getChild();
	
	/**
	 * get the weight of the child to this parent
	 * @return weight value
	 */
	public double getWeight();

}
