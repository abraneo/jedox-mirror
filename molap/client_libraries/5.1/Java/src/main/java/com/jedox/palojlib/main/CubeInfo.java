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

import java.math.BigDecimal;
import java.math.BigInteger;

import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.ICube.CubeType;


public class CubeInfo extends ComponentInfo {

	private final CubeType type;
	private final long CB_token;
	private long CC_token;
	private final BigInteger numberOfFilledCells;
	private final BigInteger numberOfCells;
	private int[] dimensionIds;


	protected CubeInfo(int[] dimensionIds,String numberOfCells,String numberOfFilledCells,String type, String cb_token,String cc_token){

		this.dimensionIds = dimensionIds;
		this.numberOfCells = parseNumber(numberOfCells);
		this.numberOfFilledCells = parseNumber(numberOfFilledCells);

		if(type.equals("0")) this.type =CubeType.CUBE_NORMAL;
		else if(type.equals("1"))  this.type =CubeType.CUBE_SYSTEM;
		else if(type.equals("2"))  this.type =CubeType.CUBE_ATTRIBUTE;
		else if(type.equals("3"))  this.type =CubeType.CUBE_USERINFO;
		else this.type = CubeType.CUBE_GPU;

		this.CB_token = Long.parseLong(cb_token);
		this.CC_token = Long.parseLong(cc_token);

	}
	
	// number can be in Exponential representation which cannot be directly converted into BigInteger
	private BigInteger parseNumber(String number) {
		Double d = new Double(number);
		BigDecimal in = new BigDecimal(d);
		return in.toBigInteger();
	}
	
	protected int getToken(){
		throw new PaloJException("Not yet implemented.");
	}

	public CubeType getType() {
		return type;
	}


	public long getCB_Token() {
		return CB_token;
	}
	
	public long getCC_Token() {
		return CC_token;
	}


	public BigInteger getNumberOfFilledCells() {
		return numberOfFilledCells;
	}


	public BigInteger getNumberOfCells() {
		return numberOfCells;
	}

	public int[] getDimensionIds() {
		return dimensionIds;
	}


}
