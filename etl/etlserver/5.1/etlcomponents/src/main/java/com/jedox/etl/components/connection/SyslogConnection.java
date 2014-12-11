package com.jedox.etl.components.connection;

import au.com.bytecode.opencsv.CSVReader;

import java.text.SimpleDateFormat;
import com.jedox.etl.core.aliases.IAliasMap;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.Connection;
import com.jedox.etl.core.connection.IStreamReadable;
import com.jedox.etl.core.connection.IStreamWritable;
import com.jedox.etl.core.connection.MetadataCriteria;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.Recoder;
import com.jedox.etl.core.writer.IWriter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.productivity.java.syslog4j.SyslogBackLogHandlerIF;
import org.productivity.java.syslog4j.SyslogIF;
import org.productivity.java.syslog4j.SyslogRuntimeException;
import org.productivity.java.syslog4j.impl.net.tcp.TCPNetSyslogConfig;
//import org.productivity.java.syslog4j.impl.net.udp.UDPNetSyslogConfig;
import org.productivity.java.syslog4j.server.SyslogServerEventHandlerIF;
import org.productivity.java.syslog4j.server.SyslogServerEventIF;
import org.productivity.java.syslog4j.server.SyslogServerIF;
import org.productivity.java.syslog4j.server.impl.net.tcp.TCPNetSyslogServerConfig;
//import org.productivity.java.syslog4j.server.impl.net.udp.UDPNetSyslogServerConfig;
import org.productivity.java.syslog4j.util.SyslogUtility;

import java.io.InputStreamReader;
import java.io.PipedInputStream;
import java.io.PrintStream;
import java.io.PipedOutputStream;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.Date;
import java.util.List;
import java.util.Properties;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

/**
 * 
 * @author Christian Schwarzinger
 * License: GPLv2
 */

public class SyslogConnection extends Connection implements IStreamWritable, IStreamReadable {
	
	protected class StreamProcessor extends Processor {

		private Row row;
		private Row staticColumns;
		private CSVReader reader;
		private Recoder recoder = new Recoder();
		private SimpleDateFormat dateFormat = new SimpleDateFormat("EEE MMM dd HH:mm:ss z yyyy");
		private ExecutorService executor = Executors.newFixedThreadPool(2);
		private long timeout;

		public StreamProcessor(InputStream stream, Row staticColumns, long timeout) throws RuntimeException {
			this.reader = getReader(stream);
			this.staticColumns = staticColumns;
			this.timeout = timeout;
		}

		private CSVReader getReader(InputStream stream) throws RuntimeException {
			CSVReader reader;
			try {
				char sep = delimiter.toCharArray()[0];
				if (delimiter.equals("\\t"))
					sep = '\t';
				String readingQuote = quote;
				if(quote.equals("")) readingQuote = " ";
				reader = new CSVReader(new InputStreamReader(stream), sep, readingQuote.toCharArray()[0]);
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to read csv data from stream "+getName()+": "+e.getMessage());
			}
			return reader;
		}
		
		private String[] readNextWithTimeout(long timeout) {
			Callable<String[]> readTask = new Callable<String[]>() {
		        @Override
		        public String[] call() throws Exception {
		            return reader.readNext();
		        }
		    };
		    Future<String[]> future = executor.submit(readTask);
		    try {
		    	return future.get(timeout, TimeUnit.SECONDS);
		    }
		    catch(Exception e) {
		    	return null;
		    }
		}


		protected boolean fillRow(Row row) throws Exception {
			String[] line = null;
			if (timeout == 0) {
				line = reader.readNext();
			} else {
				line = readNextWithTimeout(timeout);
			}
			if (line == null || line.length == staticColumns.size()) {
				try {
					if (timeout == 0) reader.close();
				}
				catch (Exception e) {};
				return false;
			}
			if (line.length > 0) {
				for (int i=0; i<line.length; i++) {
					String value =recoder.recode(line[i]);
					if (i < row.size()) {
						IColumn c = row.getColumn(i);
						if (c.getValueType().equals(Date.class.getCanonicalName()))
							c.setValue(dateFormat.parse(value));
						else 
							c.setValue(value);
					}
				}
				for (int i=Math.min(row.size(),line.length); i<row.size(); i++) {
					row.getColumn(i).setValue(null);
				}
			}
			return true;
		}

		protected Row getRow() {
			return row;
		}

		@Override
		protected void init() throws RuntimeException {
			this.row = new Row();
			row.addColumns(staticColumns);
		}
	}
	
	protected class BacklogHandler implements SyslogBackLogHandlerIF {
		
		private String errorMessage;

		@Override
		public void initialize() throws SyslogRuntimeException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void down(SyslogIF syslog, String reason) {
			log(syslog,SyslogIF.LEVEL_ERROR,"Syslog server is down",reason);
		}

		@Override
		public void up(SyslogIF syslog) {
			// TODO Auto-generated method stub
		}

		@Override
		public void log(SyslogIF syslog, int level, String message,
				String reason) throws SyslogRuntimeException {
			if (level <= SyslogIF.LEVEL_ERROR) {
				errorMessage = message+": "+reason;
			}
			
		}
		
		public boolean hasError() {
			return errorMessage != null;
		}
		
		public String getErrorMessage() {
			return errorMessage;
		}
		
	}
	
	protected class SyslogWriter implements IWriter {
		private int lines;
		private SyslogIF client;
		private BacklogHandler handler;
		
		public SyslogWriter(SyslogIF client, BacklogHandler handler) throws Exception {
			this.client = client;
			this.handler = handler;
		}

		public int getLinesOut() {
			return lines;
		}

		public void write(IProcessor rows) throws RuntimeException {
			Row row = rows.next();
			while (row != null) {
				List<String> values = row.getColumnValues();
				StringBuffer buf = new StringBuffer();
				for (int i=0; i<values.size()-1; i++) {
					buf.append(quote+values.get(i)+quote+delimiter);
				}
				buf.append(quote+values.get(values.size()-1)+quote);
				//fix for problem in special character encoding.
				try {
					int padding = buf.toString().getBytes(getEncoding()).length - buf.length();
					for (int i=0; i<padding; i++)
						buf.append(" ");
				} catch (UnsupportedEncodingException e) {
					e.printStackTrace();
				}
				client.log(SyslogIF.LEVEL_INFO,buf.toString());
				if (handler.hasError()) {
					throw new RuntimeException(handler.getErrorMessage());
				}
				lines++;
				/*
				if (lines % 4 == 0) {
					try {
						Thread.sleep(1);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				*/
				row = rows.next();
			}
			log.info(lines+" written to syslog.");
			client.info("");
			client.flush();
			client.shutdown();
		}
		
	}
	
	protected class MySyslogEventHandler implements SyslogServerEventHandlerIF {
		private static final long serialVersionUID = 3372362811689210674L;
		protected PrintStream stream = null;
		
		public MySyslogEventHandler(PrintStream stream) {
			this.stream = stream;
		}
		
		private String quote(String s, boolean withDelimiter) {
			StringBuffer buf = new StringBuffer();
			buf.append(quote);
			buf.append(s);
			buf.append(quote);
			if (withDelimiter)
				buf.append(delimiter);
			return buf.toString();
		}

		public synchronized void event(SyslogServerIF syslogServer, SyslogServerEventIF event) {
			String date = (event.getDate() == null ? new Date() : event.getDate()).toString();
			String facility = SyslogUtility.getFacilityString(event.getFacility());
			String level = SyslogUtility.getLevelString(event.getLevel());
			String host = event.getHost();
			String message = event.getMessage();
			StringBuffer buf = new StringBuffer();
			buf.append(quote(host,true));
			buf.append(quote(facility,true));
			buf.append(quote(date,true));
			buf.append(quote(level,!message.isEmpty()));
			buf.append(message);
			//if (message.startsWith(quote) && !message.endsWith(quote))
			//	buf.append(quote);
			stream.println(buf.toString());
			//System.err.println(buf.toString());
		}
	}

	SyslogServerIF server;
	private String delimiter = ";";
	private String quote ="\"";
	private PipedInputStream stream; 
	private String protocol = "udp";
	private static final Log log = LogFactory.getLog(SyslogConnection.class);

	@Override
	public void close() {
		if (stream != null) { 
			try {
				server.shutdown();
				stream.close();
			}
			catch (Exception e) {
				log.warn(e.getMessage());
			}
		}
	}

	@Override
	public synchronized InputStream open() throws RuntimeException {
		if (stream == null) {
			try {
				//to consider
				stream = new PipedInputStream();
				PipedOutputStream pipeOut = new PipedOutputStream();
				stream.connect(pipeOut);
				PrintStream logStream = new PrintStream(pipeOut);
				TCPNetSyslogServerConfig config = new TCPNetSyslogServerConfig();
				config.addEventHandler(new MySyslogEventHandler(logStream));
				if (getPort() != null) {
					config.setPort(Integer.parseInt(getPort()));
				} else {
					config.setPort(5556);
				}
				config.setCharSet(getEncoding());
				Class<?> syslogClass = config.getSyslogServerClass();	
				server = (SyslogServerIF) syslogClass.newInstance();
				server.initialize(protocol,config);
				Thread thread = new Thread(server);
				thread.setName("SyslogServer: " + getName());	
				server.setThread(thread);
				thread.start();
				return stream;
			}
			catch (Exception e) {
				throw new RuntimeException(e.getMessage());
			}
		}
		else
			return stream;
	}
	
	public IProcessor getProcessor(IAliasMap map, int size, long timeout) throws RuntimeException {
		//IAliasMap processorMap = map.clone();
		//shift aliases to make room for logger generated info. 
		//processorMap.shift(4);
		//define static colums
		Row staticColumns = new Row();
		Column host = new Column("_host");
		Column facility = new Column("_facility");
		Column date = new Column("_date");
		date.setValueType(Date.class);
		Column level = new Column("_level");
		staticColumns.addColumn(host);
		staticColumns.addColumn(facility);
		staticColumns.addColumn(date);
		staticColumns.addColumn(level);
		IProcessor processor = initProcessor(new StreamProcessor(open(), staticColumns, timeout),Facets.CONNECTION);
		//processor.current().addColumns(processorMap.getOutputDescription());
		return initProcessor(processor,Facets.CONNECTION);
	}
	
	
	
	public void init() throws InitializationException {
		try {
			super.init();
			delimiter = getConfigurator().getParameter("delimiter", delimiter);
			quote = getConfigurator().getParameter("quote", quote);
		}
		catch (ConfigurationException e) {
			throw new InitializationException(e);
		}
	}

	@SuppressWarnings("unchecked")
	public IWriter getWriter(boolean append) throws RuntimeException {
		try {
			TCPNetSyslogConfig config = new TCPNetSyslogConfig();
			if (getPort() != null)
				config.setPort(Integer.parseInt(getPort()));
			if (getHost() != null)
				config.setHost(getHost());
			config.setIdent("");
			config.setSendLocalName(false);
			config.setFacility(SyslogIF.FACILITY_USER);
			config.setMaxMessageLength(4096);
			config.setTruncateMessage(false);
			config.setCharSet(getEncoding());
			config.setWriteRetries(10);
			config.getBackLogHandlers().clear();
			BacklogHandler handler = new BacklogHandler();
			config.getBackLogHandlers().add(handler);
			Class<?> syslogClass = config.getSyslogClass();	
			SyslogIF client = (SyslogIF) syslogClass.newInstance();
			client.initialize(protocol,config);
			IWriter writer = new SyslogWriter(client,handler);
			return writer;
		}
		catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}
	}

	public void setWritable(boolean isWriteable) {
		//compatibility reasons. Nothing to to
	}
	
	public String getMetadata(Properties properties) throws RuntimeException {
		//TODO
		return "";
	}

	@Override
	public MetadataCriteria[] getMetadataCriterias() {
		// TODO Auto-generated method stub
		return new MetadataCriteria[0];
	}
}
