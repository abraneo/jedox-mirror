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

import java.util.HashMap;

import com.jedox.etl.core.component.EventSender;

/**
 * Definition class to provide constants and basic conversion functions on these constants for the {@link ExecutionState}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class ResultCodes extends EventSender{
	
	/**
	 * Defines the destination, where data produced by an {@link Execution} is written to. Data is produced by an Execution, if the {@link IExecutable} implements {@link com.jedox.etl.core.source.ISource}
	 * @author chris
	 *
	 */
	public enum DataTargets {
		/**
		 * writes data to the ExecutionStae via {@link ExecutionState#setData(String)}
		 */
		CSV_INLINE,
		/**
		 * writes data to the standard output stream, which is the console unless redirected elsewhere.
		 */
		CSV_STDOUT,
		/**
		 * writes data to an internal datastore, where it is kept persistent and can be further queried.
		 */
		CSV_PERSISTENCE,
		/**
		 * writes data (in XML) to the ExecutionStae via {@link ExecutionState#setData(String)}
		 */
		XML_INLINE,
		/**
		 * writes data (in XML) to the standard output stream, which is the console unless redirected elsewhere.
		 */
		XML_STDOUT
	}
	
	/**
	 * Defines the status codes, describing the status of an {@link ExecutionState}
	 * @author chris
	 *
	 */
	public enum Codes {
		/**
		 * {@link Execution} has encountered an unrecoverable error and thus was terminated premature for internal fault reasons. Some parts of it may not have been executed.
		 * Note: If failOnError (set in config.xml) is true (default), this is the status returned, when any error occurs within the execution.  
		 * <br>
		 * String code is: "Failed"
		 * <br>
		 * Numeric code is: 40
		 */
		statusFailed, 
		/**
		 * {@link Execution} finished without encountering any errors or warnings. Everything is perfect. This is how things should be!
		 * <br>
		 * String code is: "Completed successfully"
		 * <br>
		 * Numeric code is: 10
		 */
		statusOK, 
		/**
		 * {@link Execution} is finished and has encountered one or more errors causing parts of it to fail. Nevertheless the execution finished trying to execute all of its parts.
		 * Note: This status can only occur, if failOnError (set in config.xml) is false.  
		 * <br>
		 * String code is: "Completed with Errors"
		 * <br>
		 * Numeric code is: 30
		 */
		statusErrors, 
		/**
		 * {@link Execution} is finished and has encountered one or more warnings indicating that there are some problem within the data causing some data not to be processed successfully. No structural problems (errors) occurred.  
		 * <br>
		 * String code is: "Completed with Warnings"
		 * <br>
		 * Numeric code is: 20
		 */
		statusWarnings, 
		/**
		 * {@link Execution} is currently being executed and still running.
		 * <br>
		 * String code is: "Running"
		 * <br>
		 * Numeric code is: 5
		 */
		statusRunning, 
		/**
		 * {@link Execution} was terminated premature by user intervention. Some parts where executed, some parts where not. 
		 * <br>
		 * String code is: "Stopped"
		 * <br>
		 * Numeric code is: 50
		 */
		statusStopped, 
		/**
		 * {@link Execution} was tried to be terminated but it is still running.    
		 * <br>
		 * String code is: "Stopping"
		 * <br>
		 * Numeric code is: 49
		 */
		statusStopping, 
		/**
		 * {@link Execution} was interrupted by user intervention.    
		 * <br>
		 * String code is: "Interrupted"
		 * <br>
		 * Numeric code is: 55
		 */
		statusInterrupted, 
		/**
		 * {@link Execution} is present in the {@link IExecutor Executor} but not yet running.    
		 * <br>
		 * String code is: "Queued"
		 * <br>
		 * Numeric code is: 0
		 */
		statusQueued, 
		/**
		 * {@link Execution} was canceled by {@link IExecutor#stop(Long) user intervention} before it reached status running. No action was done by it. An aborted execution can NOT be reactivated and started again.      
		 * <br>
		 * String code is: "Aborted"
		 * <br>
		 * Numeric code is: 60
		 */
		statusAborted, 
		/**
		 * {@link Execution} is not valid and thus will not be executed. This indicates that the configuration of at least one of the involved components is faulty, so that this component cannot be created and initialized.
		 * <br>
		 * String code is: "Invalid"
		 * <br>
		 * Numeric code is: 70
		 */
		statusInvalid ,
		/**
		 * {@link Execution} was interrupted by user intervention.    
		 * <br>
		 * String code is: "Interrupting"
		 * <br>
		 * Numeric code is: 54
		 */
		statusInterrupting, 
	}
	
	private HashMap<Codes, String> codesNumeric = new HashMap<Codes,String>();
	private HashMap<Codes, String> codesString = new HashMap<Codes,String>();
	
	public ResultCodes () {
		codesString.put(Codes.statusErrors, "Completed with Errors");
		codesString.put(Codes.statusFailed, "Failed");
		codesString.put(Codes.statusOK, "Completed successfully");
		codesString.put(Codes.statusQueued, "Queued");
		codesString.put(Codes.statusRunning, "Running");
		codesString.put(Codes.statusStopped, "Stopped");
		codesString.put(Codes.statusStopping, "Stopping");
		codesString.put(Codes.statusWarnings, "Completed with Warnings");
		codesString.put(Codes.statusAborted, "Aborted");
		codesString.put(Codes.statusInvalid, "Invalid");
		codesString.put(Codes.statusInterrupted, "Interrupted");
		codesString.put(Codes.statusInterrupting, "Interrupting");
		codesNumeric.put(Codes.statusQueued, "0");
		codesNumeric.put(Codes.statusRunning, "5");
		codesNumeric.put(Codes.statusOK, "10");
		codesNumeric.put(Codes.statusWarnings, "20");
		codesNumeric.put(Codes.statusErrors, "30");
		codesNumeric.put(Codes.statusFailed, "40");
		codesNumeric.put(Codes.statusStopping, "49");
		codesNumeric.put(Codes.statusStopped, "50");
		codesNumeric.put(Codes.statusInterrupting, "54");
		codesNumeric.put(Codes.statusInterrupted, "55");
		codesNumeric.put(Codes.statusAborted, "60");
		codesNumeric.put(Codes.statusInvalid, "70");
		
	}
	
	/**
	 * gets the numeric representation of a status code
	 * @param code the status code
	 * @return the numeric representation
	 */
	public String getNumeric(Codes code) {
		return codesNumeric.get(code);
	}
	
	/**
	 * gets the string representation if a status code
	 * @param code the status code
	 * @return the string representation
	 */
	public String getString(Codes code) {
		return codesString.get(code);
	}	
	
	/**
	 * gets the status code from its numeric representation
	 * @param numericCode the numeric representation
	 * @return the status code
	 */
	public Codes getCode(String numericCode) {
		for (Codes c : codesNumeric.keySet()) {
			String s = getNumeric(c);
			if (s.equalsIgnoreCase(numericCode))
				return c;
		}
		return Codes.statusInvalid;
	}
}
