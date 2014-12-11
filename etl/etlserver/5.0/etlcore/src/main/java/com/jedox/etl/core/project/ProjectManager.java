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
package com.jedox.etl.core.project;

import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;
import com.jedox.etl.core.component.RuntimeException;
/**
 * A Manager for the management of Projects
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
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
public class ProjectManager extends Manager {
	
	private ArrayList<IProject> orderedProjects = new ArrayList<IProject>();
	private static final Log log = LogFactory.getLog(ProjectManager.class);
	
	public ProjectManager() {
	}
	
	public IProject add(IComponent project) throws RuntimeException {
		if (project instanceof IProject) {
			IProject old = (IProject) super.add(project);
			orderedProjects.remove(old);
			orderedProjects.add((IProject)project);
			return old;
		}
		throw new RuntimeException("Failed to add component: not a project");
	}
	
	public IProject get(String name) {
		if (name != null)
			return (IProject) super.get(name);
		else {
			log.error("Failure: No Projectname is set! Please use <project name=\"someName\">");
		}
		return null;
	}

	public IProject remove(String name) {
		if (name != null) {
			IProject project = (IProject)super.remove(name);
			if (project != null) {
				orderedProjects.remove(project);
				return project;
			}
		}
		return null;
	}
	
	public IProject[] getAll() {
		return orderedProjects.toArray(new IProject[orderedProjects.size()]);
	}
	
	public IProject add(Element config) throws CreationException, RuntimeException {
		//IProject project = ProjectFactory.getInstance().newProject(this, config);
		String type = config.getAttributeValue("type","default");
		IProject project = ComponentFactory.getInstance().createProject(type, this, config);
		add(project);
		return project;
	}
	
	public void clear() {
		super.clear();
		orderedProjects.clear();
	}
	
	public String getName() {
		return ITypes.Projects;
	}
}
