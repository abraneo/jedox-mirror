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
package com.jedox.etl.core.execution;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

import java.util.Properties;
import java.util.Iterator;
import java.util.Collections;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Level;
import org.apache.log4j.helpers.ISO8601DateFormat;

import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.ExecutionLog;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.hibernate.IPersistable;
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

	@Id @GeneratedValue(strategy=GenerationType.TABLE)
	@Column(name = "RESULT_ID")
	private Long id;
	private int errors = 0;
	private int warnings = 0;
	private Date startDate;
	private Date stopDate;
	private Codes status = Codes.statusQueued;
	private String project;
	private String type;
	private String name;
	@Column(length = 1000)
	private String firstErrorMessage;
	@Transient
	private int infos = 0;
	@Transient
	private List<Message> messages = Collections.synchronizedList(new LinkedList<Message>());
	@Transient
	private Row metadata;
	@Transient
	private DataTargets dataTarget = DataTargets.INLINE;
	@Transient
	private String data;
	@Transient
	private boolean archive = true;
	@Transient
	private ExecutionLog elog;


	public ExecutionState() {
	}

	public ExecutionState(Locator locator) {
		setProject(locator.getRootName());
		setType(locator.getManager());
		setName(locator.getName());
		setStartDate(new Date());
		elog = new ExecutionLog(this);
	}

	/**
	 * opens the ExecutionState and sets its state to "running".
	 * @param context the context the execution is running in.
	 * @param threadName the name of the execution thread, which may access this state.
	 */
	public void open(IContext context, boolean dependent) {
		logSettings("Variables",context.getVariables(),false);
		if (!dependent) {
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
			if (!showExecutionParams && key.startsWith(NamingUtil.internalPrefix())) {
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
	 * closes the ExecutionState and sets its state accordingly to what has happened during the execution, which is all reflected in this ExecutionState objects properties.
	 */
	public Codes close(boolean dependent) {
		Codes code = Codes.statusStopped;
		if (!getStatus().equals(Codes.statusStopped)) { //execution is not already stopped.
			code = Codes.statusOK;
			if (getWarnings() > 0) code = Codes.statusWarnings;
			if (getErrors() > 0) code = Codes.statusErrors;
			//if aborted on errors
			if (!isExecutable()) code = Codes.statusFailed;
			if (getStatus().equals(Codes.statusStopping)) code = Codes.statusStopped;
		}
		if (!dependent) {
			setStopDate(new Date());
			setExecutable(false);
			setStatus(code);
		}
		return code;
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
		}
		setStatus(Codes.statusAborted);
	}

	/**
	 * sets the {@link #isExecutable()} flag, indication whether components sharing this state should cleanly terminate executing.
	 * @param isExecutable false if execution is to be stopped.
	 */
	public void setExecutable(boolean isExecutable) {
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

	/**
	 * gets all Messages, each wrapping a log entry
	 * @return the list of messages
	 */
	public List<Message> getMessages() {
		return messages;
	}

	/**
	 * sets the messages, each wrapping a log entry
	 * @param messages the messages
	 */
	public void setMessages(List<Message> messages) {
		this.messages = messages;
	}
	
	public Message getFirstMessageOfType(String type) {
		for (Message m : messages) {
			if (m.getType().equals(type)) return m;
		}
		return null;
	}
	
	public void removeMessage(Message m) {
		if (m != null) messages.remove(m);
	}
	
	public void addMessage(boolean insert, String type, Long timestamp, String message) {
		//CSCHW: removed next line check to allow info messages to be written after an error
		//if (isExecutable()) {
			synchronized(messages) {
				if(message.length()>1000){
					//log.info("shrinking the message length to 900");
					message = message.substring(0, 900);
				}
				if (insert)
					messages.add(0,new Message(getId(),type,timestamp, message));
				else 
					messages.add(new Message(getId(),type,timestamp, message));
			}
			// add the new error or warning message
			// store only if there was no previous errors stored
			if(Level.toLevel(type).isGreaterOrEqual(Level.WARN) && getErrors() == 0){// no error message is stored but may be warning
				if(getWarnings() == 0 && type.equals("WARN")){// no warning message too
						setFirstErrorMessage(message.substring(message.indexOf("WARN :") + 6)); // just to chop the the date and the code line info and give the only the message
				}else if(Level.toLevel(type).isGreaterOrEqual(Level.ERROR)){
						setFirstErrorMessage(message.substring(message.indexOf("ERROR :") + 7)); // just to chop the the date and the code line info and give the only the message
				}else{}
			}

			//if (Level.toLevel(type).isGreaterOrEqual(Level.ERROR) && getFirstErrorMessage() == null)
			//	setFirstErrorMessage(message.substring(message.indexOf(") - ") + 4)); // just to chop the the date and the code line info and give the only the error message
				//setFirstErrorMessage(message);
		//}
	}
	
	public String getMessagesText(String type, Long timestamp) {
		return getMessagesText(type,timestamp,0,0);
	}

	/**
	 * adds a message wrapping a log entry
	 * @param type the type of the message (e.g. error, warning, ...)
	 * @param timestamp the timestamp of the message
	 * @param message the message text
	 */
	public void addMessage(String type, Long timestamp, String message) {
		addMessage(false,type,timestamp,message);
	}

	/**
	 * Adds a message to the log of the execution without using the Logger
	 * @param type the type of the message (e.g. error, warning, ...)
	 * @param message  the message text
	 */
	public void addMessageExternal(String type, String message) {
		long currentTime = System.currentTimeMillis();
		// Apply same format to message text as defined in PatternLayout of ExecutionLog
		// Format: "%d %5p : %m%n"
		ISO8601DateFormat format = new ISO8601DateFormat();
		StringBuffer sbuf = new StringBuffer();
		format.format(new Date(currentTime),sbuf,null);
		sbuf.append("  ").append(type).append(" : ").append(message).append("\r");
		// Add the message for the Execution
		addMessage(type,new Timestamp(currentTime).getTime(),sbuf.toString());
		// Put the message also to Log
		log.info("Message added: "+message);
	}	
	
	/**
	 * gets all log entries of a given type starting at a given time
	 * @param type the type of the log messages
	 * @param timestamp the time to start to return the messages. All messages bevore that time are ignored.
	 * @return a String representation of the filtered log.
	 */
	public String getMessagesText(String type, Long timestamp, int start, int pagesize) {
		//incase it is persisted and not in memory
		if(getMessages().size() == 0){
			try {
				setMessages(StateManager.getInstance().getMessages(getId()));
			} catch (RuntimeException e) {	}
		}
		StringBuffer buffer = new StringBuffer();
		List<String> validMessages = new ArrayList<String>();
		for (Message m : getMessages()) {
			if (((timestamp == null) || (timestamp == 0) || (timestamp > m.getTimestamp())) && ((type == null) || (Level.toLevel(m.getType()).isGreaterOrEqual(Level.toLevel(type))))) {
				validMessages.add(m.getMessage());
			}
		}
		if (pagesize == 0) pagesize = validMessages.size();
		for (int i=start; (i < start + pagesize) && (i < validMessages.size()); i++) {
			buffer.append(validMessages.get(i));
		}
		// If no log messages available check for first error message from configuration phase
		if (buffer.length() == 0 && getFirstErrorMessage() != null)
			buffer.append("Configuration Error : "+getFirstErrorMessage());
		return buffer.toString();
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
}
