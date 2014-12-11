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
package com.jedox.etl.core.function;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.jedox.etl.core.aliases.IAliasElement;
import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.function.FunctionConfigurator;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.util.TypeConversionUtil;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.ArrayList;

/**
 * Abstract base class for {@link IFunction Functions}. All concrete function implementations should inherit from this class.
 * This class fusions the functionality of the {@link com.jedox.etl.core.component.IComponent} Interface and the {@link IFunction} Interface, so that functions can act both as components and {@link IColumn Columns}. 
 * <br>
 * Methods intended to be overloaded by subclasses:
 * <ul>
 * 	<li>{@link #transform(Row)}: mandatory - does to i/o transformation</li>
 *  <li>{@link #validateInputs()}: optional - does input validation</li>
 *  <li>{@link #validateParameters()}: optional - does parameter validation</li>
 * </ul> 
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public abstract class Function extends Component implements IFunction {
	
	/**
	 * Enumeration of allowed input / output types mapping to java classes.
	 * @author chris
	 *
	 */
	protected static enum Types {
		tDouble, tInteger, tLong, tByte, tFloat, tShort, tString, tObject, tint, tdouble, tbyte, tfloat, tshort, tlong, tboolean, tBoolean
	}
	
	private static final Log log = LogFactory.getLog(Function.class);
	private Log aggLog = new MessageHandler(log);
	
	private Row inputs = new Row();
	private Row calculatedInputs;
	private HashMap<String,Object> lookup = new HashMap<String,Object>(100);
	private LinkedList<String> fifo = new LinkedList<String>();
	private int cacheSize = 100; 
	private Column output;
	private IColumn rowCountColumn;
	private int counter = 0;
	private boolean isForceEval = false;
	private List<IProcessor> processors = new ArrayList<IProcessor>();
	

	public Function() {
		setConfigurator(new FunctionConfigurator());
	}
	
	public FunctionConfigurator getConfigurator() {
		return (FunctionConfigurator)super.getConfigurator();
	}
	
	protected String toString(Object[] objects) {
		StringBuffer buffer = new StringBuffer();
		for (Object o: objects) {
			if (o != null)
				buffer.append(o.toString()+" ");
			else buffer.append("NULL ");
		}
		return buffer.toString();
	}
	
	/**
	 * gets the java classes for types defined by {@link Types}
	 * @param inputTypes the types as string
	 * @return the java classes corresponding to these types.
	 */
	protected Class<?>[] getClassesForTypes(String[] inputTypes) {
		Class<?>[] classes = new Class[inputTypes.length];
		for (int i=0; i<inputTypes.length; i++) {
			if (inputTypes[i]==null) {
				classes[i] = String.class;
			} else {
				Types t = Types.valueOf("t"+inputTypes[i]);
				switch (t) { //first test for primitives then for classes.
					case tint: classes[i] = Integer.TYPE; break;
					case tdouble: classes[i] = Double.TYPE; break;
					case tshort: classes[i] = Short.TYPE; break;
					case tlong: classes[i] = Long.TYPE; break;
					case tbyte: classes[i] = Byte.TYPE; break;
					case tboolean: classes[i] = Boolean.TYPE; break;
					case tString: classes[i] = String.class; break;
					case tDouble: classes[i] = Double.class; break;
					case tInteger: classes[i] = Integer.class; break;
					case tLong: classes[i] = Long.class; break;
					case tByte: classes[i] = Byte.class; break;
					case tFloat: classes[i] = Float.class; break;
					case tShort: classes[i] = Short.class; break;
					case tBoolean: classes[i] = Boolean.class; break;
					default: classes[i] = String.class; break;
				}
			}
		}
		return classes;
	}
	
	/**
	 * gets the canonical java class name for a type defined by {@link Types}  
	 * @param inputType the type as string
	 * @return the class name
	 * @throws ConfigurationException
	 */
	protected Class<?> getClassnameForType(String inputType) throws ConfigurationException {
		Types t = Types.valueOf("t"+inputType);
		switch (t) { //first test for primitives then for classes.
			case tint: return Integer.class; 
			case tInteger: return Integer.class;
			case tdouble: return Double.class;
			case tDouble: return Double.class;
			case tshort: return Short.class;
			case tShort: return Short.class;
			case tlong: return Long.class;
			case tboolean: return Boolean.class;
			case tBoolean: return Boolean.class;
			case tLong: return Long.class;
			case tbyte: return Byte.class;
			case tByte: return Byte.class;
			case tString: return String.class;	
			case tfloat: return Float.class;
			case tFloat: return Float.class;	
			case tObject: return Object.class;
			default: throw new ConfigurationException("Input type must be one of the following:"+toString(Types.values()));
		}
	}
	
	/**
	 * gets the java canonical java class names for some types defined by {@link Types} 
	 * @param inputTypes the types as String
	 * @return the classnames for these types.
	 * @throws ConfigurationException
	 */
	protected Class<?>[] getClassnamesForTypes(String[] inputTypes) throws ConfigurationException {
		Class<?>[] names = new Class<?>[inputTypes.length];
		for (int i=0; i<inputTypes.length; i++) {
			if (inputTypes[i]==null) {
				names[i] = null;
			} else {
				names[i] = getClassnameForType(inputTypes[i]);
			}
		}
		return names;
	}
	
	/**
	 * converts the value of the input column to an object of the type defined by {@link IColumn#getValueType()} 
	 * @param column the input column 
	 * @return an object of the type defined by {@link IColumn#getValueType()} 
	 * @throws FunctionException
	 */
	protected Object convert(IColumn column) throws FunctionException {
		try {
			return TypeConversionUtil.convert(column);
		}
		catch (Exception e) {
			throw new FunctionException(column.getName(),column.getValueAsString(),e.getMessage());
		}
	}
	
	/**
	 * Method for for input/output transformation to be implemented by subclasses
	 * @param values the array of values from the input nodes of this transformation
	 * @return the transformed output value
	 */
	protected Object transform(Row inputs) throws FunctionException {
		Object[] values = new Object[inputs.size()];
		for (int i=0; i<inputs.size(); i++) {
			IColumn c = inputs.getColumn(i);
			values[i] = c.getValue();
		}
		return transform(values);
	}
	
	/**
	 * Method for for input/output transformation for backwards compatibility. Use {@link #transform(Row)} instead.
	 * @param values the values of the inputs for this transformation
	 * @return the result of the transformer.
	 * @depreciated
	 */
	protected Object transform(Object[] values) {
		return null;
	}
	
	/**
	 * sets the size of the cache of input/output mappings this functions has. If a mapping is in cache {@link #transform(Row)} is NOT called but the mapped value is returned gaining performance especially for scripting functions
	 * @param cacheSize the number of input/output mappings to cache.
	 */
	protected void setCacheSize(int cacheSize) {
		this.cacheSize = cacheSize;
	}
	
	protected int getCacheSize() {
		return cacheSize;
	}
	
	protected boolean isCached() {
		return cacheSize > 0;
	}
	
	/**
	 * gets the hash key of the current input row values for cache lookup.
	 * @param row the input row
	 * @return the hash key
	 */
	protected String getKey(Row row) {
		StringBuffer buf = new StringBuffer();
		for (IColumn c : row.getColumns()) {
			buf.append("@");
			buf.append(c.getValueAsString());
		}
		return buf.toString();
	}
	

	public void setValue(Object value) {
		// do nothing value is calculated
	}
	
	public int getCalculationCount() {
		return counter;
	}
	
	protected Row getCalculatedInputs(Row inputs)
	{
		if(calculatedInputs == null || calculatedInputs.size() != inputs.size())
		{ // init calculated row
			calculatedInputs = new Row();
			for(IColumn c : inputs.getColumns())
			{
				Column cc = ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(), c);
				cc.mimic(c);
				calculatedInputs.addColumn(cc);
			}
		}
		for(int i = 0; i < inputs.size(); i++)
		{// fill calculated row
			calculatedInputs.getColumn(i).setValue(inputs.getColumn(i).getValue());
		}
		return calculatedInputs;
	}

	/**
	 * gets the output value of the function
	 */
	public final Object getValue()
	{
		try
		{
			isForceEval = true; //function is requested to be evaluated. we expect, that this will happen again force evaluation in the corresponding Transforminputprocessor to get correct time measurements.
			Row calculated = getCalculatedInputs(getInputs()); // precalculate inputs outside of own processing time
			Object value = null;
			if(isCached())
			{
				value = lookup.get(getKey(calculated));
			}
			if(value == null)
			{
				value = transform(calculated);
				counter++;
				// setValue(value);
				if(isCached())
				{
					if(fifo.size() >= getCacheSize())
					{
						String removeKey = fifo.removeLast();
						lookup.remove(removeKey);
					}
					String key = getKey(calculated);
					fifo.addFirst(key);
					lookup.put(key, value);
				}
			}
			return value;
		}
		catch(FunctionException e)
		{
			getLog().warn(e.getMessage(getName()));
		}
		/*
		 * catch (Exception e) { getLog().warn("Unexpected exception caught in Function "+getName()+": "+e.getClass().getCanonicalName()+" - "+e.getMessage()); }
		 */
		return null;
	}
	
	/**
	 * adds a column to the input
	 * @param input the input column
	 */
	protected void addInput(IColumn input) {
		if (input != null) {
			inputs.addColumn(input);
		}
	}
	
	public Row getInputs() {
		return inputs;
	}
	
	
	public void setRowCountColumn(IColumn rowCountColumn) {
		this.rowCountColumn = rowCountColumn;
	}
	
	public int getRowCount() {
		return (rowCountColumn == null) ? 0 : (Integer)rowCountColumn.getValue();
	}
	
	/**
	 * checks for a certain number of inputs. 
	 * @param number the number of inputs to have
	 * @param allowMore allow also more than specified by number
	 * @throws ConfigurationException
	 */
	protected void checkInputSize(int number, boolean allowMore) throws ConfigurationException {
		boolean result;
		if (allowMore)
			result = (getInputs().size() >= number);
		else
			result = (getInputs().size() == number);
		if (!result) {
			throw new ConfigurationException("Wrong number of inputs. Expected: "+number+" - Encountered: "+getInputs().size());
		}
	}
	
	//Override this to do ParameterValidation
	protected void validateParameters() throws ConfigurationException {
		//to be overridden by specific transformers
	}
	
	//Override this to do InputValidation
	protected void validateInputs() throws ConfigurationException {
		//to be overridden by specific transformers
	}
	
	protected Log getLog() {
		return aggLog;
	}
	
	public Class<?> getValueType() {
		return output.getValueType();
	}
	
	public void setValueType(Class<?> type) {
		output.setValueType(type);
	}
	
	@Override
	public void setAliasElement(IAliasElement aliasElement) {
		output.setAliasElement(aliasElement);
	}

	@Override
	public IAliasElement getAliasElement() {
		return output.getAliasElement();
	}

	public boolean isEmpty() {
		//own value is always null, since we do not allow constant transformers
		return true;
	}
	
	
	public String getValueAsString() {
		output.setValue(getValue());
		return output.getValueAsString();
	}
	
	public void mimic(IColumn source) {
		output.mimic(source);
	}
	
	protected IColumn getOutputColumn() {
		return output;
	}
	
	public void test() throws RuntimeException {
		//To be overloaded by subclasses!
	}
	
	public void close() {
		processors.clear();
		//To be overloaded by subclasses!
	}
	
	public List<IProcessor> getUsedProcessorList() {
		return processors;
	}
	
	public boolean isForceEval() {
		return isForceEval;
	}
		
	public void init() throws InitializationException {
	try {
			super.init();
			if (getParameter("buffered","true").equalsIgnoreCase("false")) {
				setCacheSize(0); //deactivate caching
			}
			isForceEval = getParameter("forceEval", "false").equalsIgnoreCase("true") || getParameter("#forceEval","false").equalsIgnoreCase("true");
			output = new Column(getName());
			aggLog = new MessageHandler(log);
			String[] inputNames = getConfigurator().getInputNames();
			String[] inputTypes = getConfigurator().getInputTypes();
			String[] inputValues = getConfigurator().getInputValues();
			Class<?>[] inputClassNames = getClassnamesForTypes(inputTypes);
			for (int j=0; j<inputNames.length; j++) {
				String inputName = inputNames[j];
				String inputValue = inputValues[j];
				Class<?> inputClassName = inputClassNames[j];
				CoordinateNode column = ColumnNodeFactory.getInstance().createCoordinateNode(inputName,null);
				column.setValueType(inputClassName);
				if (inputValue != null){ //a constant
					column.setName(inputValue + "ETL_CONST_FIELD"+ j); //otherwise, constant inputs will always have the same name
					column.setValue(inputValue);
				}
				addInput(column);
			}
			setValueType(getClassnameForType(getParameter("type","String")));
			validateParameters();
			validateInputs();
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}
	
}
