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

import java.io.UnsupportedEncodingException;
/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Recoder {
	
	public final static String internalCoding = "UTF8";
	private boolean recoding = false;
	private String sourceEncoding = internalCoding;
	private String targetEncoding = internalCoding;
	
	public static boolean needsRecoding(String sourceEncoding, String targetEncoding) {
		if (sourceEncoding == null) sourceEncoding = internalCoding;
		if (targetEncoding == null) targetEncoding = internalCoding;
		return !sourceEncoding.equalsIgnoreCase(targetEncoding);
	}

	public void setRecoding(String sourceEncoding, String targetEncoding) {
		if (sourceEncoding != null) this.sourceEncoding = sourceEncoding;
		if (targetEncoding != null) this.targetEncoding = targetEncoding;
		recoding = !this.sourceEncoding.equalsIgnoreCase(this.targetEncoding);
	}
	
	public boolean isRecoding() {
		return recoding;
	}
	
	private String doRecode(Object s) {
		StringBuffer data = new StringBuffer("");
		try {
			//Compensate for Character Encoding
			data.append(new String(s.toString().getBytes(sourceEncoding), targetEncoding));
		} catch (UnsupportedEncodingException e) {return s.toString();};
		return data.toString();
	}
	
	public String recode(String s) {
		if ((!recoding) || (s==null)) 
			return s;
		return doRecode(s);
	}
}
