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
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.Writer;
import java.util.regex.Matcher;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;

public class JsonWriter extends BaseWriter {

	protected String inputTemplate = "\"%s\"";
	protected String doubleQuote = "\"\"";
	protected String quote = "\"";
	private String fileToAdd = null;
	private String customRoot = "data";
		
	public JsonWriter(OutputStream out) {
		super(out);
	}

	public JsonWriter(Writer out) {
		super(out);
	}
	
	public JsonWriter(Writer out, String fileToAdd) {
		super(out);
		this.fileToAdd = fileToAdd;
	}
	
	public void write(IProcessor rows) throws RuntimeException {
		if (rows != null) {
			try {
				checkFileToAdd();
				Row row = rows.current();
				if(row.size()>0){
					writer.println("{\"" + customRoot + "\":[");
				}
				doFileAdd();
				row = rows.next();
				boolean firstLine = (fileToAdd!=null?false:true);
				while (row != null && (row.size() > 0)) {
						if(!firstLine){
							writer.print(",");
						}
						writer.print("{");
						for (int i=0; i<row.size()-1;i++) {
							writer.print( getCSVEncoding(row.getColumn(i).getName()) + ":" + getCSVEncoding(getValue(row.getColumn(i).getValueAsString()))+",");
						}
						writer.println(getCSVEncoding(row.getColumn(row.size()-1).getName()) + ":" + getCSVEncoding(getValue(row.getColumn(row.size()-1).getValueAsString()))+"}");
						linesOut++;
						//TODO there is an empty line in this setup at the end of the file.
						row = rows.next();
						firstLine = false;
				}

				writer.println("]}");
			} catch (Exception e) {
				throw new RuntimeException("Failed to write to csv stream: "+e.getMessage());
			} finally{
				close();
			}			
			writer.flush();
		}
		if (autoclose) 
			close();
	}
	
	private void doFileAdd() throws FileNotFoundException, IOException {
		if(fileToAdd!=null){
			BufferedReader bf = new BufferedReader(new FileReader(new File(fileToAdd)));
			String line = bf.readLine(); //ignores the first line
			 while (( line = bf.readLine()) != null && !line.trim().equals("]}")){
					 writer.println(line);
			 }
			 bf.close();
		}
	}
	
	private void checkFileToAdd() throws IOException, RuntimeException {
		if(fileToAdd!=null){
			BufferedReader bf = null;
			try {
				bf = new BufferedReader(new FileReader(new File(fileToAdd)));
				//skip the prolog
				String line = bf.readLine();
				if (line==null){
					throw new RuntimeException("The existing file is empty.");
				}									
				int index=line.trim().indexOf('[');
				if (index==-1 || !line.trim().substring(0,index+1).matches("\\{\\s*\""+customRoot+"\"\\s*:\\s*\\[")) {
					throw new RuntimeException("The start line does not include the root as expected. The first line should start with {\""+customRoot+"\":[");
				}
				bf.close();
			} catch (RuntimeException e) {
				throw e;
			}finally{
				if(bf!=null)
					bf.close();
			}
		}
		
	}

	
	// Duplicate quotes in String and set it in quotes
	protected String getCSVEncoding (String input) {
		return inputTemplate.replaceFirst("%s", Matcher.quoteReplacement(input.replaceAll(quote,doubleQuote)));
	}
	
	public void setCustomRoot(String customRoot){
		this.customRoot = customRoot;
	}

}
