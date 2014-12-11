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
package com.jedox.etl.components.config.function;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.function.FunctionConfigurator;
import com.jedox.etl.core.config.function.InputInfo;

public class JavaConfigurator extends FunctionConfigurator {
	
	private static Log log = LogFactory.getLog(JavaConfigurator.class);
	
	private InputInfo classInfo;
	private InputInfo methodInfo;
	private String methodName;
	private String className;
	private int varClassInputs = -1;
	private int varMethodInputs = -1;
	
	public void configure() throws ConfigurationException {
		super.configure();
		className = getParameter("class",null);
		methodName = getParameter("method",null);
		if ((className == null) || (methodName == null)) {
			log.error("'class' and 'method' have to be set as parameters");
		}
		classInfo = new InputInfo(getChildren(getXML(),"input"),"class");
		methodInfo = new InputInfo(getChildren(getXML(),"input"),"method");
	}
	
	private String[] concat(String[] a, String[] b) {
		String[] c = new String[a.length+b.length];
		for (int i=0; i<a.length; i++) {
			c[i] = a[i];
		}
		for (int i=0; i<b.length; i++) {
			c[i+a.length] = b[i];
		}
		return c;
	}
	
	public String[] getClassInputNames() throws ConfigurationException {
		return classInfo.getInputNames();
	}
	
	public String[] getClassInputValues() throws ConfigurationException {
		return classInfo.getInputValues();
	}
	
	public String[] getClassInputTypes() throws ConfigurationException {
		return classInfo.getInputTypes();
	}
	
	public String[] getMethodInputNames() throws ConfigurationException {
		return methodInfo.getInputNames();
	}
	
	public String[] getMethodInputValues() throws ConfigurationException {
		return methodInfo.getInputValues();
	}
	
	public String[] getMethodInputTypes() throws ConfigurationException {
		return methodInfo.getInputTypes();
	}
	
	private int getVariableInputCount(String[] values) {
		int count = 0;
		for (int i=0; i<values.length; i++) {
			if (values[i] == null) count++;
		}
		return count;
	}
	
	public int getVariableClassInputCount() throws ConfigurationException {
		if (varClassInputs < 0) varClassInputs = getVariableInputCount(getClassInputValues()); 
		return varClassInputs;
	}
	
	public int getVariableMethodInputCount() throws ConfigurationException {
		if (varMethodInputs < 0) varMethodInputs = getVariableInputCount(getMethodInputValues()); 
		return varMethodInputs;
	}
	
	public String getClassName() throws ConfigurationException {
		return className;
	}
	
	public String getMethodName() throws ConfigurationException {
		return methodName;
	}
	
	public String[] getInputNames() throws ConfigurationException {
		return concat(classInfo.getInputNames(), methodInfo.getInputNames());
	}
	
	public String[] getInputValues() throws ConfigurationException {
		return concat(classInfo.getInputValues(), methodInfo.getInputValues());
	}
	
	public String[] getInputTypes() throws ConfigurationException {
		return concat(classInfo.getInputTypes(), methodInfo.getInputTypes());
	}
}
