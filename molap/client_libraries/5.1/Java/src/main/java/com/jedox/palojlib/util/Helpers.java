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

package com.jedox.palojlib.util;

import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.net.URLEncoder;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import org.apache.log4j.Logger;

import com.jedox.palojlib.managers.LoggerManager;

public final class Helpers {

	private static Logger log = LoggerManager.getInstance().getLogger(Helpers.class.getSimpleName());

	public static String encrpytPassword(String password){
		String hashword = null;
		try {
			MessageDigest md5 = MessageDigest.getInstance("MD5");
			md5.update(password.getBytes());
			BigInteger hash = new BigInteger(1, md5.digest());
			hashword = hash.toString(16);
			int old_length = hashword.length();
			
			for(int i=old_length-1;i<31;i++){
				hashword = "0" + hashword;
			}
		} catch (NoSuchAlgorithmException nsae) {
		// ignore
		}
		return hashword;
	}

	public static String getConcatFromArray(int[] path){

		StringBuilder pathAsString = new StringBuilder();		
		for(int j=0;j<path.length;j++){
			pathAsString.append(path[j] + ",");
		}
		pathAsString.deleteCharAt(pathAsString.length() - 1);
		return pathAsString.toString();
	}

	public static String addDoubleQuotes(String value){
		return "\"" +  value.replaceAll("\"", "\"\"") + "\"";
	}
	
	public static String urlEncode(String value){

		try {
			value = URLEncoder.encode(value, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			log.debug("Error while encoding the value " + value);
		}

		return value;
	}

}
