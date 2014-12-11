/**
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
package com.jedox.etl.core.component;

/** 
*   Defines the available Types of {@link IComponent Components} and {@link IManager Managers}.
*   For each component type there exists an associated Interface.
*   For each manager type a component can define a manager, which manages all sub-components of this type.
*/
public interface ITypes {
	
	/**
	 * The available component types.
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum Components {
		connection, extract, transform, load, job, project
	}
	
	/**
	 * The available manager types
	 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
	 *
	 */
	public static enum Managers {
		projects, connections, extracts, alias_maps, transforms, loads, jobs, functions, variables , sources
	}
	
	
    //the following are available for backwards compatibility and will be probably removed in the future
	public static final String Projects = Managers.projects.toString();
	public static final String Connections = Managers.connections.toString();
	public static final String Extracts = Managers.extracts.toString();
	public static final String Transforms = Managers.transforms.toString();
	public static final String Loads = Managers.loads.toString();
	public static final String Jobs = Managers.jobs.toString();
	public static final String Functions = Managers.functions.toString();
	public static final String Contexts = Managers.variables.toString();
	public static final String Sources = Managers.sources.toString();
	public static final String Any = "any";
}
