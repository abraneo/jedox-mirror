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
package com.jedox.etl.components.connection;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IFileConnection;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.MetadataWriter;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.PersistenceUtil;
import com.jedox.etl.core.util.Recoder;
import com.jedox.etl.core.util.SSLUtil;
import com.jedox.etl.core.util.SSLUtil.SSLModes;
import com.jedox.etl.core.util.URLUtil;
import com.jedox.etl.core.writer.CSVWriter;

import au.com.bytecode.opencsv.CSVParser;
import au.com.bytecode.opencsv.CSVReader;


public class FileConnection extends RelationalConnection implements IFileConnection {

	private static final Log log = LogFactory.getLog(FileConnection.class);
	private static final Map<String,Filestore> stores = new HashMap<String,Filestore>();
	private String filename;
	private char delimiter = ';';
	private String quote ="\"";
	private String readingQuote;
	private char escapeQuote;
	private boolean header = true;
	//private boolean isWriteable = false;
	private SSLModes sslMode;
	private PersistorDefinition storeDefinition;
	private Properties externalParameters = new Properties();
	
	protected synchronized static Filestore getStore(PersistorDefinition storeDefinition) throws RuntimeException {
		Filestore store = stores.get(storeDefinition.getPersistentName());
		if (store == null) {
			store = new Filestore(storeDefinition);
			stores.put(storeDefinition.getPersistentName(), store);
		}
		return store;
	}
	
	private class CachedProcessor extends Processor {
		
		private IProcessor cachedResult;
		private Filestore store;
		private int cacheSize;
		private int size;		
		private String query;
		
		public CachedProcessor(Filestore store, String query, int cacheSize, int size) {
			this.store = store;
			this.cacheSize = cacheSize;
			this.size = size;
			this.query = query;
		}

		@Override
		protected boolean fillRow(Row row) throws Exception {
			return cachedResult.next() != null;
		}

		@Override
		protected Row getRow() throws RuntimeException {
			return cachedResult.current();
		}

		@Override
		protected void init() throws RuntimeException {
			cacheProcessor();
			cachedResult = store.getProcessor(query, size);
		}
		
		private void cacheProcessor() throws RuntimeException {
			PersistorDefinition definition = getStoreDefinition();
			File file = new File(getFilename());
			long fileLastModified = file.lastModified();
			if(FileUtil.isURL(getDatabase())){
				try {
					fileLastModified = URLUtil.getInstance().getResponseLastModified(getDatabase());
				} catch (Exception e) {
					fileLastModified = Long.MAX_VALUE;
					log.debug("Error trying to get the Last-Modified property from the response header: " + e.getMessage());
				}
			}
			if (fileLastModified >= store.getTimestamp()) {
				log.debug("Caching File Connection "+getName()+" to internal persistence (lines: "+cacheSize+")");
				IProcessor processor = getFileProcessor(new AliasMap(), cacheSize);
				definition.setInput(processor.current());
				store.setDefinition(definition);
				store.setWriteable();
				try {
					Row row = processor.next();
					while (row != null) {
						store.write();
						row = processor.next();
					}
					if (cacheSize <= 0) { //only consider full import as valid cache.
						store.setTimestamp(new Date().getTime());
					}
				}
				catch (Exception e) {
					throw new RuntimeException("Failed to persist file data to store: "+e.getMessage());
				}
				finally {
					store.commit();
				}
			}
		}
		
	}

	protected class FileProcessor extends Processor {

		private Row row;
		private CSVReader reader;
		private String encoding;
		private String filename;
		private IAliasMap aliasMap;
		
		public FileProcessor(String filename, String encoding, IAliasMap aliasMap) throws RuntimeException {
			this.encoding = encoding;
			this.filename = filename;
			if (aliasMap != null)
				this.aliasMap = aliasMap;
			else
				this.aliasMap = new AliasMap();
		}
		
		public String getLogDisplayType() {
			return (getOwner() == null || getOwner() instanceof IFileConnection) ? "file" : super.getLogDisplayType();
		}

		protected void init() throws RuntimeException {
			log.debug("Start reading file " + filename + " with encoding" + encoding);
			CSVReader reader = getReader();
			try {
				//do the skip before building the header
				skip(reader);
				row = getHeader(reader.readNext(), getColumns());
				reader.close();
			}
			catch (Exception e) {
				throw new RuntimeException("Error when starting reading the file " + filename + ": " + e.getMessage());
			}
		}

		public void close() {
			log.debug("Closing file processor "+getName());
			super.close();
			try {
				if (reader!= null) {
					reader.close();
					reader = null;
				}
			}
			catch (Exception e) {
				log.debug(e.getMessage());
			};
		}

		private CSVReader getReader() throws RuntimeException {
			CSVReader reader;
			try {
				InputStream s = FileUtil.getInputStream(filename, true, sslMode);
				reader = new CSVReader(new InputStreamReader(s,encoding), delimiter, readingQuote.toCharArray()[0], escapeQuote);
			}
			catch (Exception e) {
				String filePath = (FileUtil.isURL(getFilename())?filePath = getFilename():new File(getFilename()).getAbsolutePath());
				throw new RuntimeException("Failed to read file "+ filePath +": "+e.getMessage());
			}
			return reader;
		}

		private Row getHeader(String[] line, int columns) throws RuntimeException {
			if (columns == 0) {
				if (line==null) {
					throw new RuntimeException("File is empty but header row is required.");
				}
				columns = line.length;
			}	
			List<String> columnNames = new ArrayList<String>();
			List<String> origNames = new ArrayList<String>();
			HashSet<String> names = new HashSet<String>();
			boolean headerValuesFound=false;
			for (int i=0; i<columns;i++) {
				String name = null;
				//original default name for no header
				String origName = "column"+Integer.toString(i+1);
				//default name based on alias for no header
				String defaultName = aliasMap.getAlias(i+1, origName);
				if ((header) && (i < line.length)) {
					//header name from file
					String headerName =line[i].trim();
					if (!headerName.isEmpty()) {
						origName = headerName;
						headerValuesFound=true;
					}	
					name = aliasMap.getAlias(i+1, headerName);
					if (name.isEmpty())
						name = defaultName;
					if(name.length()>128){
						log.warn("Column name: " + name + " contains more than 128 charaters, hence it will be cut to only 128.");
						name = name.substring(0, 127);
					}
					if (names.contains(name.toLowerCase())) {
						String substitute = defaultName;
						if (names.contains(substitute)) {
							substitute = NamingUtil.internal(defaultName);
						}
						log.warn("Header name "+name+" is duplicate in file "+getName()+". Using name "+substitute+" instead.");
						name = substitute;
					}
				}
				else name = defaultName;
				columnNames.add(name);
				names.add(name.toLowerCase());
				origNames.add(origName);
			}
			if (header && !headerValuesFound)
				log.warn("Header line is empty");
			
			Row row = PersistenceUtil.getColumnDefinition(columnNames);
			//set orignal names in aliases
			for (int i=0; i<origNames.size(); i++) {
				String origName = origNames.get(i);
				if (!row.getColumn(i).getName().equals(origName)) {
					if (!aliasMap.hasAlias(i+1)) {
						AliasMapElement m = new AliasMapElement();
						m.setOrigin(origName);
						m.setColumn(i+1);
					} else {
						for (String key : aliasMap.getAliases()) {
							AliasMapElement m = aliasMap.getElement(key);
							if (m.getColumn()==i+1) {
								m.setOrigin(origName);
								break;
							}
						}
					}
				}
			}
			return row;
		}
		
		private void skip(CSVReader reader) {
			
			try {
				for(int i=0;i<getSkip();i++){	
					reader.readNext();
				}
			} catch (IOException e) {
				log.error("Error while skipping the file " + getName());
			}
		}

		protected boolean fillRow(Row row) throws Exception {
			if (reader == null) {
				reader = getReader();
				
				//do the skip before reading the file
				skip(reader);
				if (header)
					reader.readNext();
			}
			String[] line = reader.readNext();
			if (line == null) {
				reader.close();
				return false;
			}
			if (line.length > 0) {
				for (int i=0; i<Math.min(row.size(),line.length); i++) {
					String value =line[i];
					// A single occurence of the quote in the file is not allowed. CSV Reader returns complete file in that case, so this line contains a Carriage Return 
					// if(value != null && value.contains("\n") && value.contains(readingQuote) && value.indexOf(readingQuote)<value.indexOf("\n")){
					//	log.warn("Single occurence of enclosure character "+ quote +" is not allowed in data of file extract " + getName() + " in line starting with " + NamingUtil.removeIllegalWhiteSpaces(line[0]) +",... The rest of the file is ignored.");
					//	break;
					//}
					//else{
						row.getColumn(i).setValue(value);
					//}
				}
				for (int i=Math.min(row.size(),line.length); i<row.size(); i++)
					row.getColumn(i).setValue(null);
			}
			return true;
		}

		protected Row getRow() {
			return row;
		}
		
	}


	protected boolean isCached() throws RuntimeException {
		File file = new File(getFilename());
		return (file.exists() && (file.lastModified() < getTimestamp()));
	}

	

	protected java.sql.Connection connect2Relational() throws RuntimeException {
		return getInternalConnection().open();
	}

	public CSVWriter getWriter(boolean append) throws RuntimeException {
//		if (getFilename() == null) {
//			if (FileUtil.isRelativ(filename))
//				filename = Settings.getInstance().getDataDir() + "/" + filename;
//			setFilename(filename);
//		}
		CSVWriter writer = null;
		try {
			writer = new CSVWriter(new OutputStreamWriter(new FileOutputStream(getFilename(),append),getFileEncoding()));
			writer.setDelimiter(delimiter);
			writer.setHeader(header);
			writer.setQuote(quote);
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to open file "+getFilename()+" for writing: "+e.getMessage());
		}
		return writer;
	}

	protected int getFirstRow() {
		try {
			int startLine =  Math.max(Integer.parseInt(externalParameters.getProperty("start","1"))-1,0);
			return startLine;
		}
		catch (NumberFormatException e) {
			log.error("Failed to parse number of start line in connection "+ getName() + ": "+e.getMessage());
		}
		return 0;
		}


	protected int getLastRow() throws RuntimeException {
		try {
			int lastLine = Integer.parseInt(externalParameters.getProperty("end","0"));
			if(lastLine<getFirstRow() && lastLine != 0){
				throw new RuntimeException("In File connection " + getName() + " end line number can be either 0 or smaller than start line.");
			}
			else{
				return (lastLine == 0) ? Integer.MAX_VALUE: lastLine;
			}
		}
		catch (NumberFormatException e) {
			log.error("Failed to parse number of end line: "+e.getMessage());
		}
		return 0;
		}

	public String getServerName() {
		return "internal";
	}

	public String getFileEncoding() {
		return super.getEncoding();
	}

	public String getEncoding() {
		//Data is internally stored in utf8 database no matter what the original file encoding was.
		return Recoder.internalCoding;
	}

	protected void setFilename(String filename) {
		this.filename = filename;
	}

	protected String getFilename() {
		return filename;
	}

	protected long getTimestamp() throws RuntimeException {
		return getStore(getStoreDefinition()).getTimestamp();
	}

	public void test() throws RuntimeException {
		//checkSSL();
		FileUtil.test(getFilename(), true, sslMode);		
	}


	protected PersistorDefinition getStoreDefinition() throws RuntimeException {
		if (storeDefinition == null) {
			storeDefinition = new PersistorDefinition();
			storeDefinition.setConnection(getInternalConnection());
			Locator loc = getLocator().clone();
			loc.setContext("c"+String.valueOf(getHash().hashCode()).replace("-", "m"));
			storeDefinition.setLocator(loc);
			storeDefinition.setMode(Modes.CREATE);
		}
		return storeDefinition;
	}

	protected IProcessor getFileProcessor(IAliasMap aliasMap, int size) throws RuntimeException {
		IProcessor processor = initProcessor(new FileProcessor(getFilename(),getFileEncoding(), aliasMap),Facets.CONNECTION);
		processor.setFirstRow(getFirstRow());
		size = (size == 0) ? getLastRow() : size;
		processor.setLastRow(Math.min(getLastRow(),getFirstRow()+size));
		return processor;
	}

	public IProcessor getProcessor(String query, IAliasMap aliasMap, Boolean onlyHeader, Boolean ignoreInternalKey, int size) throws RuntimeException {
		//checkSSL();		
		//if we have no query and thus are able to use direct access, there is no need to cache, but we use cache if it is available
		if (query == null || query.isEmpty()) {
			if (!isCached()) { //use direct access if not already cached
				return getFileProcessor(aliasMap, size);
			}
			else //use cache
				return getStore(getStoreDefinition()).getProcessor("select * from "+NamingUtil.internalDatastoreName(),size);
		}
		else { //we need to cache since we have a query.
			//File file = new File(getFilename());
			Filestore store;
			try {
				FileUtil.test(getFilename(),false, sslMode);
				store = getStore(getStoreDefinition());
				IProcessor result = initProcessor(new CachedProcessor(store,query,onlyHeader ? size : 0, size),Facets.CONNECTION);
				result.current().setAliases(aliasMap);
				return result;
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to read data from file: "+ new File(getFilename()).getAbsolutePath()+": "+e.getMessage());
			}
			finally {
			}
		}
	}

	protected String getHash() throws RuntimeException {
		StringBuffer result = new StringBuffer();
		result.append(getDatabase());
		result.append(getFileEncoding());
		result.append(delimiter);
		result.append(quote);
		result.append(header);
		result.append(getColumns());
		result.append(getFirstRow());
		result.append(getLastRow());		
		return result.toString();
	}

	public String getConnectionUrl() throws RuntimeException {
		return getInternalConnection().getConnectionUrl();
	}
	
	public void setExternalParameter(String key, String value) {
		externalParameters.put(key, value);
	}
	
	private int getSkip() {
		return Integer.parseInt(externalParameters.getProperty("skip","0"));
	}
	
	private int getColumns() {
		return Integer.parseInt(externalParameters.getProperty("columns","0"));
	}
	
	/*protected void checkSSL() throws RuntimeException{		
		String urlString = getFilename();
		if (SSLUtil.supportsSSL(urlString) && sslMode.equals(SSLUtil.SSLModes.trust)) {
			try {
				URL url = new URL(urlString);
				SSLUtil util = new SSLUtil();					
				try {
					util.addCertToKeyStore(url);
				}
				catch (Exception e) {
					if (e instanceof UnknownHostException)
						throw new RuntimeException("Host "+urlString+" is unknown.");
					if (e instanceof ConnectException)
						throw new RuntimeException("Could not connect to host "+urlString+" : "+e.getMessage());
					if (e instanceof MalformedURLException) 
						throw new RuntimeException("Filename "+getFilename()+" is not a legal URL. SSL trust mode is only available for URLs.");
					throw new RuntimeException(e);						
				}
			}			
			catch (Exception e) {
				throw new RuntimeException(e);
			}			
		}
	}*/


	public void init() throws InitializationException {
		try {
			super.init();
			if(getDatabaseString().trim().isEmpty()){
				 throw new InitializationException("File name is empty in connection " + getName());
			}
			setFilename(getDatabase()); //Relative();
			sslMode = SSLModes.valueOf(getParameter("ssl",SSLUtil.SSLModes.verify.toString()));
			// Delimiter should be set in File-Connection. In forthcoming release it will be obligatory via xsd 
			if (getParameter("delimiter","").isEmpty()) {
				log.warn("No Delimiter is set in Connection "+getName()+". Default value \",\" is used");
			}					
			String delimiterStr =  getParameter("delimiter",",").replaceAll(NamingUtil.spaceValue(), " ");
			if (delimiterStr.isEmpty())
				delimiter = ' ';
			else {
				delimiter = delimiterStr.toCharArray()[0];
				if (delimiterStr.equals("\\t"))
					delimiter = '\t';
				else if (delimiterStr.startsWith("\\u"))
					delimiter = FileUtil.convertUniCodeToChar(delimiterStr); 
				else {
					if (delimiterStr.length()>1)
						log.warn("Delimiter length should not be longer than one character. The file " + getName() + " may not be parsed as expected.");					
				}
			}
			// Quote should be set in File-Connection. In forthcoming release it will be obligatory via xsd 
			if (getParameter("quote","").isEmpty()) {
				log.info("No Enclosure Character (quote) is set in Connection "+getName()+". Default value \" is used");
			}					
			quote =  getParameter("quote","\"");
			escapeQuote = (getParameter("enableEscape","false").equalsIgnoreCase("true") ? CSVParser.DEFAULT_ESCAPE_CHARACTER : au.com.bytecode.opencsv.CSVWriter.NO_ESCAPE_CHARACTER);
			readingQuote = quote;
			if (quote.equalsIgnoreCase(NamingUtil.internal("none")) || quote.equalsIgnoreCase("NONE")) {
				// CSV reader has bug if no quote is defined (Inputs starting with "), so use not-used special character instead
				// For compatibility reasons keep value NONE
				readingQuote = "\u263A";
				quote="";
			}
			header =  getParameter("header","true").equalsIgnoreCase("true");
			setExternalParameter("skip", getParameter("skip","0"));
			setExternalParameter("columns", getParameter("columns","0"));
			setExternalParameter("end", getParameter("end","0"));
			setExternalParameter("start", getParameter("start","1"));
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

	protected StringWriter getColumnNamesWriter(Row firstRow) throws RuntimeException{
		StringWriter out = new StringWriter();
		try {
			MetadataWriter writer = new MetadataWriter(out);
			writer.println("Column");
			for (int i=0; i<firstRow.size(); i++) {
				writer.println(firstRow.getColumn(i).getName());
			}			
			close();
		}
		catch (Exception e) {
			throw new RuntimeException("Failed to get columns of connection "+getName()+": "+e.getMessage());
		}		
		return out;
	}
	
	public String getMetadata(Properties properties) throws RuntimeException {
		String selector = properties.getProperty("selector");
		if (selector.equals("columns")){
			Row firstRow = getProcessor(null, null, true, true, 1).getOutputDescription();
			return getColumnNamesWriter(firstRow).toString();
		}
		else
			throw new RuntimeException("Invalid value "+selector+" for Property 'selector'.");
	}

	public MetadataCriteria[] getMetadataCriterias() {
		MetadataCriteria[] crit = new MetadataCriteria[1];
		crit[0] = new MetadataCriteria("columns");
		return crit;
	}

	@Override
	public IProcessor getProcessor(IAliasMap map, int size, long timeout) throws RuntimeException {
		return getFileProcessor(map,size);
	}	
	
}
