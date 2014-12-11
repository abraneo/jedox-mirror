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
 *   You may obtain a copy of the License at
*
 *   If you are developing and distributing open source applications under the
 *   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 *   ISVs, and VARs who distribute Palo with their products, and do not license
 *   and distribute their source code under the GPL, Jedox provides a flexible
 *   OEM Commercial License.
 *
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.palojlib.main;

import java.util.Arrays;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.IDimension;

/**
 * 
 * @author khaddadin
 * 
 */
public class Cell implements ICell {

	private final int[] path;
	private final Object value;
	private final CellType type;
	private final IDimension[] dimensions;
	private final String[] pathNames;
	
	public Cell(int[] path, Object value, CellType type, IDimension[] dimensions,String[] pathNames) {
		
		this.path = path;
		this.value = value;
		this.type = type;
		this.dimensions = dimensions;
		this.pathNames = pathNames;
	}

	public int[] getPathIds() {
		return path;
	}

	public String getPathIdsAsString() {
		return Arrays.toString(path).substring(1, Arrays.toString(path).length()-1).replaceAll(" ","");
	}

	public String[] getPathNames() throws PaloException, PaloJException {
		if(pathNames!=null)
			return pathNames;
		
		String[] pathNames = new String[path.length];
		for (int i = 0; i < path.length; i++)
			pathNames[i] = ((Dimension)dimensions[i]).getElementById(path[i]).getName();

		return pathNames;
	}
	
	public String getPathNameAt(int index) throws PaloException, PaloJException {
		if(pathNames!=null)
			return pathNames[index];
		
		return	((Dimension)dimensions[index]).getElementById(path[index]).getName();

	}

	private Object getDefaultValue() {
		if (type.equals(CellType.CELL_NUMERIC))
			return new Double(0);
		return "";
	}

	public Object getValue() {
		try {
			if (type.equals(CellType.CELL_NUMERIC))
				return new Double(value.toString());
			return value;
		} catch (Exception e) {
			return getDefaultValue();
		}
	}

	public CellType getType() {
		return type;
	}

}
