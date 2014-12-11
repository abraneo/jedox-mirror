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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.Row;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;

public class OlapData extends Function {

	private IOLAPConnection connection;
	private String cubeName;
	
	private IDimension[] dims;
	private ICube cube;
	private List<HashMap<String,IElement>> maps;

	
	private void initOlapData() throws RuntimeException {
		maps = new ArrayList<HashMap<String,IElement>>();
		IConnection conn = (IConnection) connection.open();
		
		cube = conn.getDatabaseByName(connection.getDatabase()).getCubeByName(cubeName);
		if (cube==null) {
			throw new RuntimeException("Cube "+cubeName+" does not exist in connection "+connection.getName());
		}
		dims = cube.getDimensions();

		for(int i=0;i<dims.length;i++){
			maps.add(new HashMap<String,IElement>());
		}
	}	
	
	@Override
	protected Object transform (Row inputs) throws FunctionException  {

		try{
			if(maps==null) {
				initOlapData();		
			} else { 				
				if(dims==null || cube==null)
					throw new RuntimeException("An error occured while trying to read the cube " + cubeName + " the first time, please check the previous warning for more information.");
			}

			if (dims.length != inputs.getColumns().size())
				throw new RuntimeException("Number of dimensions does not match with the number of inputs");
			
			IElement[] elements = new IElement[inputs.getColumns().size()];	
			for (int i=0; i<inputs.getColumns().size(); i++) {	
				String elementName = inputs.getColumn(i).getValueAsString();
				IElement element = maps.get(i).get(elementName);
				if(element == null){
					if(!maps.get(i).containsKey(elementName)){
						element = dims[i].getElementByName(elementName,false);
						maps.get(i).put(elementName, element);
						if(element==null){
							throw new RuntimeException("Element " + elementName + " does not exist in dimension " + dims[i].getName());
						}
					}else{
						throw new RuntimeException("Element " + elementName + " does not exist in dimension " + dims[i].getName());
					}
				}
				elements[i] = element;

			}			
			ICell cell = cube.getCell(elements);
			return cell.getValue();
		}
		catch (RuntimeException e) {
			throw new FunctionException(e.getMessage());
		}
	}

	public void close() {
		if(maps!=null){
			maps.clear();
		}
			maps=null;
			dims=null;
			cube=null;
	}

	protected void validateParameters() throws ConfigurationException {

		String connectionName = getParameter("connection","");
		if(connectionName.isEmpty()){
			throw new ConfigurationException("Connection parameter can not be empty.");
		}
		String locatorstr = this.getLocator().getRootName() + ".connections."+connectionName;
		Locator locator = Locator.parse(locatorstr);
		connection = (IOLAPConnection) ConfigManager.getInstance().getComponent(locator, getContextName());
		
		cubeName = getParameter("cube","");
		if (cubeName.isEmpty())
			throw new ConfigurationException("Cube parameter can not be empty.");
	}
	
	public void init() throws InitializationException {
		try {
			super.init();			
			// add to Connection Manager for dependency graph		
			ConnectionManager manager = new ConnectionManager();
			addManager(manager);
			manager.add(connection);
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}	
	
}
