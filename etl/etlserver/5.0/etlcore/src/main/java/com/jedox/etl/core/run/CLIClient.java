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

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.OptionGroup;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.PosixParser;
import org.apache.commons.cli.UnrecognizedOptionException;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.PatternLayout;
import com.jedox.etl.core.logging.LogManager;
import java.util.Properties;
import java.io.FileOutputStream;
import java.io.PrintStream;

/**
 * This class provides a command line argument parser for the simple command Line Standalone Client
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 */
public class CLIClient {

	/* holds the parsed cli parameters. */
	private CommandLine cli = null;
	/* parsing succeeded */
	private boolean parseSucceeded =  true;
	/* logger instance. */
	private static final Log log = LogFactory.getLog(CLIClient.class);
	
	Options options;

	/**
	 * print help message for command line usage.
	 * 
	 * @param options
	 */
	public void showHelp() {
		HelpFormatter formatter = new HelpFormatter();
		formatter.printHelp("ETL-Server", options);
		System.exit(1);
	}

	/**
	 * Create a CLIClient instance. The constructor parses the given command
	 * line and the resulting instance provides convenience methods to access
	 * the given arguments.
	 * 
	 * @param args
	 */
	public CLIClient(String[] args) {
		Option previewopt = new Option("n", "sample", true,
				"Limitation of data from a source to sample of n. Valid only in combination with the -d option");
		previewopt.setArgName("size");
		Option projectopt = new Option("p", "project", true,
				"Specify the ETL-Project. If no further option is set the default job of the project is processed.");
		projectopt.setArgName("projectname");
		Option exportopt = new Option("l", "load", true,
				"Execute the specified load of the project. Multiple loads are separated with '.'. Valid only in combination with -p option.");
		exportopt.setArgName("exportname");
		exportopt.setArgs(99);
		exportopt.setValueSeparator('.');
		Option jobopt = new Option("j", "job", true,
				"Execute the specified job of the project. If no job is specified, the default job of a project is processed. Valid only in combination with the -p option.");
		jobopt.setArgName("jobname");
		jobopt.setValueSeparator('.');
		jobopt.setArgs(99);
		Option datasourceopt = new Option("d", "data", true,
				"Get data from the specified source. Intended for testing of sources (extracts and transforms). Valid only in combination with -p option.");
		datasourceopt.setArgName("sourcename");
		Option listopt = new Option("ls", "list", false, "List all components within a project given via the -p option");
		Option helpopt = new Option("h", "help", false,
				"Show this help message");
		Option controlopt = new Option("c", "context", true,
				"Context variables to use for execution of jobs, loads and sources");
		controlopt.setArgName("variables");
		controlopt.setArgs(99);
		Option validateopt = new Option("v","validate",true,"Validate project configuration without executing.");
		Option migrateopt = new Option("m","migrate",true,"Migrate project configuration to actual definition standard.");
		Option output = new Option("o","output",true,"Write output to a file");
		Option logopt = new Option("ll", "loglevel", true, "Set the log level. Possible values are: OFF, FATAL, ERROR, WARN, INFO, DEBUG, ALL");
		Option formatopt = new Option("f", "format", true, "Set the output format of a tree source. Valid only in combination with -p option.");
		Option testopt = new Option("t","test",true,"Validates and tests project configuration in runtime without actually writing data.");
		Option componentoutputopt = new Option("co","componentoutput",true,"Returns the output structure a source");
		Option dependopt = new Option("dg","depend",true,"Show the dependencies of a component");
		Option graphopt = new Option("gr","graph",true,"Get the flow graph of a component as SVG definition.");
		logopt.setArgName("level");
		OptionGroup og0 = new OptionGroup();
		og0.addOption(projectopt);
		og0.addOption(validateopt);
		og0.addOption(migrateopt);
		OptionGroup og1 = new OptionGroup();
		og1.addOption(testopt);
		og1.addOption(componentoutputopt);
		og1.addOption(dependopt);
		og1.addOption(graphopt);
		og1.addOption(exportopt);
		og1.addOption(jobopt);
		og1.addOption(datasourceopt);
		og1.addOption(listopt);
		OptionGroup og3 = new OptionGroup();
		og3.addOption(helpopt);
		options = new Options();
		options.addOptionGroup(og0);
		options.addOptionGroup(og1);
		options.addOptionGroup(og3);
		options.addOption(previewopt);
		options.addOption(formatopt);
		options.addOption(logopt);
		options.addOption(controlopt);
		options.addOption(output);
		
		addConsoleAppender();
		
		CommandLineParser parser = new PosixParser();
		try {
			cli = parser.parse(options, args);
			for(int i=0;i<args.length;i++){
				if( args[i].charAt(0)== '-' && !(options.hasOption(args[i]))){
					throw new UnrecognizedOptionException("Option " + args[i] + " is not recognised");
				}
			}
		} catch (ParseException e) {
			parseSucceeded = false;
			System.err.println(e.getMessage());
			System.err.println("use -h for a list of possible options");
			// showHelp();
		}
		if(parseSucceeded){
		StringBuffer line = new StringBuffer();
		for (String arg : args) {
			line.append(arg + " ");
		}
		log.info("Options: " + line.toString());
		}
	}

	private void addConsoleAppender() {
		ConsoleAppender console = new ConsoleAppender(new PatternLayout("%d %5p [%t] (%F:%L) - %m%n"));
		console.setTarget("System.err");
		LogManager.getInstance().addAppender(console);	
	}

	/**
	 * adds the xml postfix to a given project file name, if not already present
	 * @param name the project file name
	 * @return the postfixed project file name
	 */
	public String checkPostfix(String name) {
		if ((name != null) && (!name.endsWith(".xml")))
			name = name += ".xml";
		return name;
	}
	
	/**
	 * Determines if the runtime options are valid
	 * @return true, if so
	 */
	public boolean getParseSucceeded() {
		return parseSucceeded;
	}
	
	/**
	 * gets the postfixed project file name. 
	 * @return the project file name
	 */
	public String getFileName() {
		String p = checkPostfix(cli.getOptionValue("project"));
		if (p == null)
			p = checkPostfix(cli.getOptionValue("validate"));
		if (p == null)
			p = checkPostfix(cli.getOptionValue("migrate"));
		return p;
	}

	/**
	 * gets the project name (-p option value)
	 * @return
	 */
	public String getProjectName() {
		String p = cli.getOptionValue("project");
		return p;
	}

	/**
	 * gets the value for a given option
	 * @param optionName the name of the option
	 * @return the option value
	 */
	public String getComponentName(String optionName) {
		return cli.getOptionValue(optionName);
	}
	
	/**
	 * gets all specified loads (-l option values)
	 * @return the specified loads
	 */
	public String[] getLoads() {
		return cli.getOptionValues("load");
	}

	/**
	 * gets all specified jobs (-j option value)
	 * @return the specified jobs
	 */
	public String[] getJobs() {
		return cli.getOptionValues("job");
	}

	/**
	 * gets the specified data source (-d option value)
	 * @return the specified data source
	 */
	public String getData() {
		return cli.getOptionValue("data");
	}

	/**
	 * gets the sample size specified for a datasource
	 * @return
	 */
	public int getSample() {
		String preview = cli.getOptionValue("sample", "0");
		int sample = 0;
		try {
			sample = Integer.parseInt(preview);
		} catch (Exception e) {
			System.err.println("Preview Sample Size is not a number: "
					+ preview);
		}
		return sample;
	}

	/**
	 * Determines if should list project entities. (-l option)
	 * @return true, if so
	 */
	public boolean listEntities() {
		return cli.hasOption("list");
	}

	/**
	 * gets the context variables specified (-c option)
	 * @return the context variables
	 */
	public Properties getContext() {
		Properties result = new Properties();
		String[] parameter = cli.getOptionValues("context");
		if (parameter != null)
			for (String p : parameter) {
				String[] pair = p.split("=");
				if (pair.length == 2) 
					result.put(pair[0], pair[1]);
				else
					log.error("Invalid context parameter argument. Must be: name=value");
			}
		//return cli.getOptionValues("context");
		return result;
	}
	
	/**
	 * gets the context variables (-c option)
	 * @return the context variables
	 */
	public String[] getContextVariables() {
		return cli.getOptionValues("context");
	}

	/**
	 * Determines, if a project is specified (-p option)
	 * @return true, if so
	 */
	public boolean doProject() {
		return cli.hasOption("project");
	}
	
	/**
	 * Determines if a validation has to be done (-v option)
	 * @return true, if so
	 */
	public boolean doValidate() {
		return cli.hasOption("validate");
	}
	
	/**
	 * Determines, if a migration has to be done (-m option)
	 * @return true, if so.
	 */
	public boolean doMigrate() {
		return cli.hasOption("migrate");
	}
	
	/**
	 * gets the specified log level (-ll option)
	 * @return
	 */
	public String getLogLevel() {
		return cli.getOptionValue("loglevel", "INFO");
	}
	
	/**
	 * gets the format for a data source (-f option)
	 * @return the data source format
	 */
	public String getFormat() {
		return cli.getOptionValue("format");
	}
	
	/**
	 * Determines, if a test should be done (-t option)
	 * @return
	 */
	public boolean doTest() {
		return cli.hasOption("test");
	}

	/**
	 * Determines, if a output description should be returned (-od option)
	 * @return
	 */
	public boolean doComponentOutput() {
		return cli.hasOption("componentoutput");
	}
	
	
	/**
	 * Determines, if the dependencies of a component should be output (-dg option)
	 * @return
	 */
	public boolean doDepend() {
		return cli.hasOption("depend");
	}

	/**
	 * Determines, if the component overview graph of a component should be output
	 * @return
	 */
	public boolean doGraph() {
		return cli.hasOption("graph");
	}
		
	/**
	 * gets the Output Stream for result and logging data. Default is System.out, unless specified otherwise (-o option)
	 * @return
	 */
	public PrintStream getOutputStream() {
		if (cli.hasOption("output")) {
			try {
				return new PrintStream(new FileOutputStream(cli.getOptionValue("output")),true,"UTF8");
			}
			catch (Exception e) {
				log.error("Failed to write to file "+cli.getOptionValue("output")+": "+e.getMessage());
			}
		}
		return System.out;
	}
}
