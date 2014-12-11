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
package com.jedox.etl.core.node;

import com.jedox.etl.core.aliases.IAliasElement;

/**
 * General base implementation class of the {@link IColumn} interface
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Column extends NamedValue<Object> implements IColumn {
	private Class<?> type = null;
	private IAliasElement aliasElement = null;

	public Column() {
	}

	public Column(String name) {
		super(name);
	}


	public void setValueType(Class<?> type) {
		this.type = type;
	}

	/**
	 * gets the Value Type if set
	 * @return the value type or NULL if no explicit Value Type is set.
	 */
	protected Class<?> getExplicitValueType() {
		return type;
	}

	public Class<?> getValueType() {
		if (type == null) return String.class;
		return type;
	}

	public boolean isEmpty() {
		return super.getValue() == null;
	}

	public void mimic(IColumn source) {
		setName(source.getName());
		setValueType(source.getValueType());
		this.aliasElement = source.getAliasElement();	
	}

	public void setAliasElement(IAliasElement aliasElement) {
		this.aliasElement = aliasElement;
		if (aliasElement != null && aliasElement.hasName()) {
			setName(aliasElement.getName());
		}
	}

	public IAliasElement getAliasElement() {
		return aliasElement;
	}
	
	public Object getValue() {
		Object value = super.getValue();
		if (aliasElement != null && ((value == null) || value.toString().trim().isEmpty())) {
			if (aliasElement.getDefaultValue() == null) {
				return value;
			}
			return aliasElement.getDefaultValue();
		}
		return value;
	}
	
	


}
