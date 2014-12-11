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
package com.jedox.etl.core.writer;

import java.io.BufferedReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import com.jedox.etl.core.component.RuntimeException;

public abstract class BaseWriter implements IFileWriter {

	protected PrintWriter writer;
	
	protected String defaultValue = "";
	protected int linesOut = 0;
	protected boolean autoclose = true;
		
	public BaseWriter(OutputStream out) {
		autoclose = !out.equals(System.out);
		writer = new PrintWriter(out);
	}

	public BaseWriter(Writer out) {
		writer = new PrintWriter(out);
	}
		
	public void setAutoClose(boolean autoclose) {
		this.autoclose = autoclose;
	}

	public int getLinesOut() {
		return linesOut;
	}
	
	protected String getValue(Object value) {
		if (value == null)
			return defaultValue;
		// Display of numerical values 
		if (value instanceof Number) { 
			Number number = (Number)value;
			DecimalFormat format;		
			if (number.intValue()!=0 && (Math.abs(number.floatValue())<0.00001) || Math.abs(number.floatValue())>1E+10) {
				format = new DecimalFormat("0.######E0");			
			}
			else {
				format = new DecimalFormat("0.######");
			}						
			DecimalFormatSymbols symbols = format.getDecimalFormatSymbols();
			symbols.setDecimalSeparator('.');
			format.setDecimalFormatSymbols(symbols);
			return format.format(number.doubleValue());
		}
		String str = value.toString();
		return str;
	}
	

	public int writeFromReader(BufferedReader reader, int start, int end) throws RuntimeException {
		int lines=1;
        try {
            String tmp;
            for (; (tmp = reader.readLine()) != null && lines<=end; lines++) {
            	if (lines>=start)
            		writer.println(tmp);
            }
           	reader.close();         
        } catch (Exception e) {
			throw new RuntimeException("Failed to write from reader: "+e.getMessage());
        }	
       writer.flush();
       return lines;
	}
	
	public void writeEmptyLines (int lines) {
		for (int i=0; i<lines;i++) {
			writer.println();	
		}								
	    writer.flush();	
	}

	public void setDefaultValue(String defaultValue) {
		this.defaultValue = defaultValue;
	}

	public void close() {
		writer.close();
	}
	
		

}
