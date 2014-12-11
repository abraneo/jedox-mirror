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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.core.util;

import java.io.Writer;
import java.util.ArrayList;

import com.jedox.etl.core.writer.CSVWriter;

public class MetadataWriter extends CSVWriter {

	private final static String quote = "\"";
	private final static char delimiter = ';';
   
	/**
	 * parse one line into into array of strings
	 * method taken from HTTPParser in palojlib
	 * @param str the line in response
	 * @return
	 */	
	public static String[] parseLine(String str) {
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
    
    
	public static char getDelimiter() {
		return delimiter;
	}
	
	public static String getQuote() {
		return quote;
	}	
        
	public MetadataWriter(Writer out) {
		super(out);
		setQuote(quote);
		setDelimiter(delimiter);
	}
	
}