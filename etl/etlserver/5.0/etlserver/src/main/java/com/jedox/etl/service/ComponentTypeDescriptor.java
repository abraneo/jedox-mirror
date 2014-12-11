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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.service;

/**
 * SOAP Transport class for component type meta data.
 * @author chris
 *
 */
public class ComponentTypeDescriptor {
	private String scope;
	private String type;
	private String classname;
	private String schema;
	private String caption;
	private boolean isTree;
	
	/**
	 * gets the scope this component type is defined in. Available scopes are "connections","extracts","transforms","loads","jobs"
	 * @return the scope
	 */
	public String getScope() {
		return scope;
	}
	/**
	 * sets the scope of this component type.
	 * @param scope
	 */
	public void setScope(String scope) {
		this.scope = scope;
	}
	
	/**
	 * gets the name of the component typ
	 * @return the type name
	 */
	public String getType() {
		return type;
	}
	
	/**
	 * sets the component type name.
	 * @param type
	 */
	public void setType(String type) {
		this.type = type;
	}
	/**
	 * gets the canonical class name, which implements this type.
	 * @return the class name
	 */
	public String getClassname() {
		return classname;
	}
	/**
	 * sets the canonical class name, which implements this type.
	 * @param classname
	 */
	public void setClassname(String classname) {
		this.classname = classname;
	}
	/**
	 * gets the schema document (xsd) for this type as string.
	 * @return the xsd as string
	 */
	public String getSchema() {
		return schema;
	}
	/**
	 * sets the schema document (xsd) for this type. 
	 * @param schema
	 */
	public void setSchema(String schema) {
		this.schema = schema;
	}
	/**
	 * gets a caption for this type, as how it should be displayed in an UI.
	 * @return the caption
	 */
	public String getCaption() {
		return caption;
	}
	/**
	 * sets a caption for this type.
	 * @param caption
	 */
	public void setCaption(String caption) {
		this.caption = caption;
	}
	/**
	 * Determines if this type has a tree data representation allowing multiple renderings.
	 * @return true, if component implements ITreeSource interface.
	 *
	 */
	public boolean isTree() {
		return isTree;
	}
	/**
	 * Defines if this type has a tree data representation allowing multiple renderings. 
	 * @param isTree
	 */
	public void setTree(boolean isTree) {
		this.isTree = isTree;
	}
	
	
}
