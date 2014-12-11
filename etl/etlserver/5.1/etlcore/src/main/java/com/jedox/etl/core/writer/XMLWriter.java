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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;

public class XMLWriter extends BaseWriter {
	
	private String dataStartTag = "<data>"; 
	private String dataEndTag = "</data>"; 
	private String headerStartTag = "<header>"; 
	private String headerEndTag = "</header>"; 
	private String rowStartTag = "<row>"; 
	private String rowEndTag = "</row>";
	private String valueStartTag = "<value>"; 
	private String valueEndTag = "</value>";
	protected String inputTemplate = valueStartTag + "%s" + valueEndTag;
	private boolean columnNameAsTagName = false;
	private boolean withProlog = false;
	private String fileToAdd = null;
	
	private static String tab1 = "\t";
	private static String tab2 = "\t\t";
	private static String prolog = "<?xml version=\"1.0\" encoding=\"ENCODING_VALUE\"?>";
	private static final Log log = LogFactory.getLog(XMLWriter.class);

		
	public XMLWriter(OutputStream out, String encoding) {
		super(out);
		prolog = prolog.replace("ENCODING_VALUE", encoding);
	}

	public XMLWriter(Writer out, String encoding) {
		super(out);
		prolog = prolog.replace("ENCODING_VALUE", encoding);
	}
	
	public XMLWriter(Writer out, String fileToAdd, String encoding) {
		super(out);
		prolog = prolog.replace("ENCODING_VALUE", encoding);
		this.fileToAdd = fileToAdd;
	}
	
	public void write(IProcessor rows) throws RuntimeException {
		if (rows != null) {
			try {
				Row row = rows.current();
				checkFileToAdd();
				if(withProlog){
					writer.println(prolog);
					withProlog=false;
				}
				if (row != null) {
					writer.println(dataStartTag);
					if (row.size() > 0 && !columnNameAsTagName) {//print header

						writer.println(tab1 +headerStartTag);
						for (int i=0; i<row.size();i++) {
							writer.println(tab2 + getXMLEncoding(row.getColumn(i),row.getColumn(i).getName()));
						}
						writer.println(tab1 + headerEndTag);
						
					}
					
					doFileAdd();
					
					row = rows.next();
					while (row != null && (row.size() > 0)) {
						writer.println(tab1 + rowStartTag);
						for (int i=0; i<row.size();i++) {
							writer.println(tab2 + getXMLEncoding(row.getColumn(i),getValue(row.getColumn(i).getValueAsString())));
						}
						writer.println(tab1 + rowEndTag);
						linesOut++;
						//TODO there is an empty line in this setup at the end of the file.
						row = rows.next();
					}
					writer.println(dataEndTag);
				}
			} catch (Exception e) {
				throw new RuntimeException("Failed to write to xml stream: "+e.getMessage());
			}finally{
				writer.flush();
				if (autoclose) 
					close();
			}
		}
	}

	/**
	 * @throws FileNotFoundException
	 * @throws IOException
	 */
	private void doFileAdd() throws FileNotFoundException, IOException {
		if(fileToAdd!=null){
			BufferedReader bf = new BufferedReader(new FileReader(new File(fileToAdd)));
			String startString= dataStartTag;
			String endString= dataEndTag;
			if(!this.columnNameAsTagName){
				startString = headerEndTag;
			}
			String line = null;
			boolean include = false;
			 while (( line = bf.readLine()) != null){
				 if(include && line.indexOf(endString)==-1)
					 writer.println(line);
				 else if(!include && line.indexOf(startString)!=-1){
					 include=true;
				 }else{}
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
					log.debug("The existing file is empty.");
					return;
				}					
				line = bf.readLine().trim();
				if(line.indexOf(dataStartTag)==-1){
					throw new RuntimeException("The existing file does not have the root tag <"+dataStartTag+">.");
				}
				line = bf.readLine().trim();
				if(!this.columnNameAsTagName){
					if(line.indexOf(headerStartTag)==-1){ 
						 throw new RuntimeException("The existing file has no header tag although columnNameAsTag is false.");
					}
				}else{
					if(line.indexOf(headerStartTag)!=-1){ 
						 throw new RuntimeException("The existing file has a header tag although columnNameAsTag is true.");
					}
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

	protected String getXMLEncoding (IColumn iColumn, String input) {
		if(!columnNameAsTagName)
			return inputTemplate.replaceFirst("%s", Matcher.quoteReplacement(input));
		else
			return "<" + iColumn.getName() + ">" + Matcher.quoteReplacement(input) + "</" + iColumn.getName() + ">";
	}
	
	public void setDataTag(String data){
		dataStartTag = "<" + data +">";
		dataEndTag = "</" + data +">";
	}
	
	public void setRowTag(String row){
		rowStartTag = "<" + row +">";
		rowEndTag = "</" + row +">";
	}
	
	public void setColumnNameAsTagName(boolean columnNameAsTagName){
		this.columnNameAsTagName = columnNameAsTagName;
	}
	
	public void setWithProlog(boolean withProlog){
		this.withProlog = withProlog;
	}

}
