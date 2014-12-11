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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.connection;

import java.util.Properties;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;

/**
 * Interface which unifies the access to the different types of possible connections like relational databases, olap systems, files, etc.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public interface IConnection extends IComponent {
	
	/**
	 * Holds the available fetch modes. Not every type of connection does support each mode. Even not every relational database does support each mode. So the mode always servers as a hint, but it cannot be guaranteed, that this mode is actually used. 
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum FetchModes {
		/**
		 * All data is fetched in a single attempt, which is usually fast, but may require huge amounts of memory.
		 */
		FULL, 
		/**
		 * Data is fetched in small chunks, which is usually slower, but only leafs a small memory footprint. Recommended for large data-set.
		 */
		BUFFERED
	}
	
	/**
	 * Possible types of proxy servers. 
	 * @author Kais Haddadin. Mail: kais.haddadin@jedox.com
	 *
	 */
	public static enum ProxyTypes {
		
		HTTP,
		SOCKS
	}
	
	

	/**
	 * Opens the underlying connection.
	 * @return The Connection Object depending on the specific type of connection implemented.
	 */
	public Object open() throws RuntimeException;
	/**
	 * Closes the underlying connection.
	 */
	public void close();
	
	/**
	 * Sets the driver responsible for handling the underlying connection.
	 * @param driver the driver classname in full qualified format.
	 */
	public void setDriver(String driver);
	
	/**
	 * Sets the protocol responsible for handling the underlying connection. written by Kais
	 * @param jdbc the jdbc name, exmaple: for connection oracle jdbc=oracle:thin
	 */
	public void setProtocol(String jdbc);

	/**
	 * Gets the encoding of the underlying connection. If this encoding if different to UTF8 all data read or written via this connection is converted. ETLServer internally uses only UTF8.
	 * @return the encoding. See  <a href="http://java.sun.com/j2se/1.5.0/docs/guide/intl/encoding.doc.html">Supported Encodings</a>
	 */
	public String getEncoding();
	/**
	 * Gets the name of the database accessibly via this connection. Dependent to to type of the underlying connection, this corresponds to different legacy concepts, but always means an entity, where the data is actually held.   
	 * @return the name of the database
	 */
	public String getDatabase();
	/**
	 * Gets the name of the server, which actually handles the connection of a given type. Implementation is optional.
	 * @return the server name.
	 */
	public String getServerName();
	
	/**
	 * gets metadata from this connection 
	 * @param properties the properties specifying which metadata. This is implementation specific.
	 * @return the metadata as string
	 */
	public String getMetadata(Properties properties) throws RuntimeException;

	/**
	 * gets the all selectors with its filters which are available as metadata for this connection 
	 * @return a list of Metadata criterias for each selector
	 */
	public MetadataCriteria[] getMetadataCriterias();
	
	/**
	 * get the Metadata criteria for a specific name 
	 * @param name name of the selector
	 * @return the Metadata criterias for the selector
	 */
	public MetadataCriteria getMetadataCriteria(String name);	
	
	public void keep(boolean keepOpen);
	
}
