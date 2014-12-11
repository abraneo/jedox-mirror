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
package com.jedox.etl.core.util;

import java.util.ArrayList;
import java.util.List;


public class SQLUtil {

	public static String getWhereCondition(List<String> names, List<String> values, List<String> operators, String junction) {
		StringBuffer buffer = new StringBuffer(" ");
		for (int i=0; i<names.size(); i++) {
			buffer.append(names.get(i));
			buffer.append(operators.get(i));
			buffer.append(values.get(i));
			if (i<names.size()-1) buffer.append(" "+junction+" ");
		}
		if (names.size() > 1) { //surround with parentheses
			buffer.insert(1, "(");
			buffer.append(")");
		}
		if (names.size() == 0)
			return "";
		return buffer.toString();
	}

	public static String getJunction(List<String> expressions, String junction) {
		StringBuffer buffer = new StringBuffer(" ");
		for (int i=0; i<expressions.size(); i++) {
			buffer.append(expressions.get(i));
			if (i<expressions.size()-1) buffer.append(" "+junction+" ");
		}
		if (expressions.size() > 1) { //surround with parentheses
			buffer.insert(1, "(");
			buffer.append(")");
		}
		if (expressions.size() == 0)
			return "";
		return buffer.toString();
	}

	public static String getWhereCondition(List<String> names, List<String> values, List<Integer> lengths) {
		List<String> expressions = new ArrayList<String>();
		int valueIndex = 0;
		for (int i = 0; i < names.size(); i++) {
			int length = lengths.get(i);
			int obslength = Math.abs(length);
			List<String> namesExploded = getConstants(names.get(i),obslength);
			if(obslength != 0 && obslength < 3){
				expressions.add((length<0?" NOT ":"")+getWhereCondition(namesExploded,values.subList(valueIndex,valueIndex+( obslength)),getConstants("=",namesExploded.size()),"OR"));
			}else if(obslength >= 3){
				expressions.add((length<0?" NOT ":"")+getWhereInCondition(names.get(i),values.subList(valueIndex,valueIndex+( obslength))));
			}else{}
			valueIndex += obslength;
		}
		return getJunction(expressions,"AND");
	}


	private static String getWhereInCondition(String name, List<String> values) {
		StringBuffer buffer = new StringBuffer(" ");
			buffer.append("(" + name);
			buffer.append(" IN ");
			buffer.append("(");
			int i=0;
			for(;i<(values.size()-1);i++){
				buffer.append(values.get(i) + ",");
			}
			buffer.append(values.get(i) + "))");

		return buffer.toString();
	}

	public static String getPrepearedWhereCondition(List<String> names) {
		List<String> operators = getConstants("=", names.size());
		List<String> values = getConstants("?",names.size());
		String result = getWhereCondition(names,values,operators,"AND");
		return result.isEmpty()? result : " where "+result;
	}

	public static String getPreparedUpdateCondition(List<String> names, List<Boolean> add) {
		StringBuffer buffer = new StringBuffer(" set ");
		for (int i=0; i<names.size(); i++) {
			buffer.append(names.get(i));
			buffer.append("=");
			//special case sum
			if (Boolean.TRUE.equals(add.get(i)))
				buffer.append(names.get(i)+"+");
			buffer.append("?");
			if (i<names.size()-1) buffer.append(" , ");
		}
		if (names.size() == 0)
			return "";
		return buffer.toString();
	}

	public static String getOrderPart(List<String> names, List<String> orders) {
		StringBuffer result = new StringBuffer(" ");
		if (names != null)
			for (int i=0; i<names.size(); i++) {
				String name = names.get(i);
				String order = orders.get(i);
				result.append(name);
				result.append(" ");
				result.append(order);
				result.append(",");
			}
		result.deleteCharAt(result.length()-1);
		return result.toString();
	}

	public static List<String> quoteValues(List<String> values) {
		List<String> result = new ArrayList<String>();
		for (String value : values) {
			result.add(NamingUtil.escapeValue(value));
		}
		return result;
	}

	public static String quoteName(String name, String quote) {
		return NamingUtil.escape(name, quote);
	}

	public static List<String> quoteNames(List<String> names, String quote) {
		List<String> result = new ArrayList<String>();
		for (String name : names) {
			result.add(quoteName(name, quote));
		}
		return result;
	}

	public static String aliasName(String name, String alias) {
		return name + " as " + alias;
	}

	public static List<String> aliasNames(List<String> names, List<String> aliases) {
		List<String> result = new ArrayList<String>();
		for (int i=0; i<names.size(); i++) {
			result.add(aliasName(names.get(i),aliases.get(i)));
		}
		return result;
	}

	public static String prefixName(String name, String prefix) {
		if (prefix != null && !prefix.isEmpty())
			return prefix+"."+name;
		else
			return name;
	}

	public static String prefixName(String name, String prefix, String quote) {
		return prefixName(SQLUtil.quoteName(name, quote),prefix);
	}

	public static List<String> prefixNames(List<String> names, String prefix) {
		return prefixNames(names,prefix,"");
	}

	public static List<String> prefixNames(List<String> names, String prefix, String quote) {
		List<String> result = new ArrayList<String>();
		for (String name : names) {
			result.add(prefixName(name,prefix,quote));
		}
		return result;
	}

	public static String enumNames(List<String> names) {
		StringBuffer result = new StringBuffer();
		for (int i=0; i<names.size();i++) {
			result.append(names.get(i)+",");
		}
		if (result.length() > 0)
			result.deleteCharAt(result.length()-1);
		return result.toString();
	}

	public static List<String> getConstants(String constant, int size) {
		List<String> result = new ArrayList<String>();
		for (int i = 0; i < size; i++) {
			result.add(constant);
		}
		return result;
	}

	public static String buildQuery(String table, String selects, String where, String groupby, String orderby) {
		StringBuffer buffer = new StringBuffer();
		buffer.append("select ");
		buffer.append(selects);
		buffer.append(" from ");
		buffer.append(table);
		if (!where.equalsIgnoreCase("")) {
			buffer.append(" where ");
			buffer.append(where);
		}
		if (!groupby.equalsIgnoreCase("")) {
			buffer.append(" group by ");
			buffer.append(groupby);
		}
		if (!orderby.equalsIgnoreCase("")) {
			buffer.append(" order by ");
			buffer.append(orderby);
		}
		return buffer.toString();
	}

}
