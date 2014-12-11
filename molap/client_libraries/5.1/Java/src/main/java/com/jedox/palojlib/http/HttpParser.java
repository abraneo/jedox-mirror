package com.jedox.palojlib.http;
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


import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.regex.Pattern;


/**
 * parses the OLAP server response, not the header. 
 * @author khaddadin
 *
 */
public class HttpParser {

	private static final Pattern lineEnd = Pattern.compile(";[\r\n]");

	public final String readLine(InputStream in) throws IOException {
        byte[] rawdata = readBytes(in);
        if (rawdata == null) {
            return "";
        }
        // strip CR and LF from the end
        int len = rawdata.length;
        int offset = 0;
        if (len > 0) {
            if (rawdata[len - 1] == '\n') {
                offset++;
                if (len > 1) {
                    if (rawdata[len - 2] == '\r') {
                        offset++;
                    }
                }
            }
        }
        try {
			return new String(rawdata, 0, len - offset, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			return new String(rawdata, 0, len - offset);
		}
	}

	public final byte[] readBytes(InputStream in) throws IOException {
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        int ch;
        while ((ch = in.read()) >= 0 && !Thread.currentThread().isInterrupted()) {
            buf.write(ch);
            if (ch == '\n') { // be tolerant (RFC-2616 Section 19.3)
                break;
            }
        }
        if (buf.size() == 0) {
            return null;
        }
        return buf.toByteArray();
	}

	public final String readRawLine(InputStream in) throws IOException {
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        int ch;
        int lastCh=-1;
        boolean inQuote = false;

        while ((ch = in.read()) >= 0 && !Thread.currentThread().isInterrupted()) {
            buf.write(ch);
            if (ch == '"') {
            	inQuote = !inQuote;
            }
            if(ch == '\n' && lastCh == ';' && !inQuote)
            	break;
            lastCh = ch;
        }
        if (buf.size() == 0) {
            return "";
        }
        try {
        	return new String(buf.toByteArray(),"UTF-8");
		} catch (UnsupportedEncodingException e) {
			return new String(buf.toByteArray());
		}
	}

	public final String[][] parse(String response, char delim) {

		String[] lines = lineEnd.split(response,0);
		String[][] res = new String[lines.length][];
		for(int i=0; i<lines.length;i++) {
			lines[i] = lines[i]+";";
			res[i] = parseLine(lines[i],delim);
		}
		return res;
	}

	/**
	 * parse one line into into array of strings
	 * @param str the line in response
	 * @param delim delimiter
	 * @return the parsed array
	 */
	public final String[] parseLine(String str,char delim) {
		ArrayList<String> entries = new ArrayList<String>();
		String[] possibleEntries = str.trim().split(";",-1);// maximum number of entries
		StringBuilder entry = new StringBuilder();

		for(String possibleEntry:possibleEntries){
			if(!possibleEntry.contains("\"") && entry.length()==0){// if it is the first one and no double quotes exists in it, then add it directly
				entries.add(possibleEntry);
			}else{
				// get the number of double quotes in the possible entry
				int numberOfDoubleQuotes = possibleEntry.replaceAll("[^\"]","").length();
				// if there is even number of quotes and it is the first
				if(numberOfDoubleQuotes%2 == 0 && entry.length()==0){
					// remove the quotes from the sides and remove only the duplicates quotes from the rest then add this to the final list
					possibleEntry = possibleEntry.substring(1, possibleEntry.length()-1).replaceAll("\"\"","\"");
					entries.add(possibleEntry);
				}else if(numberOfDoubleQuotes%2 == 0 && entry.length()!=0){
					entry.append(possibleEntry + ";");// an open quote is somewhere before, this should be completly added to the temp string
				}else if(numberOfDoubleQuotes%2 != 0 && entry.length()==0){
					entry.append(possibleEntry.substring(1, possibleEntry.length()) + ";");
				}else{
					entry.append(possibleEntry.substring(0, possibleEntry.length()-1));
					String entryStr = entry.toString().replaceAll("\"\"","\"");
					entries.add(entryStr);
					entry = new StringBuilder();
				}
			}
		}

		return (String[])entries.toArray(new String[entries.size()]);
	}

}
