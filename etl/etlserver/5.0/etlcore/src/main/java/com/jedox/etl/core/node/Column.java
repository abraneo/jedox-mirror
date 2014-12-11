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
package com.jedox.etl.core.node;

import com.jedox.palojlib.interfaces.IElement.ElementType;

/**
 * General base implementation class of the {@link IColumn} interface
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Column extends NamedValue<Object> implements IColumn {
	private String type = null;
	private String defaultValue = null;
	private ElementType elementType;
	private ColumnTypes columnType = ColumnTypes.data;
	private int scale=0;

	public Column() {
	}

	public Column(String name) {
		super(name);
	}


	public void setValueType(String type) {
		this.type = type;
	}

	/**
	 * gets the Value Type if set
	 * @return the value type or NULL if no explicit Value Type is set.
	 */
	protected String getExplicitValueType() {
		return type;
	}

	public String getValueType() {
		if (type == null) return String.class.getCanonicalName();
		return type;
	}

	public Object getValue() {
		Object value = super.getValue();
		if ((value == null) || value.toString().trim().isEmpty()) {
			if (getDefaultValue() == null) {
				return value;
			}
			return getDefaultValue();
		}
		return value;
	}

	
	public void setDefaultValue (String defaultValue) {
		this.defaultValue = defaultValue;
	}

	public String getDefaultValue() {
		return defaultValue;
	}

	public boolean isEmpty() {
		return super.getValue() == null;
	}

	public ColumnTypes getColumnType() {
		return columnType;
	}

	public void setColumnType(ColumnTypes columnType) {
		this.columnType = columnType;
	}

	public ElementType getElementType() {
		return (elementType != null) ? elementType : ElementType.ELEMENT_NUMERIC;
	}

	public ElementType getElementType(ElementType defaulttype) {
		return (elementType != null) ? elementType : defaulttype;
	}

	protected ElementType getElementTypeInternal() {
		return elementType;
	}
	
	public void setElementType(String type) {
		if (type != null) {
			try {
				this.elementType = ElementType.valueOf(type.toUpperCase());
			}
			catch (IllegalArgumentException e) {
				this.elementType = ElementType.ELEMENT_NUMERIC;
			}
		}
	}

	public void mimic(IColumn source) {
		setName(source.getName());
		setDefaultValue(source.getDefaultValue());
		setColumnType(source.getColumnType());
		setValueType(source.getValueType());
		setElementType(source.getElementType().toString());
		setScale(source.getScale());
	}
	
	public int getScale() {
		return scale;
	}
	
	public void setScale(int scale) {
		this.scale=scale;	
	}


}
