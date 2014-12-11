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

import com.jedox.palojlib.interfaces.IDimensionInfo;

public class DimensionInfo extends ComponentInfo implements IDimensionInfo {

	private final int maximumLevel;
	private final int maximumIndent;
	private final int maximumDepth;
	private final int attributeDimensionId;
	private final int attributeCubeId;
	private final int numberOfElements;

	protected DimensionInfo(String attributeDimensionId,String attributeCubeId,String maximumLevel,String maximumIndent,String maximumDepth,String token, String numberOfElements){
		
		this.attributeDimensionId = Integer.parseInt(attributeDimensionId.equals("")?"-1":attributeDimensionId);
		this.attributeCubeId = Integer.parseInt(attributeCubeId.equals("")?"-1":attributeCubeId);
		this.maximumLevel = Integer.parseInt(maximumLevel);
		this.maximumIndent = Integer.parseInt(maximumIndent);
		this.maximumDepth = Integer.parseInt(maximumDepth);
		this.numberOfElements = Integer.parseInt(numberOfElements);
		super.token = Integer.parseInt(token);
	}
	
	public int getAttributeDimensionId() {
		return attributeDimensionId;
	}
	
	public int getAttributeCubeId() {
		return attributeCubeId;
	}

	public int getMaximumLevel() {
		return maximumLevel;
	}

	public int getMaximumIndent() {
		return maximumIndent;
	}

	public int getMaximumDepth() {
		return maximumDepth;
	}
	
	public int getNumberOfElements() {
		return numberOfElements;
	}
}
