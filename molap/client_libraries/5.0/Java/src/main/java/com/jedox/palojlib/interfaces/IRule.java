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
package com.jedox.palojlib.interfaces;

/**
 * Interface that represents a rule in Palo cube
 * @author khaddadin
 *
 */
public interface IRule {


	/**
	 * get the palo server id for this rule, can not be null
	 * @return id
	 */
	public int getIdentifier();

	/**
	 * get the rule content or definition
	 * @return rule definition
	 */
	public String getDefinition();

	/**
	 * get the external id of the rule, can be null
	 * @return optional id
	 */
	public String getExternalIdentifier() ;

	/**
	 * get the timestamp of the rule, when it was created
	 * @return long value of the timestamp
	 */
	public long getTimestamp();

	/**
	 * optional comment
	 * @return comment of the rule
	 */
	public String getComment();

	/**
	 * get the boolean value whether a rule is active
	 * @return true if active, false otherwise
	 */
	public boolean isActive() ;

}
