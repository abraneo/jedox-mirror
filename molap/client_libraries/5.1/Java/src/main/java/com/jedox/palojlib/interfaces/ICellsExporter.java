/**
 * 
 */
package com.jedox.palojlib.interfaces;

import com.jedox.palojlib.exceptions.PaloException;

/**
 * represent a cells exporter that export cells from a cube
 * @author khaddadin
 *
 */
public interface ICellsExporter {
	
	/**
	 * get the next cell in the exported cells list
	 * @return next cell in queue
	 * @throws PaloException
	 */
	public ICell next() throws PaloException;
	
	/**
	 * check whether there is still cells to be read
	 * @return true if there is a cell to be read, false otherwise.
	 * @throws PaloException 
	 */
	public boolean hasNext() throws PaloException;
	
	/**
	 * get the dimensions specified in the cells. 
	 * In palojlib implementation, it is always the cube dimensions.
	 * This is important for the MDX implementation
	 * @return dimensions specified in the cells
	 */
	public IDimension[] getDimensions();

}
