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

public class ColumnNodeFactory {
	
	private static final ColumnNodeFactory instance = new ColumnNodeFactory();
	
	public static final ColumnNodeFactory getInstance() {
		return instance;
	}
	
	
	public AttributeNode createAttributeNode(Attribute definition, IColumn input) {
		AttributeNode node = new AttributeNode(definition.getName());
		node.setInput(input);
		node.setDefinition(definition);
		return node;
	}
	
	public LevelNode createLevelNode(String name, IColumn input) {
		LevelNode node = new LevelNode(name); 
		node.setInput(input);
		return node;
	}
	
	public CoordinateNode createCoordinateNode(String name, IColumn input) {
		CoordinateNode node = new CoordinateNode(name);
		node.setInput(input);
		return node;
	}
	
	public ValueNode createValueNode(String target, String name, IColumn input) {
		ValueNode node = new ValueNode(name,target);
		node.setInput(input);
		return node;
	}
	
	public RelationalNode createRelationalNode(String name, IColumn input) {
		RelationalNode node = new RelationalNode(name);
		node.setInput(input);
		return node;
	}

}
