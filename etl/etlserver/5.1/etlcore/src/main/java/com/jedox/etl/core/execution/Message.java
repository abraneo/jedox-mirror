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

import javax.persistence.*;

import org.apache.log4j.Level;

import com.jedox.etl.core.persistence.hibernate.IPersistable;

/**
 * Persistent class for wrapping single log messages of {@link Execution Executions}.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

@Entity
public class Message implements IPersistable {
	@Id @GeneratedValue(strategy=GenerationType.TABLE)
	@Column(name = "MESSAGE_ID") 
	private Long id;
	private Long timestamp;
	private String type;
	private Integer intType;
	@Column(length = 1000) 
	private String message;
	private Long resultId;
	
	public Message() {
	}
	
	public Message(Long resultId, String type, Long timestamp, String message) {
		setResultId(resultId);
		setMessage(message);
		setType(type);
		setIntType(Level.toLevel(type).toInt());
		setTimestamp(timestamp);
	}

	/**
	 * gets the id of this message
	 * @return the unique message id
	 */
	public Long getId() {
		return id;
	}

	/**
	 * sets the id of this message
	 * @param id the unique message id
	 */
	public void setId(Long id) {
		this.id = id;
	}
	
	/**
	 * gets the timestamp of this message (indication the exact time the underlying log message occurred).
	 * @return the timestamp of this messsage
	 */
	public Long getTimestamp() {
		return timestamp;
	}

	/**
	 * sets the timestamp of this message
	 * @param timestamp the timestamp
	 */
	public void setTimestamp(Long timestamp) {
		this.timestamp = timestamp;
	}
	
	/**
	 * gets the type (log level) of this message ("error", "info", etc)
	 * @return the message type
	 */
	public String getType() {
		return type;
	}

	/**
	 * sets the type of this message
	 * @param type the message type
	 */
	public void setType(String type) {
		this.type = type;
	}

	/**
	 * gets the message text 
	 * @return the message text
	 */
	public String getMessage() {
		return message;
	}

	/**
	 * sets the message text
	 * @param message
	 */
	public void setMessage(String message) {
		this.message = message;
	}
	
	/**
	 * gets the id of the execution this message belongs to.
	 * @return the execution id
	 */
	public Long getResultId() {
		return resultId;
	}

	/**
	 * sets the id of the execution this message belongs to.
	 * @param resultId
	 */
	public void setResultId(Long resultId) {
		this.resultId = resultId;
	}

	public void setIntType(Integer intType) {
		this.intType = intType;
	}

	public Integer getIntType() {
		return intType;
	}

}
