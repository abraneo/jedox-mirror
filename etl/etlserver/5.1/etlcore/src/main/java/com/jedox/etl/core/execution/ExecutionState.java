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
package com.jedox.etl.core.execution;

import java.sql.Timestamp;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Level;
import org.apache.log4j.LogManager;
import org.apache.log4j.spi.LoggingEvent;

import com.jedox.etl.core.config.OLAPAuthenticator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.IManager;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.execution.Execution.ExecutionTypes;
import com.jedox.etl.core.logging.ExecutionLog;
import com.jedox.etl.core.logging.ILogListener;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.hibernate.IPersistable;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.util.NamingUtil;

import javax.persistence.*;

/**
 * Persistent class, which holds the state information of an {@link Execution}. All components involved in an execution (the dependencies of the executable) share the same state. All logging information of these components is available to the state via the {@link com.jedox.etl.core.logging.ExecutionLog}.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

@Entity
public class ExecutionState extends ResultCodes implements IPersistable {

	@Transient
	private static final Log log = LogFactory.getLog(ExecutionState.class);
	@Transient
	private boolean isExecutable = true;

	@TableGenerator(name = "StateIdTable", allocationSize = 100)
	@Id @GeneratedValue(strategy=GenerationType.TABLE, generator="StateIdTable")
	//@Id @GeneratedValue(strategy=GenerationType.TABLE)
	@Column(name = "RESULT_ID")
	private Long id;
	private int errors = 0;
	private int warnings = 0;
	private Date startDate;
	private Date stopDate;
	private Codes status = Codes.statusQueued;
	private String project;
	private String type;
	private String userName;
	private String name;
	@Column(length = 1000)
	private String firstErrorMessage;
	@Enumerated(EnumType.ORDINAL) 
	private ExecutionTypes executionType;
	@Transient
	private int infos = 0;
	@Transient
	private ILogListener messages;
	@Transient
	private Row metadata;
	@Transient
	private DataTargets dataTarget = DataTargets.CSV_INLINE;
	@Transient
	private String data;
	@Transient
	private boolean archive = true;
	@Transient
	private ExecutionLog elog;
	@Transient
	private Map<String,Integer> logTypesLimits = new LinkedHashMap<String,Integer>();
	@Transient 
	private Map<String,ExecutionDetail> details = new LinkedHashMap<String,ExecutionDetail>();	
	@Transient
	private List<ISource> sourcesToClean = new ArrayList<ISource>();


	public ExecutionState() {
	}

	public ExecutionState(Locator locator) {
		setProject(locator.getRootName());
		setType(locator.getManager());
		setName(locator.getName());
		setUserName(OLAPAuthenticator.getInstance().getUserInfo(locator.getSessioncontext()).getName());
		setStartDate(new Date());
	}
	
	public void init() {
		setStartDate(new Date());
		if (archive) messages = new MessageWriter(id);
		elog = new ExecutionLog(this);
	}

	/**
	 * opens the ExecutionState and sets its state to "running".
	 * @param context the context the execution is running in.
	 * @param threadName the name of the execution thread, which may access this state.
	 */
	public void open(IContext context, boolean dependent) {
		try {
			logTypesLimits.put("INFO", Integer.parseInt(context.getParameter().getProperty("#logLimitInfo")));
			logTypesLimits.put("WARN", Integer.parseInt(context.getParameter().getProperty("#logLimitWarn")));
			logTypesLimits.put("ERROR", Integer.parseInt(context.getParameter().getProperty("#logLimitError")));
		}
		catch (Exception e) {
			log.warn("Failed to set log limit from context.");
		}
		if (dependent) {
			logSettings("Changed Variables",context.getExternalVariables(),false);			
		} 
		else {		
			logSettings("Variables",context.getVariables(),false);
			setStartDate(new Date());
			setStatus(Codes.statusRunning);
			logSettings("Execution parameters",context.getParameter(),true);
		}	
	}

	private void logSettings (String type, Properties prop, boolean showExecutionParams) {
		Iterator<Object> i = prop.keySet().iterator();
		String list = "";
		int maxlength=800; // Maximal length of Log-Message
		while (i.hasNext()) {
			String key = i.next().toString();
			if ((!showExecutionParams && key.startsWith(NamingUtil.internalPrefix())) || key.startsWith(NamingUtil.hiddenInternalPrefix())) {
			   continue;
			}
			String s = key+":"+prop.getProperty(key)+"; ";	
			if (list.length()!=0 && list.length()+s.length()>maxlength) {
				log.info(type+": "+list);
				list="";
			}			
			list = list.concat(s);
		}
		if (!list.isEmpty()) {
			log.info(type+": "+list);
		}	
	}
	
	
	/**
	 * closes the ExecutionState and sets its state accordingly to what has happened during the execution, which is all reflected in this ExecutionState objects properties.	 *
	 * initialWarnings/initialErrors is only relevant for dependent jobs and indicates the number of Warnings/Errors since last opening of Execution State 
	 */
	public Codes close(boolean dependent, int initialWarnings, int initialErrors, IContext context) {
		Codes code;
		if (getStatus().equals(Codes.statusStopped) || getStatus().equals(Codes.statusInterrupted)) {
			code = getStatus(); 
		}
		else { //execution is not already stopped.
			code = Codes.statusOK;
			if (getWarnings() > 0) code = Codes.statusWarnings;
			if (getErrors() > 0) code = Codes.statusErrors;
			//if aborted on errors
			if (!isExecutable()) code = Codes.statusFailed;
			if (getStatus().equals(Codes.statusStopping)) code = Codes.statusStopped;
			if (getStatus().equals(Codes.statusInterrupting) ||(getStatus().equals(Codes.statusStopping) && Thread.currentThread().isInterrupted())) code = Codes.statusInterrupted;
		}
		if (!dependent) {
			setStopDate(new Date());
			setExecutable(false);
			setStatus(code);
		} 
		else {
			// the returned Status for dependent subjobs is Error/Warning only if since start of subjob a new Error/Warning occured 
			if (code.equals(Codes.statusErrors)) {
				if (initialErrors==getErrors() && initialWarnings==getWarnings())
					code=Codes.statusOK;
				else if (initialErrors==getErrors())
					code=Codes.statusWarnings;				
			}
			else if (code.equals(Codes.statusWarnings)) {
				if (initialWarnings==getWarnings())
					code=Codes.statusOK;
			}	
		}
		//if (context != null) addDetails(context.getExecutionDetails()); cschw: done in context.clear() now 
		return code;
	}
	
	public void addDetails(Map<String,ExecutionDetail> contextDetails) {
		for (ExecutionDetail d : contextDetails.values()) {
			ExecutionDetail summedDetail = details.get(d.getLocator()); 
			if (summedDetail != null) {
				summedDetail.setProcessedInputRows(summedDetail.getProcessedInputRows()+d.getProcessedInputRows());
				summedDetail.setProcessedOutputRows(summedDetail.getProcessedOutputRows()+d.getProcessedOutputRows());
				summedDetail.setRuntime(summedDetail.getRuntime()+d.getRuntime());
				summedDetail.setInputCalls(summedDetail.getInputCalls()+d.getInputCalls());
				summedDetail.setOutputCalls(summedDetail.getOutputCalls()+d.getOutputCalls());
			} else {
				d.setResultId(getId());
				details.put(d.getLocator(), d);
			}
		}
	}

	/**
	 * sets the state to "stopped" and the {@link #isExecutable()} to false. So all components sharing this state and periodically checking this flag know, that the execution is stopped and all processing within the components should be terminated cleanly.
	 */
	public void stop() {
		log.info("Execution "+getId()+" is stopped.");
		setExecutable(false);
		setStatus(Codes.statusStopping);
	}

	/**
	 * like {@link #stop()} but sets the state to aborted, indicating that the execution never started to process.
	 */
	public void abort() {
		if (isExecutable()) {
			log.info("Execution "+getId()+" is aborted.");
			setExecutable(false);
		    messages.flush();
		}
		setStatus(Codes.statusAborted);
	}

	/**
	 * sets the {@link #isExecutable()} flag, indication whether components sharing this state should cleanly terminate executing.
	 * @param isExecutable false if execution is to be stopped.
	 */
	public void setExecutable(boolean isExecutable) {
		if(isExecutable==false)
			fireExecutionStopping(this);
		this.isExecutable = isExecutable;
	}

	/**
	 * Determines if the components sharing this state shall be continuing executing or terminate cleanly. components should periodically query this state property in a way to achieve transaction save execution.
	 * @return true if execution shall go on, false if it should be stopped.
	 */
	public boolean isExecutable() {
		return isExecutable;
	}

	/**
	 * gets the number of errors, which occurred while executing.
	 * @return the number of errors.
	 */
	public int getErrors() {
		return errors;
	}

	/**
	 * adds a number of errors to the error count.
	 * @param errors the number of errors to add.
	 */
	public void addErrors(int errors) {
		this.errors += errors;
	}

	/**
	 * gets the number of warnings, which occurred while executing
	 * @return the number of warnings
	 */
	public int getWarnings() {
		return warnings;
	}

	/**
	 * adds a number of warnings to the warning count
	 * @param warnings the number of warnings to add
	 */
	public void addWarnings(int warnings) {
		this.warnings += warnings;
	}
	
	/**
	 * adds a number of infos to the info count
	 * @param infos the number of infos to add
	 */
	public void addInfos(int infos) {
		this.infos += infos;
	}
	
	public int getInfos() {
		return infos;
	}

	/**
	 * gets the meta-data (column description). This only works if the executable is of type {@link com.jedox.etl.core.source.ISource} and produces some date while executing.
	 * @return the source meta-data
	 */
	public Row getMetadata() {
		return metadata;
	}

	/**
	 * sets the meta-date (column description).
	 * @param metadata
	 */
	public void setMetadata(Row metadata) {
		this.metadata = metadata;
	}

	/**
	 * gets the DataTarget of an executing {@link com.jedox.etl.core.source.ISource}. This describes, where the produced data is written to (inline in this state, to a console or to a datastore)
	 * @return the DataTarget
	 */
	public DataTargets getDataTarget() {
		return dataTarget;
	}

	/**
	 * sets the DataTarget of an executing {@link com.jedox.etl.core.source.ISource}
	 * @param dataTarget
	 */
	public void setDataTarget(DataTargets dataTarget) {
		this.dataTarget = dataTarget;
	}

	/**
	 * gets the data an executable writes to this state. Currently used only if the executable is of type {@link com.jedox.etl.core.source.ISource}.
	 * @return the data
	 */
	public String getData() {
		return data;
	}

	/**
	 * sets the data. Currently used only if the executable is of type {@link com.jedox.etl.core.source.ISource} to store the produced data.
	 * @param data the data to set.
	 */
	public void setData(String data) {
		this.data = data;
	}

	/**
	 * gets the start date.
	 * @return the start date
	 */
	public Date getStartDate() {
		return startDate;
	}

	/**
	 * sets the start date
	 * @param startDate the start date
	 */
	public void setStartDate(Date startDate) {
		this.startDate = startDate;
	}

	/**
	 * sets the stop date
	 * @param stopDate the stop date
	 */
	public void setStopDate(Date stopDate) {
		this.stopDate = stopDate;
	}

	/**
	 * gets the stop date
	 * @return the stop date
	 */
	public Date getStopDate() {
		return stopDate;
	}

	/**
	 * gets the status code
	 * @return the status code
	 */
	public Codes getStatus() {
		return status;
	}

	/**
	 * sets the status code
	 * @param status the status code
	 */
	public void setStatus(Codes status) {
		this.status = status;
	}

	/**
	 * gets the unique id of this state
	 * @return the unique id
	 */
	public Long getId() {
		return id;
	}

	/**
	 * sets the unique id of this state
	 * @param id the unique id
	 */
	public void setId(Long id) {
		this.id = id;
	}

	/**
	 * gets the name of the project
	 * @return the project name
	 */
	public String getProject() {
		return project;
	}

	/**
	 * sets the name of the project
	 * @param project the project name
	 */
	public void setProject(String project) {
		this.project = project;
	}

	/**
	 * gets the type of the executable of this state
	 * @return the executable type (e.g. "job")
	 */
	public String getType() {
		return type;
	}

	/**
	 * sets the type of the executable of this state
	 * @param type the executable type
	 */
	public void setType(String type) {
		this.type = type;
	}

	/**
	 * gets the name of the executable
	 * @return the name of the executable
	 */
	public String getName() {
		return name;
	}

	/**
	 * sets the name of the executable
	 * @param name the name of the executable
	 */
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * return the first error message of the log (if any is present)
	 * @return the first error message
	 */
	public String getFirstErrorMessage() {
		return firstErrorMessage;
	}

	/**
	 * sets the first error message
	 * @param error the message
	 */
	public void setFirstErrorMessage(String error) {
		this.firstErrorMessage = error;
	}
	
	public void setExecutionType(ExecutionTypes executionType) {
		this.executionType = executionType;
	}

	public ExecutionTypes getExecutionType() {
		return executionType;
	}
	
	private int countMessagesOfType(String type) {
		if ("INFO".equalsIgnoreCase(type)) return getInfos();
		if ("WARN".equalsIgnoreCase(type)) return getWarnings();
		if ("ERROR".equalsIgnoreCase(type)) return getErrors();
		if ("FATAL".equalsIgnoreCase(type)) return 0;// always write it
		return Integer.MAX_VALUE;
	}
	
	/**
	 * adds a message wrapping a log entry
	 * @param type the type of the message (e.g. error, warning, ...)
	 * @param timestamp the timestamp of the message
	 * @param message the message text
	 */
	public void addMessage(String type, Long timestamp, String message) {
		int count = countMessagesOfType(type);
		int logLimit = getLogTypeLimit(type);
		if(message.length()>1000){
			//log.info("shrinking the message length to 900");
			message = message.substring(0, 900);
		}
		if (messages != null) {
			if (count == logLimit) {
				LoggingEvent event = createLoggingEvent(type,"LogLimit for "+type+" messages exeeded limit " + logLimit + ". Ignoring further messages of type "+type+".");
				messages.addMessage(type, timestamp, elog.getLayout().format(event));
			}
			if (count < logLimit)
				messages.addMessage(type, timestamp, message);
		}
		if(Level.toLevel(type).isGreaterOrEqual(Level.WARN) && getErrors() == 0){// no error message is stored but may be warning
			if(getWarnings() == 0 && type.equals("WARN")){// no warning message too
					setFirstErrorMessage(message.substring(message.indexOf("WARN :") + 6)); // just to chop the the date and the code line info and give the only the message
			}else if(Level.toLevel(type).isGreaterOrEqual(Level.ERROR)){
					setFirstErrorMessage(message.substring(message.indexOf("ERROR :") + 7)); // just to chop the the date and the code line info and give the only the message
			}else{}
		}
	}
	
	private LoggingEvent createLoggingEvent(String level, String message) {
		return new LoggingEvent(this.getClass().getCanonicalName(),LogManager.getLogger(this.getClass()),Level.toLevel(level),message,null);
	}

	/**
	 * @param type
	 * @return
	 */
	private int getLogTypeLimit(String type) {
		Integer limit = logTypesLimits.get(type);
		return (limit!= null?limit:Integer.MAX_VALUE-1);
	}

	/**
	 * Adds a message to the log of the execution without using the Logger
	 * @param type the type of the message (e.g. error, warning, ...)
	 * @param message  the message text
	 */
	public void addMessageExternal(String type, String message) {
		long currentTime = System.currentTimeMillis();
		/*
		// Apply same format to message text as defined in PatternLayout of ExecutionLog
		// Format: "%d %5p : %m%n"
		ISO8601DateFormat format = new ISO8601DateFormat();
		StringBuffer sbuf = new StringBuffer();
		format.format(new Date(currentTime),sbuf,null);
		sbuf.append("  ").append(type).append(" : ").append(message).append("\r");
		*/
		LoggingEvent event = createLoggingEvent(type, message);
		// Add the message for the Execution
		addMessage(type,new Timestamp(currentTime).getTime(),elog.getLayout().format(event));
		if(Level.toLevel(type).equals(Level.WARN))
			this.addWarnings(1);
		else if((Level.toLevel(type).isGreaterOrEqual(Level.ERROR)))
			this.addErrors(1);
		// Put the message also to Log
		log.info("Message added: "+message);
		messages.flush();
	}	

	public void setArchive(boolean archive) {
		this.archive = archive;
	}

	public boolean isArchive() {
		return archive;
	}

	public ExecutionLog getExecutionLog() {
		return elog;
	}

	/**
	 * @param username the username to set
	 */
	public void setUserName(String userName) {
		this.userName = userName;
	}

	/**
	 * @return the username
	 */
	public String getUserName() {
		return userName;
	}
	
	public ILogListener getMessageWriter() {
		return messages;
	}
	
	@Transient
	public void setMessageWriter(ILogListener messages) {
		this.messages = messages;
	}


	public Map<String,ExecutionDetail> getDetails() {
		return details;
	}
	
	public void addSourcesToClean(List<ISource> sources) {
		sourcesToClean.addAll(sources);
	}
	
	protected List<ISource> getSourcesToClean() {
		return sourcesToClean;
	}
}
