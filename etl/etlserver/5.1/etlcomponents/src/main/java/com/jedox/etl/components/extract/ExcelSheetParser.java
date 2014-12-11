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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.poi.openxml4j.exceptions.InvalidFormatException;
import org.apache.poi.openxml4j.opc.OPCPackage;
import org.apache.poi.ss.usermodel.BuiltinFormats;
import org.apache.poi.ss.usermodel.DataFormatter;
import org.apache.poi.ss.usermodel.DateUtil;
import org.apache.poi.xssf.eventusermodel.ReadOnlySharedStringsTable;
import org.apache.poi.xssf.eventusermodel.XSSFReader;
import org.apache.poi.xssf.model.StylesTable;
import org.apache.poi.xssf.usermodel.XSSFCellStyle;
import org.apache.poi.xssf.usermodel.XSSFRichTextString;
import org.xml.sax.Attributes;
import org.xml.sax.ContentHandler;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

import com.jedox.etl.components.extract.ExcelExtract.Range;
import com.jedox.etl.core.component.RuntimeException;

    public class ExcelSheetParser {
    	
    	private static final Log log = LogFactory.getLog(ExcelSheetParser.class);    	

        enum xssfDataType {
            BOOL, ERROR, FORMULA, INLINESTR, SSTINDEX, NUMBER,
        }

        int countrows = 0;
        static final String stopParsing = "STOP"; 

        private class XSSFSheetHandler extends DefaultHandler {

            private StylesTable stylesTable;
            private ReadOnlySharedStringsTable sharedStringsTable;
 
            // Set when V start element is seen
            private boolean vIsOpen;

            // Set when cell start element is seen;
            // used when cell close element is seen.
            private xssfDataType nextDataType;

            // Used to format numeric cell values.
            private short formatIndex;
            private String formatString;
            private final DataFormatter formatter;

            private int thisColumn = 0;
            // The last column printed to the output stream
            private int lastColumnNumber = 0;

            // Gathers characters as they are seen.
            private StringBuffer value;

            private int thisRow = 0;
            private boolean useRow = true;
			private ArrayList<Object> row = new ArrayList<Object>();
			
				            
            public XSSFSheetHandler(StylesTable styles, ReadOnlySharedStringsTable strings) {
                this.stylesTable = styles;
                this.sharedStringsTable = strings;
                this.value = new StringBuffer();
                this.nextDataType = xssfDataType.NUMBER;
                this.formatter = new DataFormatter();
                row = new ArrayList<Object>();
            }

            public void startElement(String uri, String localName, String name, Attributes attributes) throws SAXException {

            	if  ("row".equals(name)) {
                    thisRow++;
    				useRow=(range.getMinRow()<=thisRow && thisRow<=range.getMaxRow());            		
            	}	
            	if (!useRow) {
            		return;
            	}
                if ("inlineStr".equals(name) || "v".equals(name)) {
                    vIsOpen = true;
                    // Clear contents cache
                    value.setLength(0);
                }
                // c => cell
                else if ("c".equals(name) && useRow ) {
                    // Get the cell reference
                    String r = attributes.getValue("r");
                    // Get the column index 
                    thisColumn = range.getColumnIndex(r);

                    // Set up defaults.
                    this.nextDataType = xssfDataType.NUMBER;
                    this.formatIndex = -1;
                    this.formatString = null;
                    String cellType = attributes.getValue("t");
                    String cellStyleStr = attributes.getValue("s");
                    if ("b".equals(cellType))
                        nextDataType = xssfDataType.BOOL;
                    else if ("e".equals(cellType))
                        nextDataType = xssfDataType.ERROR;
                    else if ("inlineStr".equals(cellType))
                        nextDataType = xssfDataType.INLINESTR;
                    else if ("s".equals(cellType))
                        nextDataType = xssfDataType.SSTINDEX;
                    else if ("str".equals(cellType))
                        nextDataType = xssfDataType.FORMULA;
                    else if (cellStyleStr != null) {
                        // It's a number, but almost certainly one
                        // with a special style or format
                        int styleIndex = Integer.parseInt(cellStyleStr);
                        XSSFCellStyle style = stylesTable.getStyleAt(styleIndex);
                        this.formatIndex = style.getDataFormat();
                        this.formatString = style.getDataFormatString();
                        if (this.formatString == null)
                            this.formatString = BuiltinFormats.getBuiltinFormat(this.formatIndex);
                    }
                }

            }

            @SuppressWarnings("unchecked")
			public void endElement(String uri, String localName, String name) throws SAXException {

            	if (!useRow) {
            		return;
            	}
                Object thisObject = null;

                // v => contents of a cell
                if ("v".equals(name) && range.isIncludedColumn(thisColumn)) {
                    // Process the value contents as required.
                    // Do now, as characters() may be called more than once
                    switch (nextDataType) {

                    case BOOL:
                        char first = value.charAt(0);
                        thisObject = first == '0' ? "FALSE" : "TRUE";
                        break;

                    case ERROR:
                        thisObject = "ERROR:" + value.toString();
                        break;

                    case FORMULA:
                        thisObject = value.toString(); 
                        break;

                    case INLINESTR:
                        // TODO: have seen an example of this, so it's untested.
                        XSSFRichTextString rtsi = new XSSFRichTextString(value.toString());
                        thisObject = rtsi.toString();
                        break;

                    case SSTINDEX:
                        String sstIndex = value.toString();
                        try {
                            int idx = Integer.parseInt(sstIndex);
                            XSSFRichTextString rtss = new XSSFRichTextString(sharedStringsTable.getEntryAt(idx));
                            thisObject = rtss.toString();
                        } catch (NumberFormatException ex) {
                            log.error("Failed to parse SST index '" + sstIndex + "': " + ex.toString());
                        }
                        break;

                    case NUMBER:
                        String n = value.toString();
                        if (this.formatString != null)
                        	
                            if (DateUtil.isADateFormat(this.formatIndex, this.formatString)){
                            	String dateStr = null;
                                this.formatIndex = 14;
                                this.formatString = BuiltinFormats.getBuiltinFormat(formatIndex);
                                dateStr = formatter.formatRawCellContents(Double.parseDouble(n), this.formatIndex, this.formatString);
                                dateStr += " " + formatter.formatRawCellContents(Double.parseDouble(n), 19, BuiltinFormats.getBuiltinFormat(19));   
                                try {
									thisObject = new SimpleDateFormat("M/d/yy h:mm:ss a").parse(dateStr.toString());
									if(sdf!=null)
										thisObject = sdf.format(thisObject);
                                } catch (ParseException e) {
									log.error(e.getMessage());
								}
                                
                             } else {
                        		 try {
									thisObject=Double.valueOf(n);
									// convert to int if possible
									if(Integer.parseInt(n)==(Double)thisObject){
										thisObject = Integer.parseInt(n);
									}
								} catch (NumberFormatException e) {
									thisObject=n;
								}                            	 
                             }
//                            thisStr = formatter.formatRawCellContents(Double.parseDouble(n), this.formatIndex, this.formatString);
                        else{
                        	 try {
									thisObject=Double.valueOf(n);
									// convert to int if possible
									if(Integer.parseInt(n)==(Double)thisObject){
										thisObject = Integer.parseInt(n);
									}
								} catch (NumberFormatException e) {
									thisObject=n;
								}   
                        }
                        break;

                    default:
                        log.error("Unexpected Excel cell type:"  + nextDataType);
                        break;
                    }
                    
        			
        				// empty columns for missing fields in row
        				for (int i = lastColumnNumber+1; i < thisColumn; ++i) {
        					if (range.isIncludedColumn(i))
        						row.add(null);
        				}
        				if (range.isIncludedColumn(thisColumn)) {
        					row.add(thisObject);
        				}	

                    if (thisColumn > 0)
                        lastColumnNumber = thisColumn;

                } else if ("row".equals(name)) {
                    // We're onto a new row
    				data.add((ArrayList<Object>)row.clone());
					row = new ArrayList<Object>();
                    lastColumnNumber = 0;
                    
                    if (thisRow>=range.getMaxRow())
                    	throw new SAXException(stopParsing);
                    
                }
            }

            /**
             * Captures characters only if a suitable element is open. Originally
             * was just "v"; extended for inlineStr also.
             */
            public void characters(char[] ch, int start, int length)  throws SAXException {
                if (vIsOpen)
                    value.append(ch, start, length);
            }

        }

        private OPCPackage xlsxPackage;
        private Range range;
		private SimpleDateFormat sdf = null;                
        private ArrayList<ArrayList<Object>> data;
        
        public ExcelSheetParser(OPCPackage pkg, Range range, SimpleDateFormat sdf) {
            this.xlsxPackage = pkg;
            this.range = range;
            this.sdf = sdf;
        }

        /**
         * Parses and shows the content of one sheet using the specified styles and shared-strings tables.
         */
        private void processSheet(StylesTable styles, ReadOnlySharedStringsTable strings, InputStream sheetInputStream)
                throws IOException, ParserConfigurationException, SAXException, RuntimeException {

			log.info("Parsing Excel worksheet "+range.getSheetName());
        	InputSource sheetSource = new InputSource(sheetInputStream);
            SAXParserFactory saxFactory = SAXParserFactory.newInstance();
            SAXParser saxParser = saxFactory.newSAXParser();
            XMLReader sheetParser = saxParser.getXMLReader();
            ContentHandler handler = new XSSFSheetHandler(styles, strings);
            sheetParser.setContentHandler(handler);
			try {
	            sheetParser.parse(sheetSource);
			} catch (SAXException e) {
				if (!e.getMessage().endsWith(stopParsing))					
					throw new RuntimeException(e.getMessage());
			}             
        }


        private InputStream getSheetStream (XSSFReader xssfReader) throws RuntimeException, InvalidFormatException, IOException {
            XSSFReader.SheetIterator iter = (XSSFReader.SheetIterator) xssfReader.getSheetsData();
            if (range.getSheetName()==null) {
            	// take first sheet
            	InputStream stream = iter.next();
            	range.setSheetName(iter.getSheetName()); // Sheet name only required for logging
            	return stream;            	
            }
            else {
                while (iter.hasNext()) {
                    InputStream stream = iter.next();
                    if (range.getSheetName().equals(iter.getSheetName())) {
                    	return stream;
                    }	
                }
                throw new RuntimeException("Sheet with name "+range.getSheetName()+" not found");           	
            }       	
        }
        
        public ArrayList<String> getWorkSheets() throws RuntimeException{
			try {
				ArrayList<String> sheets = new ArrayList<String>();
				XSSFReader xssfReader = new XSSFReader(this.xlsxPackage);
				XSSFReader.SheetIterator iter = (XSSFReader.SheetIterator) xssfReader.getSheetsData();
	            while (iter.hasNext()) {
	               iter.next();
	               sheets.add(iter.getSheetName());
	             }  
				return sheets;
			} catch (Exception e) {
				throw new RuntimeException("Error in parsing Excel sheet: "+e.getMessage());
			}
        	
        }
        
        /**
         * Starts the parsing of the XLSx workbook file
         * 
         */
        public void process() throws RuntimeException {
        	
			data = new ArrayList<ArrayList<Object>>();
			InputStream stream= null;
			try {
				ReadOnlySharedStringsTable strings = new ReadOnlySharedStringsTable(this.xlsxPackage);
				XSSFReader xssfReader = new XSSFReader(this.xlsxPackage);
				StylesTable styles = xssfReader.getStylesTable();
				stream=getSheetStream(xssfReader);
				processSheet(styles, strings, stream);
				
			} catch (Exception e) {
				throw new RuntimeException("Error in parsing Excel sheet: "+e.getMessage());
			}finally{
				if(stream!=null){
					try {
						xlsxPackage.close();
						stream.close();
					} catch (IOException e) {
					}
				}
			}
        }
        
        public ArrayList<ArrayList<Object>> getData() {
        	return data;
        };

            
        
}

