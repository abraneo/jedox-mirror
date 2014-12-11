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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.service;

/**
 * @author khaddadin
 *
 */
public class GroupDescriptor {
	
	private String name;
	private String description;
	private String[] projects = new String[0];
	private long changedDate;
	private String changedUser;
	private boolean valid = true;
	private String errorMessage;
	
	/**
	 * @param name the name to set
	 */
	public void setName(String name) {
		this.name = name;
	}
	/**
	 * @return the name
	 */
	public String getName() {
		return name;
	}
	/**
	 * @param description the description to set
	 */
	public void setDescription(String description) {
		this.description = description;
	}
	/**
	 * @return the description
	 */
	public String getDescription() {
		return description;
	}
	/**
	 * @param projects the projects to set
	 */
	public void setProjects(String[] projects) {
		this.projects = projects;
	}
	/**
	 * @return the projects
	 */
	public String[] getProjects() {
		return projects;
	}
	/**
	 * @param valid the valid to set
	 */
	public void setValid(boolean valid) {
		this.valid = valid;
	}
	/**
	 * @return the valid
	 */
	public boolean getValid() {
		return valid;
	}
	/**
	 * @param errorMessage the errorMessage to set
	 */
	public void setErrorMessage(String errorMessage) {
		this.errorMessage = errorMessage;
	}
	/**
	 * @return the errorMessage
	 */
	public String getErrorMessage() {
		return errorMessage;
	}
	/**
	 * @param changedDate the changedDate to set
	 */
	public void setChangedDate(long changedDate) {
		this.changedDate = changedDate;
	}
	/**
	 * @return the changedDate
	 */
	public long getChangedDate() {
		return changedDate;
	}
	/**
	 * @param changedUser the changedUser to set
	 */
	public void setChangedUser(String changedUser) {
		this.changedUser = changedUser;
	}
	/**
	 * @return the changedUser
	 */
	public String getChangedUser() {
		return changedUser;
	}

}
