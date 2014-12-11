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
 * SOAP Transport class for the server status.
 * @author chris
 *
 */
public class ServerStatus {
	
	private String version;
	private int runningExecutions;
	private int processorsAvailable;
	private long maxMemory;
	private long totalMemory;
	private long freeMemory;
	private boolean validating;
	private boolean autosaving;
	private String logLevel;
	private String[] datastores;
	
	/**
	 * sets the number of processors available
	 * @param processorsAvailable
	 */
	public void setVersion(String version) {
		this.version = version;
	}
	/**
	 * gets the version number of the server 
	 * @return the version number
	 */
	public String getVersion() {
		return version;
	}
	
	/**
	 * sets the number of processors available
	 * @param processorsAvailable
	 */
	public void setProcessorsAvailable(int processorsAvailable) {
		this.processorsAvailable = processorsAvailable;
	}

	/**
	 * sets the maximum memory available
	 * @param maxMemory
	 */
	public void setMaxMemory(long maxMemory) {
		this.maxMemory = maxMemory;
	}

	/**
	 * sets the total memory available
	 * @param totalMemory
	 */
	public void setTotalMemory(long totalMemory) {
		this.totalMemory = totalMemory;
	}

	/**
	 * sets the free memory available
	 * @param freeMemory
	 */
	public void setFreeMemory(long freeMemory) {
		this.freeMemory = freeMemory;
	}

	/**
	 * Defines whether validation is turned on
	 * @param validating
	 */
	public void setValidating(boolean validating) {
		this.validating = validating;
	}

	/**
	 * Defines whether autosaving is turned on
	 * @param autosaving
	 */
	public void setAutosaving(boolean autosaving) {
		this.autosaving = autosaving;
	}

	/**
	 * sets the log level.
	 * @param logLevel
	 */
	public void setLogLevel(String logLevel) {
		this.logLevel = logLevel;
	}

	/**
	 * gets the processors available on the server
	 * @return the number of processors available to the server.
	 */
	public int getProcessorsAvailable() {
		return processorsAvailable;
	}
	
	/**
	 * gets the maximum amount of memory available to the server.
	 * @return the maximum amount of memory in bytes.
	 */
	public long getMaxMemory() {
		return maxMemory;
	}
	
	/**
	 * gets the total amount of memory available to the server.
	 * @return the total amount of memory in Bytes.
	 */
	public long getTotalMemory() {
		return totalMemory;
	}
	
	/**
	 * gets the amount of free memory available to the server on the machine it is running on.
	 * @return the amount of free memory in Bytes.
	 */
	public long getFreeMemory() {
		return freeMemory;
	}
	
	/**
	 * Determines if the server is automatically validating the components configs by its schemas on instantiating components. 
	 * @return true, if validation is on.
	 */
	public boolean isValidating() {
		return validating;
	}
	
	/**
	 * Determines if the autosave property of this server is on. If true, all component config changes are persisted immediatly. If no, config changes have to be committed by the server API-call save().
	 * @return true, if autosave is on.
	 */
	public boolean isAutosaving() {
		return autosaving;
	}
	
	/**
	 * gets the log level set for this server
	 * @return the log level
	 */
	public String getLogLevel() {
		return logLevel;
	}

	/**
	 * gets the number of currently running executions on this server
	 * @return the number of running executions.
	 */
	public int getRunningExecutions() {
		return runningExecutions;
	}

	/**
	 * sets the number of currently running executions on this server
	 * @param runningExecutions
	 */
	public void setRunningExecutions(int runningExecutions) {
		this.runningExecutions = runningExecutions;
	}

	/**
	 * gets the available datastores on this server
	 * @return the available datastores
	 */
	public String[] getDatastores() {
		return datastores;
	}

	/**
	 * sets the available datastores on this server
	 * @param datastores
	 */
	public void setDatastores(String[] datastores) {
		this.datastores = datastores;
	}
	
	

}
