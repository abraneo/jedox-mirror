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
package com.jedox.etl.core.node.tree;

import java.util.HashMap;
import java.util.Map;

import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;

public class TreeColumn extends Column implements IElement {
	
	private IElement element;
	
	public TreeColumn() {
		super("Element");
	}
	
	public void setElement(IElement element) {
		this.element = element;
		setValue(element.getName());
	}

	@Override
	public Object getAttributeValue(String arg0) throws PaloJException,
			PaloException {
		return element.getAttributeValue(arg0);
	}

	@Override
	public int getChildCount() throws PaloException, PaloJException {
		return element.getChildCount();
	}

	@Override
	public IElement[] getChildren() throws PaloException, PaloJException {
		return element.getChildren();
	}

	@Override
	public int getParentCount() throws PaloException, PaloJException {
		return element.getParentCount();
	}

	@Override
	public IElement[] getParents() throws PaloException, PaloJException {
		return element.getParents();
	}

	@Override
	public ElementPermission getPermission() {
		return element.getPermission();
	}

	@Override
	public int getPosition() {
		return element.getPosition();
	}

	@Override
	public Map<String, IElement[]> getSubTree() throws PaloException,
			PaloJException {
		return element.getSubTree();
	}

	@Override
	public Map<String, HashMap<String, Object>> getSubTreeAttributes()
			throws PaloException, PaloJException {
		return element.getSubTreeAttributes();
	}

	@Override
	public ElementType getType() {
		return element.getType();
	}

	@Override
	public double getWeight(IElement arg0) throws PaloException, PaloJException {
		return element.getWeight(arg0);
	}

	@Override
	public void move(int arg0) throws PaloException, PaloJException {
		element.move(arg0);
	}

	@Override
	public void rename(String arg0) throws PaloException, PaloJException {
		element.rename(arg0);
	}
	
	public void mimic(IColumn source) {
		super.mimic(source);
		if (source instanceof TreeColumn) {
			TreeColumn n = (TreeColumn)source;
			this.element = n.element;
		}
	}

}
