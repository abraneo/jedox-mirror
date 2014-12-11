/**
*   @brief <Description of Class>
*  
*   @file
*  
*   Copyright (C) 2008-2009 Jedox AG
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
*   You may obtain a copy of the License at
*  
*   <a href="http://www.jedox.com/license_palo_bi_suite.txt">
*     http://www.jedox.com/license_palo_bi_suite.txt
*   </a>
*  
*   If you are developing and distributing open source applications under the
*   GPL License, then you are free to use Palo under the GPL License.  For OEMs, 
*   ISVs, and VARs who distribute Palo with their products, and do not license
*   and distribute their source code under the GPL, Jedox provides a flexible  
*   OEM Commercial License.
*  
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.service;

/**
 * @author khaddadin
 *
 */
public class ExecutionDetailDescriptor {
	
	private ExecutionDetailEntry[] executionDetailentries;
	private String svgGraph;
	/**
	 * @return the executionDetailentries
	 */
	public ExecutionDetailEntry[] getExecutionDetailentries() {
		return executionDetailentries;
	}
	/**
	 * @param executionDetailentries the executionDetailentries to set
	 */
	public void setExecutionDetailentries(ExecutionDetailEntry[] executionDetailentries) {
		this.executionDetailentries = executionDetailentries;
	}
	/**
	 * @return the svgGraph
	 */
	public String getSvgGraph() {
		return svgGraph;
	}
	/**
	 * @param svgGraph the svgGraph to set
	 */
	public void setSvgGraph(String svgGraph) {
		this.svgGraph = svgGraph;
	}

}
