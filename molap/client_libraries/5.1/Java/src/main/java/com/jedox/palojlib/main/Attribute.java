/**
 *
 */


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
 *	Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.main;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class Attribute implements IAttribute{

	private String name;
	private final int id;
	private final int dimensionId;
	private final int databaseId;
	private final ElementType type;
	private final ElementHandler elementhandler;

	protected Attribute(String contextId,int id,int databaseId,int dimensionId,String name, ElementType elementType) throws PaloException, PaloJException{
		elementhandler = new ElementHandler(contextId);
		this.name = name;
		this.id = id;
		this.type = elementType;
		this.dimensionId = dimensionId;
		this.databaseId = databaseId;
	}

	/************************** public method from the interface *****************************/

	public String getName() {
		return name;
	}

	public ElementType getType() {
		return type;
	}
	
	public void rename(String name) throws PaloException, PaloJException{
		elementhandler.rename(databaseId,dimensionId,id,name);
		this.name = name;
		
	}

	/*********************************************************************************/

	protected int getId() {
		return id;
	}

}
