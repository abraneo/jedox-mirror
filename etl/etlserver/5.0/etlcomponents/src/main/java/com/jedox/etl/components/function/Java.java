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
package com.jedox.etl.components.function;
import java.lang.reflect.Method;


import com.jedox.etl.components.config.function.JavaConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

/**
 * 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class Java extends Function {
	
	private String className;
	private String methodName;
	private Method method = null;
	private Object object = null;
	private Class<?>[] constructorParam = null;
	private Class<?> transformClass = null;
	private String[] classInputNames = null;
	private String[] classInputTypes = null;
	private Object[] classInputValues = null;
	private String[] methodInputNames = null;
	private String[] methodInputTypes = null;
	private Object[] methodInputValues = null;
	
	public Java() {
		setConfigurator(new JavaConfigurator());
	}

	public JavaConfigurator getConfigurator() {
		return (JavaConfigurator)super.getConfigurator();
	}

	protected Object transform(Row values) throws FunctionException {
		return getResult(values);
	}
	
	private Object[] assignValues(Row row, String[] inputNames, Object[] inputValues) throws FunctionException {
		try { 
			Object[] result = new Object[inputValues.length];
			for (int i=0; i<inputValues.length; i++) {
				if (inputValues[i] == null) {//variable input
					IColumn column = row.getColumn(inputNames[i]);
					if (column != null) {
						result[i] = convert(column);
					}
				} else {
					Column c = new Column(inputNames[i]);
					c.setValue(inputValues[i]);
					result[i] = convert(c);
				}
			}
			return result;
		}
		catch (Exception e) {
			throw new FunctionException(e);
		}
	}
	
	private Method getMethod() throws FunctionException {
		try {
			if (method == null)  {
				Class<?>[] methodParamTypes = getClassesForTypes(methodInputTypes);
				method = transformClass.getMethod(methodName, methodParamTypes);
			}
		}
		catch (NoSuchMethodException e) {
			throw new FunctionException("No method "+methodName+" found in class "+className);
		}
		return method;
	}
	
	private Object getResult(Row row) throws FunctionException {
//		try to instantiate with a constructor having the input as parameter. This is rather costly in performance!
		try {
			if (object == null) {
				constructorParam = getClassesForTypes(classInputTypes);
				try {
					object = transformClass.getConstructor(constructorParam).newInstance(assignValues(row,classInputNames,classInputValues));
				} catch (Exception e) {
					//fail silent and try static mode. set object to some non null value.
					object = "static";
				}
			} else if (getConfigurator().getVariableClassInputCount() > 0) {//if there are variables, construct a new object on them
				object = transformClass.getConstructor(constructorParam).newInstance(assignValues(row,classInputNames,classInputValues));
			}
			return getMethod().invoke(object, assignValues(row,methodInputNames,methodInputValues));
			
		} catch (Exception e) {
			throw new FunctionException(e);
		}
	}
	
	public String getValueType() {
		String result = null;
		try {
			result = getMethod().getReturnType().getCanonicalName();
		}
		catch (Exception e) {}
		return result;
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,true);
	}
	
	protected void validateParameters() throws ConfigurationException {
		try {
			className = getConfigurator().getClassName();
			methodName = getConfigurator().getMethodName();
			classInputNames = getConfigurator().getClassInputNames();
			classInputTypes = getConfigurator().getClassInputTypes();
			classInputValues = getConfigurator().getClassInputValues();
			methodInputNames = getConfigurator().getMethodInputNames();
			methodInputTypes = getConfigurator().getMethodInputTypes();
			methodInputValues = getConfigurator().getMethodInputValues();
			transformClass = Class.forName(className);
		}
		catch (ClassNotFoundException e) {
			throw new ConfigurationException("Class not found: "+e.getMessage()); 
		}
		catch (Exception e) {
			throw new ConfigurationException("Unexpected Exception: "+e.getMessage());
		}
	}
}
