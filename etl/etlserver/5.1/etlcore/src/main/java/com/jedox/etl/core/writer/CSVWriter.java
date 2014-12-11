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

import java.io.OutputStream;
import java.io.Writer;
import java.util.regex.Matcher;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;

public class CSVWriter extends BaseWriter {

	private char delimiter = ';';
	protected String quote = "\"";
	protected boolean header = true;
	
	protected String inputTemplate = "\"%s\"";
	protected String doubleQuote = "\"\"";
		
	public CSVWriter(OutputStream out) {
		super(out);
	}

	public CSVWriter(Writer out) {
		super(out);
	}
	
	public void write(IProcessor rows) throws RuntimeException {
		if (rows != null) {
			try {
				Row row = rows.current();
				if (row != null) {
					if (header && (row.size() > 0)) {//print header
						for (int i=0; i<row.size()-1;i++) {
							writer.print(getCSVEncoding(row.getColumn(i).getName())+getDelimiter());
						}
						writer.println(getCSVEncoding(row.getColumn(row.size()-1).getName()));
					}
					row = rows.next();
					while (row != null && (row.size() > 0)) {
						for (int i=0; i<row.size()-1;i++) {
							writer.print(getCSVEncoding(getValue(row.getColumn(i).getValueAsString()))+getDelimiter());
						}
						writer.println(getCSVEncoding(getValue(row.getColumn(row.size()-1).getValueAsString())));
						linesOut++;
						//TODO there is an empty line in this setup at the end of the file.
						row = rows.next();
					}
				}
				writer.flush();
			} catch (Exception e) {
				throw new RuntimeException("Failed to write to csv stream: "+e.getMessage());
			}finally{
				if (autoclose) 
					close();
			}
		}
	}
	
	public void setQuote(String quote) {
		this.quote = quote;
		this.doubleQuote = quote+quote;
	}
	
	// Duplicate quotes in String and set it in quotes
	protected String getCSVEncoding (String input) {
		if (getQuote().isEmpty())
			return input;
		else
			return inputTemplate.replaceFirst("%s", Matcher.quoteReplacement(input.replaceAll(getQuote(),getDoubleQuote())));
	}

	private String getQuote() {
		return quote;
	}

	public void setDelimiter(char delimiter) {
		this.delimiter = delimiter;
	}

	private char getDelimiter() {
		return this.delimiter;
	}
	
	public void println(Object[] values) {
		if (values.length>0) {
			for (int i=0; i<values.length-1;i++) {
				writer.print(getCSVEncoding(values[i].toString())+getDelimiter());
			}		
			writer.println(getCSVEncoding(values[values.length-1].toString()));
		}	
	}	
	
	public void print(Object value) {
		writer.print(getCSVEncoding(value.toString())+getDelimiter());
	}
	
	public void println(Object value) {
		writer.println(getCSVEncoding(value.toString()));
	}
	
	private String getDoubleQuote() {
		return doubleQuote;
	}
	
	public void setHeader(boolean header) {
		this.header = header;
	}
	

}
