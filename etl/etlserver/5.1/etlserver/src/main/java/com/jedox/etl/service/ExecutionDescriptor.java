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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.service;


/**
 * SOAP Transport class for execution description.
 * @author chris
 *
 */
public class ExecutionDescriptor {
	
	private Long id;
	private int errors = 0;
	private int warnings = 0;
	private Long startDate;
	private Long stopDate;
	private String status;
	private String statusCode;
	private String project;
	private String type;
	private String name;
	private String executionType;
	private String userName;
	private boolean valid = true;
	private String errorMessage;
	private String result;
	private ComponentOutputDescriptor[] metadata; 
	
	public ExecutionDescriptor() {
	}
	

	/**
	 * determines, if the execution is valid. The execution is valid, if it could be successfully created and initialized by the Executor. The execution itself is valid, even runtime errors may occur. Runtime errors are described by {@link #getStatus()} and {@link #getErrorMessage()} 
	 * @return true if valid, false otherwise
	 */
	public boolean getValid() {
		return valid;
	}
	/**
	 * sets the validity of the execution
	 * @param valid
	 */
	public void setValid(boolean valid) {
		this.valid = valid;
	}
	/**
	 * get the error message, if any is set. if the execution is not valid than this message should describe the reason why. if the execution is valid but the status indicates an error, this message should describe the runtime error.  
	 * @return the error message, if any is set.
	 */
	public String getErrorMessage() {
		return errorMessage;
	}
	/**
	 * sets an error message for this execution. if the execution is not valid than this message should describe the reason why. if the execution is valid but the status indicates an error, this message should describe the runtime error.   
	 * @param errorMessage
	 */
	public void setErrorMessage(String errorMessage) {
		setValid(false);
		this.errorMessage = errorMessage;
	}
	/**
	 * gets the result data of the execution. The interpretation of this data is generic and depends on the concrete API-Method 
	 * @return the execution data
	 */
	public String getResult() {
		return result;
	}
	/**
	 * sets the result data of the execution
	 * @param result
	 */
	public void setResult(String result) {
		this.result = result;
	}
	
	/**
	 * gets the unique identifier for this execution.
	 * @return the id
	 */
	public Long getId() {
		return id;
	}
	/**
	 * sets the unique identifier. 
	 * @param id
	 */
	public void setId(Long id) {
		this.id = id;
	}
	/**
	 * gets the number of errors, which occurred in runtime. see also {@link #getStatus()}
	 * @return the number of errors.
	 */
	public int getErrors() {
		return errors;
	}
	/**
	 * sets the number of error occurred in runtime
	 * @param errors
	 */
	public void setErrors(int errors) {
		this.errors = errors;
	}
	/**
	 * gets the number of warnings, which occurred in runtime. see also {@link #getStatus()}
	 * @return the number of warnings
	 */
	public int getWarnings() {
		return warnings;
	}
	/**
	 * sets the number of warnings occurred in runtime
	 * @param warnings
	 */
	public void setWarnings(int warnings) {
		this.warnings = warnings;
	}
	/**
	 * gets the start date of the execution in milliseconds as used in a Java Date object. If the execution was not started yet, this returns 0
	 * @return the timestamp for the start 
	 */
	public Long getStartDate() {
		return startDate;
	}
	/**
	 * sets the start date of the execution in milliseconds as used in a Java Date object.
	 * @param startDate the timestamp for the stop
	 */
	public void setStartDate(Long startDate) {
		this.startDate = startDate;
	}
	/**
	 * gets the stop date of the execution in milliseconds as used in a Java Date object. If the execution is not finished yet, this return 0
	 * @return the stop date in time milliseconds
	 */
	public Long getStopDate() {
		return stopDate;
	}
	/**
	 * sets the stop date of the execution in milliseconds as used in a Java Date object.
	 * @param stopDate
	 */
	public void setStopDate(Long stopDate) {
		this.stopDate = stopDate;
	}
	/**
	 * gets the status of the execution. The following stati are supported: 
	 * <ul>
	 * 	<li>"Queued" (Code 0): the execution is pending to be executed but waits for the executor to do so.</li>
	 *  <li>"Running" (Code 5): the execution is currently executed.</li>
	 *  <li>"Completed successfully" (Code 10): the execution completed successfully without errors or warnings.</li>
	 *  <li>"Completed with Warnings" (Code 20): the execution completed without errors, but some warnings were raised.</li>
	 *  <li>"Completed with Errors" (Code 30): the execution completed with errors. Parts of the execution were probably aborted</li>
	 *  <li>"Failed" (Code 40): the execution failed in its preconditions, before it could be started.</li>
	 *  <li>"Stopped" (Code 50): the execution was started, but stopped by user intervention or server shutdown. Parts of the execution were probably aborted.</li>
	 *  <li>"Aborted" (Code 60): the execution was never started, but stopped by user intervention or server shutdown.</li>
	 * </ul>
	 * @return the status of the execution
	 */
	public String getStatus() {
		return status;
	}
	/**
	 * sets the status of the execution
	 * @param status
	 */
	public void setStatus(String status) {
		this.status = status;
	}
	/**
	 * gets the status code of the execution. The following codes are supported:
	 * <ul>
	 * 	<li>"Queued" (Code 0): the execution is pending to be executed but waits for the executor to do so.</li>
	 *  <li>"Running" (Code 5): the execution is currently executed.</li>
	 *  <li>"Completed successfully" (Code 10): the execution completed successfully without errors or warnings.</li>
	 *  <li>"Completed with Warnings" (Code 20): the execution completed without errors, but some warnings were raised.</li>
	 *  <li>"Completed with Errors" (Code 30): the execution completed with errors. Parts of the execution were probably aborted</li>
	 *  <li>"Failed" (Code 40): the execution failed in its preconditions, before it could be started.</li>
	 *  <li>"Stopped" (Code 50): the execution was started, but stopped by user intervention or server shutdown. Parts of the execution were probably aborted.</li>
	 *  <li>"Aborted" (Code 60): the execution was never started, but stopped by user intervention or server shutdown.</li>
	 * </ul>
	 * @return the status code of the execution
	 */
	public String getStatusCode() {
		return statusCode;
	}
	/**
	 * sets the status code of the execution
	 * @param statusCode
	 */
	public void setStatusCode(String statusCode) {
		this.statusCode = statusCode;
	}
	/**
	 * gets the project of this executable
	 * @return the project name
	 */
	public String getProject() {
		return project;
	}
	/**
	 * sets the project of the executable
	 * @param project
	 */
	public void setProject(String project) {
		this.project = project;
	}
	/**
	 * gets the component type of the executable. supported types are jobs, loads, extracts, transforms, connections
	 * @return the type of the executable in its plural form as denoted in a Locator.
	 */
	public String getType() {
		return type;
	}
	/**
	 * sets the type of the executable.
	 * @param type
	 */
	public void setType(String type) {
		this.type = type;
	}
	/**
	 * gets the execution type of the executable. supported types are standard, outputdescription, test, metadata, data 
	 * @return the type of the executable in its plural form as denoted in a Locator.
	 */
	public String getExecutionType() {
		return executionType;
	}
	/**
	 * sets the type of the executable.
	 * @param type
	 */
	public void setExecutionType(String executionType) {
		this.executionType = executionType;
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
	 * @param name
	 */
	public void setName(String name) {
		this.name = name;
	}
	/**
	 * gets the meta-data of the execution. only executables, which deliver data (extract, transform) support this. 
	 * @return an Array of {@link com.jedox.etl.core.component.ComponentOutputDescriptor ComponentOutputDescriptors}
	 */
	public ComponentOutputDescriptor[] getMetadata() {
		return metadata;
	}

	/**
	 * sets the meta-data of the executable 
	 * @param metadata
	 */
	public void setMetadata(ComponentOutputDescriptor[] metadata) {
		this.metadata = metadata;
	}


	/**
	 * @param user the user to set
	 */
	public void setUserName(String userName) {
// Temporarily commented out to make 4.0 ETL-clients work against 5.0 Server	
		this.userName = userName;
	}

	/**
	 * @return the user
	 */
	public String getUserName() {
		return userName;
	}
	
}
