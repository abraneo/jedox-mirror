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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.node;

import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;


public class AttributeNode extends ColumnNode {

	private Attribute definition;
	
	protected AttributeNode() {
		super();
	}
	
	protected AttributeNode(String name) {
		super(name);
	}

	public void setDefinition(Attribute definition) {
		this.definition = definition;
		setValueType(ElementType.ELEMENT_STRING.equals(definition.getType()) ? String.class : Double.class);
	}

	public Attribute getDefinition() {
		return definition;
	}
	
	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof AttributeNode) {
			AttributeNode cn = (AttributeNode)source;			
			this.definition = cn.definition;
		}
	}

}
