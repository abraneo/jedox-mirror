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
 *   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.core.config;


import org.jdom.Element;

import com.jedox.etl.core.component.Locator;

public interface IConfigPersistence {

	
	/**
	 * loads the projects from the its persistence layer.
	 * @return the loaded repository document or a new repository document if none exists.
	 * @throws Exception 
	 */
	public abstract void load() throws Exception;

	/**
	 * saves/delete a certain project
	 * @param projectName
	 * @param project
	 * @throws Exception
	 */
	public abstract void save(Locator loc, Element project) throws Exception;
	
	/**
	 * needed for olap persistence, with file it is always false
	 * @return true only if olap persistence is used and the dimension etls does not exist
	 * @throws Exception
	 */
	
	public abstract void migrate (IConfigPersistence oldPersistence) throws Exception;	
	
	public boolean needMigration();


}
