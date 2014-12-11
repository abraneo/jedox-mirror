package com.jedox.etl.components.transform;


import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.rosuda.JRI.REXP;
import org.rosuda.JRI.Rengine;

import au.com.bytecode.opencsv.CSVReader;

import com.jedox.etl.components.config.transform.RConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.transform.ITransform;
import com.jedox.etl.core.util.FileUtil;
import com.jedox.etl.core.util.PersistenceUtil;
import com.jedox.etl.core.writer.CSVWriter;

public class RTransform extends TableSource implements ITransform  {
	
	private static final Log log = LogFactory.getLog(RTransform.class);
	
	private String script;
	private String dataset;
	private File bufferfile;
	private boolean logRCalls = false;
	private boolean useMemoryBuffer;

	
	private class RProcessor extends Processor {
		
		private Row row = new Row();
		private CSVReader reader;
		private REngineManager rEngineManager;
		
		public RProcessor()  {			
		}
		
		private void initReader() throws RuntimeException {
			if (useMemoryBuffer) {
				String result = getConsoleResult();
				if (!result.isEmpty()) {
					reader = new CSVReader(new StringReader(result), ',', '\"');
					try {
						row = PersistenceUtil.getColumnDefinition(reader.readNext());
					} catch (IOException e) {
						throw new RuntimeException(e);
					}
				}
			} else {
				try {
					reader = new CSVReader(new FileReader(bufferfile.getAbsolutePath()), ',', '\"');
					row = PersistenceUtil.getColumnDefinition(reader.readNext());
				} catch (IOException e) {
					throw new RuntimeException(e);
				}
			}
		}
		
		private boolean isPrintable(String dataset) throws RuntimeException {
			REXP x = evalR("is.data.frame("+dataset+")");
			if (x.asBool().isTRUE())
				return true;
			x = evalR("mode("+dataset+")");
			if (x.asString().equals("numeric") || x.asString().equals("character") || (x.asString().equals("logical")))
				return true;
			return false;
		}
		
		private String getConsoleResult() {
			return rEngineManager.getConsole().getWriter().toString().trim().replace("\n", "; ");
		}
		
		private REXP evalR(String s) throws RuntimeException {
			if (logRCalls)
				log.info(s);
			REXP rexp = rEngineManager.getRengine().eval(s);
			if (rEngineManager.getConsole().hasError() || rexp==null) {
				log.debug("R-Script Row: "+s);
				throw new RuntimeException("R-Script in transform "+getName()+" raised message: "+getConsoleResult());
			}
			return rexp;			
		}
		
		private CSVWriter getWriter(String filename, boolean append) throws RuntimeException {
			CSVWriter writer = null;
			try {
				writer = new CSVWriter(new OutputStreamWriter(new FileOutputStream(filename,append),"UTF8"));
				writer.setDelimiter(',');
				writer.setHeader(true);
				writer.setQuote("\"");
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to open file "+filename+" for writing: "+e.getMessage());
			}
			return writer;
		}

		@Override
		protected boolean fillRow(Row row) throws Exception {
			if (reader == null) return false;
			String[] line = reader.readNext();
			if (line == null) {
				reader.close();
				return false;
			}
			if (line.length > 0) {
				for (int i=0; i<Math.min(row.size(),line.length); i++) {
					String value = line[i];
					row.getColumn(i).setValue(value);
				}
				for (int i=Math.min(row.size(),line.length); i<row.size(); i++)
					row.getColumn(i).setValue(null);
			}
			return true;
		}

		@Override
		protected Row getRow() {
			return row;
		}

		protected void cleanUp(List<File> files) {
			for (File file : files) {
				file.delete();
			}
			// remove all objects from R. Console has to be reset before in order to clear isError
			rEngineManager.getConsole().reset();				
			try {
				evalR("rm(list = ls())");
			} catch (RuntimeException e) {
				log.error("Error on removing R objects: "+e.getMessage());
			}
			finally {			
				rEngineManager.getRengine().end();
				rEngineManager.getConsole().reset();
				// Release lock on the Rengine
				REngineManager.releaseInstance();
			}
		}
				
		@Override
		protected void init() throws RuntimeException {
			rEngineManager = REngineManager.getInstance();
			List<File> files = new ArrayList<File>();
			try {
				// Persisting sources to temporary File and read as table in R
		        for (ISource s : getSourceManager().getAll()) {
		        	String filename = Settings.getInstance().getDataDir()+File.separator+"r"+s.getLocator().toString().replace(".", "_")+s.getContextName()+System.currentTimeMillis()+".csv";
			        File file = new File(filename);
			        files.add(file);
			        CSVWriter writer = getWriter(filename,false);
			        writer.write(initProcessor(s.getProcessor(),Facets.INPUT));
			        evalR(s.getName()+" <- read.table(file=\""+file.getAbsolutePath().replaceAll("\\\\", "/")+"\", header=TRUE, sep = \",\")");
		        }
		        log.info("Starting R-Script of transform "+getName());
		        // Processing of R-Script row-wise
		        String rowBuffer = null;
				for (String s : script.split("\n")) {
					s=s.trim();
					if (s.isEmpty() || s.startsWith("#")) {
						continue;
					}						
					else if (rowBuffer==null) {
						rowBuffer=s;
					}	
					else if (s.startsWith("@")) {
						rowBuffer=rowBuffer.concat(s.substring(1));						
					}
					else {
						evalR(rowBuffer);						
						rowBuffer=s;
					}
				}
				if (rowBuffer==null)
					throw new RuntimeException("The script is empty.");
				evalR(rowBuffer);
		        log.info("Finished R-Script");
		        // Writing resulting data frame in R to temporary file
				if (!rEngineManager.getConsole().hasError()) {
					if (!isPrintable(dataset))
						throw new RuntimeException(dataset+" is not a valid R object for output. Only data frames and vectors can be used as return objects.");							
					evalR("write.table("+dataset+", stdout(), row.names=FALSE, col.names=TRUE, sep = \",\")");
					if (useMemoryBuffer) {
						rEngineManager.getConsole().setOutputEnabled(true);
						evalR("write.table("+dataset+", stdout(), row.names=FALSE, col.names=TRUE, sep = \",\")");
					} else {
						bufferfile = new File(Settings.getInstance().getDataDir()+File.separator+"r"+getLocator().toString().replace(".", "_")+getContextName()+System.currentTimeMillis()+".csv");						
						evalR("write.table("+dataset+", \""+bufferfile.getAbsolutePath().replaceAll("\\\\", "/")+"\", row.names=FALSE, col.names=TRUE, sep = \",\")");
					}
					initReader();					
				}	
			}
			catch (Exception e) {
				throw new RuntimeException(e.getMessage());
			}
			finally {
				cleanUp(files);
			}
		}	
	}
	
	public RTransform() {
		setConfigurator(new RConfigurator());
		notifyRetrieval = false;
	}

	public RConfigurator getConfigurator() {
		return (RConfigurator)super.getConfigurator();
	}
	
	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}
	
	public void invalidate() {
		super.invalidate();
		if (!useMemoryBuffer && bufferfile != null) 
			bufferfile.delete();
		bufferfile = null;
	}

	@Override
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(new RProcessor(),Facets.OUTPUT);
		processor.setLastRow(size);
		return processor;
	}
	
	
	private boolean isWindows() {
	      String os = System.getProperty("os.name");
	      if (os != null && os.startsWith("Windows"))
	         return true;
	      return false;
	}
	
		
	// perfrom checks for R and rJava installation
	private void checkRInstallation() throws Exception {
		String rPath=System.getenv("R_HOME");
		if (rPath==null)
		    throw new ConfigurationException("Installation of R is necessary for usage of transform "+getName()+". (Environment variable R_HOME not found.)");
		if (!FileUtil.fileExists(rPath))
		    throw new ConfigurationException("The environment variable R_HOME="+rPath+" does not point to a directory.");
		
		String model = System.getProperty("sun.arch.data.model");
		if (model==null) { 
			log.info("Could not check R installation. sun.arch.data.model is null");
		} else if (isWindows()){	
			String jriPath = FileUtil.convertPath(rPath + "/library/rJava/jri/" + (model.equals("32") ? "i386" : "x64"));
			if (!FileUtil.fileExists(FileUtil.convertPath(jriPath+"/jri.dll")))
				throw new ConfigurationException("The "+model+"-bit rJava package is not installed correctly. Missing file jri.dll in "+jriPath);
			if (System.getProperty("java.library.path").indexOf(jriPath)==-1)
				throw new ConfigurationException("The java.library.path does not include the path to the rJava package: "+jriPath);
			String pathToR = jriPath.replace(FileUtil.convertPath("library/rJava/jri"),"bin");
			String path = System.getenv("PATH");
			if (!path.contains(pathToR))
				throw new ConfigurationException("The PATH environment variable does to include the path to R: "+pathToR);
		}	
		//Set the property so that rJava does not make a System.exit(1)		
        // System.setProperty("jri.ignore.ule", "yes");
        
		// Explicitly try to load library jri.dll in order to catch Link error
        try {
            System.loadLibrary("jri");
        } catch (UnsatisfiedLinkError e) {
        	throw new ConfigurationException("Error in loading JRI library: "+e.getMessage());
        }
        //jriLoaded is false is rJava did not find jri library        
       	if (!Rengine.jriLoaded)
       		throw new ConfigurationException("JRI Engine is not loaded.");
       	log.debug("Rengine version: "+Rengine.getVersion()+" - RNI version: "+Rengine.rniGetVersion());
		if (!Rengine.versionCheck())
			throw new ConfigurationException("R-Version mismatch - Java files don't match library version.");		
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new SourceManager());
			getSourceManager().addAll(getConfigurator().getSources());
			script = getConfigurator().getScript();
			dataset = getConfigurator().getDataset();
			useMemoryBuffer = getConfigurator().useMemoryBuffer();
			
			checkRInstallation();
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
