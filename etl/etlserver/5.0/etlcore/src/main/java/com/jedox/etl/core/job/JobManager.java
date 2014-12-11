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
package com.jedox.etl.core.job;

import java.util.LinkedList;
import org.jdom.Element;

import com.jedox.etl.core.component.ComponentFactory;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;
import com.jedox.etl.core.component.RuntimeException;

/**
 * Manager Class for {@link IJob Jobs}
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class JobManager extends Manager {
	
//	private Hashtable<String, IJob> lookup = new Hashtable<String, IJob>();
	private LinkedList<IJob> jobs = new LinkedList<IJob>();
	
	/**
	 * Adds a job
	 */
	public IJob add(IComponent job) throws RuntimeException {
		if (job instanceof IComponent) {
			IJob old = (IJob) super.add(job);
			jobs.remove(old);
			jobs.add((IJob)job);
			return old;
		}
		throw new RuntimeException("Failed to add non Job Object.");
	}
	
	/**
	 * gets the job by name
	 */
	public IJob get(String name) {
		return (IJob) super.get(name);
	}
	
	/**
	 * gets all managed jobs 
	 */
	public IJob[] getAll() {
		return jobs.toArray(new IJob[jobs.size()]);
	}
	
	/**
	 * removes a job
	 */
	public IJob remove(String name) {
		IJob job = (IJob) super.remove(name);
		if (job != null) {
			jobs.remove(job);
		}
		return job;
	}
	
	/**
	 * Adds a Job by receiving its configuration as xml, delegating the creating process to {@link ComponentFactory#createJob(String, com.jedox.etl.core.component.ILocatable, com.jedox.etl.core.context.IContext, Element)}
	 */
	public IJob add(Element config) throws CreationException, RuntimeException {
		String type = config.getAttributeValue("type");
		IJob job = ComponentFactory.getInstance().createJob(type, this, getContext(), config);
		add(job);
		return job;
	}
	
	public String getName() {
		return ITypes.Jobs;
	}
	
	public void clear() {
		super.clear();
		jobs.clear();
	}
	
}
