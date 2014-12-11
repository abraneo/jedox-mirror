/**
*   @brief <Description of Class>
*  
*   @file
*  
*   Copyright (C) 2008-2013 Jedox AG
*  
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License (Version 2) as published
*   by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
*  
*   This program is distributed in the hope that it will be useful, but WITHOUT
*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*   more details.
*  
*   You should have received a copy of the GNU General Public License along with
*   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
*   Place, Suite 330, Boston, MA 02111-1307 USA
*
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs, 
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible  
*   OEM Commercial License.
*  
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right 
*   (commercial copyright) has Jedox AG, Freiburg.
*  
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.source;

import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnectable;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * Interface for all Sources to implement. 
 * A Source holds subset of data from a back-end (e.g. a relational database, a file, etc.), which is processed by a {@link com.jedox.etl.core.source.processor.IProcessor Processor}. 
 * A Source typically specifies some sort of selection criteria in relation to back-end expressed by a query.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface ISource extends IComponent, IConnectable, IExecutable {
	
	public enum CacheTypes {
		none, memory, disk
	}
	
	
	/**
	 * Gets the AliasMap containing aliases for this source.
	 */
	public IAliasMap getAliasMap();
	
	
	/**
	 * Sets the AliasMap for this Source. Basically an AliasMap is a mapping of names to column numbers to be addressed by this names.   
	 * @param map the AliasMap.
	 */
	public void setAliasMap(IAliasMap map);
	
	
	/**
	 * Gets the Processor of this Source. The Processor processes all lines of data from this Source Row by Row in a unified manner.
	 * @return the Processor giving access to the data held by the source.
	 */
	public IProcessor getProcessor() throws RuntimeException;
	
	/**
	 * Gets the Processor of this Source. The Processor processes 'size' lines of data from this Source Row by Row in a unified manner.
	 * @param size the number of lines to be processed.
	 * @return the Processor giving access to the data held by the source.
	 */
	public IProcessor getProcessor(int size) throws RuntimeException;
	
	/**
	 * Determines if this source is cached internally or if the data is directly retrieved for an external back-end. 
	 * @return true if it is cached, false otherwise
	 */
	public boolean isCached();
	
	/**
	 * Overrides the default caching approach of this source.
	 * @param isCached true if this Source should be cached. false if this Source should not be cached. 
	 */
	public void setCaching(CacheTypes cacheType);
	
	/**
	 * Gets the format description of a Source. 
	 * @return the format description.
	 */
	public IView.Views getFormat();
	
	/**
	 * Deletes all internal caches, so that all data is rebuilt on the next request.
	 */
	public void invalidate();
	
	public String getPersistentView();
	
	public CacheTypes getCacheType();
	
	public void clearCache();
}
