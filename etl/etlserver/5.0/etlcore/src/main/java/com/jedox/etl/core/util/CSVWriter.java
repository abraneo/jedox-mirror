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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.core.util;

import java.io.BufferedReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.sql.ResultSet;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.TableProcessor;

public class CSVWriter implements IWriter {

	private PrintWriter writer;
	private String delimiter = ";";
	private String quote = "\"";
	private boolean header = true;
	private String sourceEncoding = "UTF8";
	private String targetEncoding = "UTF8";
	private String defaultValue = "";
	private int linesOut = 0;
	private boolean autoclose = true;


	public CSVWriter(OutputStream out) {
		autoclose = !out.equals(System.out);
		writer = new PrintWriter(out);
	}

	public CSVWriter(Writer out) {
		writer = new PrintWriter(out);
	}

	public void setAutoClose(boolean autoclose) {
		this.autoclose = autoclose;
	}

	public int getLinesOut() {
		return linesOut;
	}
	
	private String getValue(Object value) {
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

	// Duplicate quotes in String and set it in quotes
	private String getCSVEncoding (String input) {
		return getQuote()+input.replaceAll(getQuote(),getQuote()+getQuote())+getQuote();
	}
	
	public void write(IProcessor rows) throws RuntimeException {
		if (rows != null) {
			try {
				Row row = rows.current();
				if (row != null) {
					Recoder recoder = new Recoder();
					recoder.setRecoding(sourceEncoding, targetEncoding);
					if (header && (row.size() > 0)) {//print header
						for (int i=0; i<row.size()-1;i++) {
							writer.print(getCSVEncoding(recoder.recode(row.getColumn(i).getName()))+getDelimiter());
						}
						writer.println(getCSVEncoding(recoder.recode(row.getColumn(row.size()-1).getName())));
					}
					row = rows.next();
					while (row != null && (row.size() > 0)) {
						for (int i=0; i<row.size()-1;i++) {
							writer.print(getCSVEncoding(recoder.recode(getValue(row.getColumn(i).getValueAsString())))+getDelimiter());
						}
						writer.println(getCSVEncoding(recoder.recode(getValue(row.getColumn(row.size()-1).getValueAsString()))));
						linesOut++;
						//TODO there is an empty line in this setup at the end of the file.
						row = rows.next();
					}
				}
			} catch (Exception e) {
				throw new RuntimeException("Failed to write to csv stream: "+e.getMessage());
			}
			writer.flush();
		}
		if (autoclose) 
			close();
	}
/*
	public void write(ISource source, ResultSet result, int start, int end) throws RuntimeException {
		TableProcessor processor = new TableProcessor(source.getName(),source.getAliasMap(),result, true);
		processor.setFirstRow(start);
		processor.setLastRow(end);
		write(processor);
	}
*/
	
	public void write(ResultSet result) throws RuntimeException {
		TableProcessor processor = new TableProcessor("",null,result, true);
		write(processor);
	}
/*
	public void write(ISource source, ResultSet result) throws RuntimeException {
		write(source, result,0,0);
	}
*/	
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

	public void setSourceEncoding(String sourceEncoding) {
		if (sourceEncoding != null) this.sourceEncoding = sourceEncoding;
	}

	public void setTargetEncoding(String targetEncoding) {
		if (targetEncoding != null) this.targetEncoding = targetEncoding;
	}

	public void setDelimiter(String delimiter) {
		this.delimiter = delimiter;
	}

	private char getDelimiter() {
		char sep = delimiter.toCharArray()[0];
		if (delimiter.equals("\\t"))
			sep = '\t';
		return sep;
	}

	public void setQuote(String quote) {
		this.quote = quote;
	}

	private String getQuote() {
		return quote;
	}

	public void setHeader(boolean header) {
		this.header = header;
	}

	public void setDefaultValue(String defaultValue) {
		this.defaultValue = defaultValue;
	}

	public void close() {
		writer.close();
	}
	
	public void println(String line) {
		Recoder recoder = new Recoder();
		recoder.setRecoding(sourceEncoding, targetEncoding);
		writer.println(getCSVEncoding(recoder.recode(line)));
	}
	
	public PrintWriter getBackend() {
		return writer;
	}

}
