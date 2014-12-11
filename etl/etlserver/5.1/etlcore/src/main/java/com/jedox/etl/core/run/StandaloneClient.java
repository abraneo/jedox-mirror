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
package com.jedox.etl.core.run;

import java.io.File;
import java.io.PrintStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.MalformedURLException;
import java.text.DecimalFormat;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import javax.net.ssl.HttpsURLConnection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.WriterAppender;
import org.apache.log4j.spi.LoggingEvent;
import org.jdom.Element;
import org.jdom.Document;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.ConfigConverter;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.XMLReader;
import com.jedox.etl.core.connection.InternalConnectionManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.ExecutionDetail;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.execution.Executor;
import com.jedox.etl.core.execution.Execution;
import com.jedox.etl.core.execution.ResultCodes.DataTargets;
import com.jedox.etl.core.logging.LogManager;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.project.IProject;
import com.jedox.etl.core.prototype.PrototypeGenerator;
import com.jedox.etl.core.util.NoHostnameVerifier;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.core.util.svg.GraphManager;
/**
 * A simple Standalone command line client for etl executions
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class StandaloneClient {

	private static final Log log = LogFactory.getLog(StandaloneClient.class);

	private CLIClient cli;
	private ExecutionLog elog;
	private ExecutionState state = null;
	
	
	private class ExecutionLog extends WriterAppender {

		private boolean isClosed = false;
		private int warnings = 0;
		private int errors = 0;

		public ExecutionLog() {
//			previous layout: "%d %5p [%t] (%F:%L) - %m%n"
			super(new PatternLayout("%d %5p : %m%n"), new java.io.StringWriter());
			Logger.getRootLogger().addAppender(this);
			this.setThreshold(Level.INFO);
		}
		
		private void addError() {
			errors++;
		}
		
		private void addWarning() {
			warnings++;
		}
		
		public int getErrors() {
			return errors;
		}
		
		@SuppressWarnings("unused")
		public int getWarnings() {
			return warnings;
		}

		/** @see org.apache.log4j.WriterAppender#subAppend(LoggingEvent) */
		protected void subAppend( LoggingEvent event ) {
			//test if sender acts in the same executive thread 
			if (!isClosed) {
				// Write all Info messages, write up to logLimit Warning and Error Messages	
			    if (event.getLevel().equals(Level.WARN)) {
			    	addWarning();
			    }
			    if (event.getLevel().equals(Level.ERROR)) {
			    	addError();
			    }
			    if (event.getLevel().equals(Level.FATAL)) {
			    	addError();
			    }
			}
		}
			
		/**
		 * closes this Appender and removes it from the {@link LogManager}
		 */
		public void close() {
			if (!isClosed) {
				isClosed = true;
				Logger.getRootLogger().removeAppender(this);
			}
		}
	}

	

	protected StandaloneClient(CLIClient cli) {
		this.cli = cli;	
		LogManager.getInstance().setLevel(cli.getLogLevel(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("loglevel")));
		LogManager.getInstance().setDebugClasses(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("logDebugClasses"));
		elog = new ExecutionLog();
		// removeHsqlFiles();
	}

	private PrintStream getOutput() {
		return cli.getOutputStream();
	}
	
	private int getExecutionCode(ExecutionState state) {
		return Integer.parseInt(state.getNumeric(state.getStatus()));
	}

	private String getMessage(Exception e) {
		/*
		Throwable t = e;
		while (t.getCause() != null) {
			t = t.getCause();
		}
		return t.toString();
		*/
		return e.getMessage();
	}

	private void shutdown() {
		// Shutdown
		try {
			log.debug("Shutting down Standalone-Client ...");
			InternalConnectionManager.getInstance().shutDown();
			log.debug("Shut-Down finished...");
			LogManager.getInstance().shutdown();
			// Persistence.getInstance().disconnect();
			elog.close();
			if (state != null) System.exit(getExecutionCode(state));
			if (elog.getErrors() > 0) System.exit(-2);
			System.exit(0);
		} catch (Exception e) {
			System.exit(-1);
		}
	}

/*	private void removeHsqlFiles() {
		//always start with fresh database when using hsql in standalone mode!
		File file = new File("./db/hsqldb/internal.lck");
		file.delete();
		file = new File("./db/hsqldb/internal.data");
		file.delete();
		file = new File("./db/hsqldb/internal.backup");
		file.delete();
		try {
			FileUtil.copyFile(new File("./db/hsqldb/internal.script.start"),new File("./db/hsqldb/internal.script"));
		} catch (IOException e) {
		}
	}
*/
	/*
	private void printData(ExecutionState result) {
		System.err.flush();
		getOutput().println(result.getData());
	}
	*/

	private void printStats(ExecutionState result) {
		System.err.flush();
		getOutput().println("Finished execution with status: "+result.getString(result.getStatus()));
		getOutput().println("Errors: "+result.getErrors());
		getOutput().println("Warnings: "+result.getWarnings());
		if ((result.getStartDate() != null) && (result.getStopDate() != null))
			getOutput().println("Duration: "+(result.getStopDate().getTime()-result.getStartDate().getTime())/1000.0+" seconds");
		getOutput().flush();
		printDetails(result);
	}
	
	private String getSeconds(long time) {
		return new DecimalFormat("###.##").format(((double)time/1000000)) + "s";
	}
	
	private void printDetails(ExecutionState result) {
		getOutput().println("Details:");
		for (ExecutionDetail d : result.getDetails().values()) {
			Locator l = Locator.parse(d.getLocator());
			getOutput().println(l.getManager().substring(0, l.getManager().length()-1)+" "+l.getName()+": "+getSeconds(d.getRuntime())+" In: "+d.getProcessedInputRows()+" Out: "+d.getProcessedOutputRows()+" InCalls: "+d.getInputCalls()+" OutCalls: "+d.getOutputCalls());
		}
		long totalTime = 0;
		for (ExecutionDetail d : result.getDetails().values()) {
			totalTime += d.getRuntime();
		}
		System.out.println("Total: "+getSeconds(totalTime));
		getOutput().flush();
	}

	private void printComponentOutput(ExecutionState result) {
		System.err.flush();		
		if(result.getErrors()==0){
			for (IColumn column : result.getMetadata().getColumns()) {
				String output=column.getName()+" ; "+column.getValueType()+" ; "+ ((column.getAliasElement() != null) ? column.getAliasElement().getOrigin() : column.getName());
				getOutput().println(output);
			}
		}
		getOutput().flush();
	}
		
	private void printDependencies(String qname, Map<String,List<String>> graph, String indent) {
		List<String> list = graph.get(qname);
		if (list != null) {
			for (String s : list) {
				getOutput().println(indent+qname+" <- "+s);
				printDependencies(s,graph,indent+"  ");
			}
		}
	}

	private void printDependants(String qname, Map<String,List<String>> graph, String indent) {
		List<String> list = graph.get(qname);
		if (list != null) {
			for (String s : list) {
				getOutput().println(indent+qname+" -> "+s);
				printDependants(s,graph,indent+"  ");
			}
		}
	}

	protected int run() {
		String project = cli.getFileName();
		String[] loads = cli.getLoads();
		String[] jobs = cli.getJobs();
		String data = cli.getData();
		String dataFormat = cli.getDataFormat();
		Properties context = cli.getContext();
		int sample = cli.getSample();
		int offset = cli.getOffset();
		String format = cli.getFormat();
		try {
			//if no project is specified show the help dialog
			if (project == null) cli.showHelp();
						
			if (cli.doPrototype()) {		
			   String proj = PrototypeGenerator.getInstance().generate(cli.getComponentName("prototype"), cli.getProjectName(), context);
			   getOutput().println(proj);
			}   
			
			// projects are not saved to disk in standalone mode
			Settings.getInstance().setAutoSave(false);

			//reconfigure project dir in Settings class based on project path
			File pf = new File(project);
			if (pf.exists()) {
				File parent = pf.getParentFile();
				if (parent != null)
					Settings.getInstance().setProjectDir(parent.getName());
				else
					Settings.getInstance().setProjectDir("");
			}

			// sets the keystore path
			try{
				 Properties props = new Properties();
				 File propFile = new File("config/ssl.properties");

					try {
						props.load(propFile.toURI().toURL().openStream());
					} catch (MalformedURLException e) {
						log.debug("Try to open SSL connecion " + e.getMessage());
					} catch (IOException e) {
						log.debug("Try to open SSL connecion " + e.getMessage());
					}


				 for (Object prop: props.keySet()) {
				     System.getProperties().setProperty((String)prop, props.getProperty((String)prop));
				 }
			}catch(Exception ioe){
				log.debug("SSL properties file is not found under the expected folder," + ioe.getMessage());
			}

			//read project file
			XMLReader reader = new XMLReader();
			Document document = reader.readDocument(project);
			Element root = (Element) document.getRootElement().clone();
			String projectName = root.getAttributeValue("name");

			if (projectName == null) {
				log.error("Project from file "+project+" must have an attribute 'name'");
				shutdown();
			}

			if (projectName.matches(".*\\..*")) {
				log.error("Invalid project name "+projectName+" in xml: The character . is not allowed");
				shutdown();
			}

			if (cli.doMigrate()) {
				ConfigConverter converter = new ConfigConverter();				
				getOutput().println(XMLUtil.jdomToString(converter.convert(document.getRootElement())));
				shutdown();
			}

			//register project for commands below.
			ConfigManager.getInstance().add(Locator.parse(projectName), root);
			IProject prj = (IProject) ConfigManager.getInstance().getProject(projectName);
			if (prj == null) {
				log.error("Error on configuration of project " + projectName);
				shutdown();
			}

			if (cli.doValidate()) {
				Locator loc = Locator.parse(projectName);
				ConfigManager.getInstance().add(loc, root);
				ConfigManager.getInstance().validate(loc);
				getOutput().println("Validation of project "+projectName+" successful.");
				shutdown();
			}

			if (cli.listEntities()) {
				getOutput().println("Project: " + prj.getName());
				getOutput().println("  Extracts:");
				for (String s : ConfigManager.getInstance().getNames(Locator.parse(projectName).add(ITypes.Extracts))) {
					getOutput().println("    " + s);
				}
				getOutput().println("  Transforms:");
				for (String c : ConfigManager.getInstance().getNames(Locator.parse(projectName).add(ITypes.Transforms))) {
					getOutput().println("    " + c);
				}
				getOutput().println("  Loads:");
				for (String e : ConfigManager.getInstance().getNames(Locator.parse(projectName).add(ITypes.Loads))) {
					getOutput().println("    " + e);
				}
				getOutput().println("  Jobs:");
				for (String j : ConfigManager.getInstance().getNames(Locator.parse(projectName).add(ITypes.Jobs))) {
					getOutput().println("    " + j);
				}
			} else if (cli.doTest()) {
				Execution e = Executor.getInstance().createTest(Locator.parse(cli.getComponentName("test")),context);
				e.getExecutionState().setDataTarget(DataTargets.CSV_STDOUT); //write directly to stdout
				Executor.getInstance().addExecution(e);
				Executor.getInstance().runExecution(e.getKey());
				state = Executor.getInstance().getExecutionState(e.getKey(), true);
				printStats(state);
			} else if (cli.doComponentOutput()) {
				Execution e = Executor.getInstance().createOutputDescription(Locator.parse(cli.getComponentName("componentoutput")),context,format);
				e.getExecutionState().setDataTarget(DataTargets.CSV_STDOUT); //write directly to stdout
				Executor.getInstance().addExecution(e);
				Executor.getInstance().runExecution(e.getKey());
				state = Executor.getInstance().getExecutionState(e.getKey(), true);
				printComponentOutput(state);
			} else if (cli.doDepend()) {
				//init project as component in default context.
				prj = (IProject) ConfigManager.getInstance().getComponent(Locator.parse(projectName),null);
				getOutput().println("Dependency Subgraph:");
				String qname = cli.getComponentName("depend");
				printDependencies(qname,prj.getAllDependencies(qname,true),"");
				getOutput().println("Dependents Subgraph:");
				printDependants(qname,prj.getAllDependents(qname,true),"");
			} else if (cli.doGraph()) {
				prj = (IProject) ConfigManager.getInstance().getProject(projectName);
				List<Locator> invalidComponents = ConfigManager.getInstance().initProjectComponents(prj, IContext.defaultName, true); 
				getOutput().println("Component Overview Graph:");
				String qname = cli.getComponentName("graph");	
				getOutput().println(GraphManager.getInstance().getSVG(prj, qname, context, invalidComponents,Long.MIN_VALUE));
			} else if (data != null) {
				Execution e = Executor.getInstance().createData(Locator.parse(projectName).add(ITypes.Sources).add(data), context,format);
				e.getExecutionState().setDataTarget((dataFormat==null|| dataFormat.equals("csv"))?DataTargets.CSV_STDOUT:DataTargets.XML_STDOUT); //write directly to stdout
				e.getExecutable().getParameter().setProperty("sample", String.valueOf(sample));
				if(offset<0)
					throw new Exception("Minimum value of offset is 1.");
				if(offset>0)
					offset--;

				e.getExecutable().getParameter().setProperty("offset", String.valueOf(offset));
				Executor.getInstance().addExecution(e);
				Executor.getInstance().runExecution(e.getKey());
				state = Executor.getInstance().getExecutionState(e.getKey(), true);
				printDetails(state);
			} else if (jobs != null) {
				for (String job : jobs) {
					Execution e = Executor.getInstance().createExecution(Locator.parse(projectName).add(ITypes.Jobs).add(job),context);
					Executor.getInstance().addExecution(e);
					Executor.getInstance().runExecution(e.getKey());
					state = Executor.getInstance().getExecutionState(e.getKey(), true);
					printStats(state);
				}
			} else if (loads != null) // we are doing exports
			{
				for (String export : loads) {
					Execution e = Executor.getInstance().createExecution(Locator.parse(projectName).add(ITypes.Loads).add(export),context);
					Executor.getInstance().addExecution(e);
					Executor.getInstance().runExecution(e.getKey());
					state = Executor.getInstance().getExecutionState(e.getKey(), true);
					printStats(state);
				}
			} else { // we are doing the default job of the project
				Execution e = Executor.getInstance().createExecution(Locator.parse(projectName).add(ITypes.Jobs).add("default"),context);
				Executor.getInstance().addExecution(e);
				Executor.getInstance().runExecution(e.getKey());
				state = Executor.getInstance().getExecutionState(e.getKey(), true);
				printStats(state);
			}
			shutdown();
		}
		catch (ConfigurationException e) { //from XMLReader
			@SuppressWarnings("unused")
			String absolutePath = new File(project).getAbsolutePath();
			if (e.getCause() instanceof FileNotFoundException)
				log.error("File not found: "+getMessage(e));
			else
				log.error(getMessage(e));
		}
		catch (Exception e) {
			log.error(getMessage(e));
			//e.printStackTrace();
			log.debug("",e);
		}
		return -1;
	}


/*	private static void printTree(com.jedox.etl.core.node.TreeManager manager,  com.jedox.etl.core.node.TreeElement element, String indent) {
		System.out.println(indent+element.getName());
		indent = " "+indent;
		for (com.jedox.etl.core.node.IColumn c : manager.getChildren(element).getColumns()) {
			printTree(manager,((com.jedox.etl.core.node.TreeNode)c).getElement(),indent);
		}
	}
*/
	/**
	 * ETL-Server Main Class. Use of Arguments: see {@link CLIClient}
	 *
	 * @param args: the arguments
	 *
	 */
	public static void main(String[] args) {

		CLIClient cliClient = new CLIClient(args);
		HttpsURLConnection.setDefaultHostnameVerifier(new NoHostnameVerifier());
		if(cliClient.getParseSucceeded()){
			StandaloneClient client = new StandaloneClient(cliClient);
			System.exit(client.run());
		}
		else
		{
			System.exit(0);
		}
	}

}
