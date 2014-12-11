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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.job;

import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.execution.ExecutionState;
import com.jedox.etl.core.job.IJob;

/**
 * 
 * @author Andreas Fr�hlich. Mail: andreas.froehlich@jedox.com
 *
 */
public abstract class Job extends Component implements IJob {
	
	private IContext context;
	private ExecutionState state;
	
	public void setState(ExecutionState state) {
		this.state = state;
	}
	
	public ExecutionState getState() {
		return state;
	}
	
	public boolean isExecutable() {
		return state.isExecutable();
	}
	
	public abstract void execute();
	
	public IContext getContext() {
		return context;
	}
		
	public void init() throws InitializationException {
		super.init();
		try {
			context = getConfigurator().getContext();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
