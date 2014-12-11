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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.util;

public class NamingUtil {

	public static String escapeValue(String value) {
		//KAIS: return the value between single quotes and replace all single quotes with \'
		return ("'"+value.replaceAll("\'", "\'\'")+"'");
	}

	public static String escape(String name, String quote) {
		return (quote+name.replaceAll(quote, "")+quote);
	}

	public static String escape(String name) {
		return escape(name,"\"");
	}

	public static String internal(String name) {
		return (internalPrefix()+name);
	}

	public static String internalPrefix() {
		return "#";
	}
	
	public static String hiddenInternalPrefix() {
		return "_#";
	}

	public static String internalKeyName() {
		return internalPrefix()+"id";
	}
	
	public static String internalHibernateKeyName() {
		return internalPrefix()+"hid";
	}

	public static String internalDatastoreName() {
		return internalPrefix()+"data";
	}

	public static String spaceValue() {
		return internalPrefix()+"space";
	}
	
	public static String skipColumn() {
		return internalPrefix()+"skipColumn";
	}	

	public static String defaultValue() {
		return internalPrefix()+"default";
	}		
	
/*	
	public static String AllValue() {
		return "*";
	}
*/	

	public static String reduceLength(String str){
		return str.substring(0, Math.min(str.length(), 500));
	}

	 public static String removeAxis2IllegalCharacters(String orig)
	 {
		 if (orig == null)
		 {
			 return "";
		 }

		 char[] chars = orig.toCharArray();

		 StringBuffer strBuf = new StringBuffer();
		 for (int i = 0; i < chars.length; i++)
		 {
			 if (chars[i] != 0){
				 strBuf.append(chars[i]);
			 }
		 }
		 return strBuf.toString();
	 }

	public static String removeIllegalWhiteSpaces(String line){
        if(line == null)
            return "";

        char[] messageChars = line.toCharArray();
        StringBuilder editedMessage = new StringBuilder();
        String allowedChars = "^\"�!�$%&/\\\n*-(){}[]<>@�=?#':;,|\t ";
        for(Character c:messageChars){
            if(Character.isLetterOrDigit(c) || allowedChars.indexOf(c)!= -1){//is letter should allow ��� or other Uni-code chars
                editedMessage.append(c);
            }
        }
	        
        return editedMessage.toString();
    }
	 
	 
	 private static String getGeneratedElementNamePrefix() {
		 return ":";
	 }

	 public static String getElementnameParent() {
		 return getGeneratedElementNamePrefix() + "parent";
	 }

	 public static String getElementnameChild() {
		 return getGeneratedElementNamePrefix() + "child";
	 }

	 public static String getElementnameElement() {
		 return getGeneratedElementNamePrefix() + "element";
	 }

	 public static String getElementnameWeight() {
		 return getGeneratedElementNamePrefix() + "weight";
	 }

	 public static String getElementnameType() {
		 return getGeneratedElementNamePrefix() + "type";
	 }
	 
	 public static String getElementnameLevel() {
		 return getGeneratedElementNamePrefix() + "level";
	 }
	 
	 public static String getElementnameNodeType() {
		 return getGeneratedElementNamePrefix() + "nodetype";
	 }

	public static String project_prefix = "p";
	public static String group_prefix = "g";
	public static String group_manager = "#_#group#_#";
	public static String username = "username";
	public static String password = "password";
	public static String session = "session";
	public static String etlsession = "etlsession";
	



}
