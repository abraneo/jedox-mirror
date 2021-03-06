package com.jedox.etl.components.scriptapi;


import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.ConnectionManager;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.context.ContextManager;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.scriptapi.Scanable;
import com.jedox.etl.core.scriptapi.ScriptAPI;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IDatabase;

public class OlapAPI extends ScriptAPI {

	public static enum Levels { ERROR, WARN, INFO, DEBUG, OFF };

	private Levels loglevel = Levels.ERROR;
	private ConnectionManager connections = new ConnectionManager();

	private IOLAPConnection getOlapConnection (String connection ) throws ConfigurationException{
		try {
			String locatorstr = getProjectName()+".connections."+connection;
			Locator locator = Locator.parse(locatorstr);
			IContext tempContext= ContextManager.getInstance().provide(getComponentContext());
			tempContext.addVariables(getApiProperties());
			return (IOLAPConnection) tempContext.getComponent(locator);
			
		} catch (Exception e) {
			throw new ConfigurationException(e.getMessage());
		}
	}
	
	public IConnection getConnection(String connection)throws ConfigurationException, RuntimeException{
		IOLAPConnection olapConn = null;
		try{
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);
			return (IConnection)olapConn.open();
		}
		catch(Exception e) {
			errorHandling("Could not get connection "+connection+": " + e.getMessage());
			return null;
		}
		finally{
		}
	}
	
	public IDatabase getDatabase(String connection) throws RuntimeException{
		IOLAPConnection olapConn = null;
		try{
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);//register the connection, if the method does not close the connection itself
			return olapConn.getDatabase(false,true);
		}
		catch(Exception e) {
			errorHandling("Could not get database from connection "+connection+": " + e.getMessage());
			return null;
		}
		finally{
			//this can not be done since the OLAP connection maybe reused (e.g. CubeLoad)
			//if(olapConn!=null) 
			//	olapConn.close();
		}
		
	}

	/** set the log level for exceptions in OLAP requests
	 * @param level Loglevel (ERROR, WARN, INFO, DEBUG, OFF)
	 */
	public void setLogLevel(String level) {
		try{
			loglevel = Levels.valueOf(level);
		}
		catch(Exception e) {
			log.error("Invalid Log Level "+level+". Allowed values are ERROR, WARN, INFO, DEBUG, OFF");
		}
	}

	/** gets the log level for exceptions in OLAP requests
	 * @return
	 */
	public Levels getLogLevel() {
		return loglevel;
	}

	/** raises log message with globally set LogLevel, if LogLevel is OFF raises a RuntimeException
	 * @param message text of log message
	 * @throws RuntimeException
	 */
	private void errorHandling (String message) throws RuntimeException {
		switch (loglevel) {
		case ERROR :
			log.error(message);	break;
		case WARN :
			log.warn(message);	break;
		case INFO :
			log.info(message);	break;
		case DEBUG :
			log.debug(message);	break;
		case OFF:
			throw new RuntimeException(message);
		}	
	}


	/** gets the cell value of a Palo cube
	 * @param connection name of the palo connection in the ETL project
	 * @param cube name of the cube
	 * @param coordinates list of elmenents for the cube dimensions
	 * @return cube cell value
	 * @throws RuntimeException
	 */
	public Object data(@Scanable(type=ITypes.Managers.connections) String connection, String cube, String[] coordinates) throws RuntimeException  {
		IOLAPConnection olapConn = null;
		try{
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);//register the connection, if the method does not close the connection itself
			IDatabase db = olapConn.getDatabase(false,true);
			ICube c = db.getCubeByName(cube);
			if (c==null) {
				throw new RuntimeException("Cube "+cube+" does not exist.");
			}				
			IDimension[] dims = c.getDimensions();
			//check for correct number of dimensions
			if (dims.length != coordinates.length) {
				throw new RuntimeException("Number of dimensions does not match");
			}	
			IElement[] elements = new IElement[coordinates.length];	
			for (int i=0; i<coordinates.length; i++) {			
				elements[i] = dims[i].getElementByName(coordinates[i],false);
				if(elements[i]==null){
					throw new RuntimeException("Element " + coordinates[i] + " does not exist in dimension " + dims[i].getName());
				}
			}			
			ICell cell = c.getCell(elements);
			return cell.getValue();

		}
		catch(Exception e) {
			errorHandling("Could not get cell data from cube "+cube+": " + e.getMessage());
			return null;
		}
		finally{
			//this can not be done since the OLAP connection maybe reused (e.g. CubeLoad)
			//if(olapConn!=null) 
			//	olapConn.close();
		}
	}

	/** Renames an element of a dimension
	 * @param connection
	 * @param dimension
	 * @param oldName
	 * @param newName
	 * @throws RuntimeException
	 */
	public void erename(@Scanable(type=ITypes.Managers.connections) String connection, String dimension, String oldName, String newName) throws RuntimeException  {
		erename(connection,dimension,new String[]{oldName},new String[]{newName});
	}
	
	/** Renames a list of elements of a dimension
	 * @param connection
	 * @param dimension
	 * @param oldNames
	 * @param newNames
	 * @throws RuntimeException
	 */
	public void erename(@Scanable(type=ITypes.Managers.connections) String connection, String dimension, String[] oldNames, String[] newNames) throws RuntimeException  {
		IOLAPConnection olapConn = null;
		try {
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);//register the connection, if the method does not close the connection itself
			IDatabase db = olapConn.getDatabase(false,true);
			IDimension dim = db.getDimensionByName(dimension);
			if (dim==null) {
				throw new RuntimeException("Dimension "+dimension+" does not exist.");
			}
			dim.setCacheTrustExpiry(3000);
			for (int i=0; i<oldNames.length; i++) {
				if (!oldNames[i].equals(newNames[i])) {
					IElement elem = dim.getElementByName(oldNames[i], false);
					if (elem==null)  
						throw new RuntimeException("Element "+oldNames[i]+" does not exist.");					
					elem.rename(newNames[i]);
				}	
			}	
			dim.resetCache();
		}	
		catch(Exception e) {
			errorHandling("Could not rename elements in dimension "+dimension+": " + e.getMessage());
		}				
	}
	
	/** change the types of a list of elements in a dimension
	 * @param connection
	 * @param dimension
	 * @param elementNames
	 * @param newType
	 * @throws RuntimeException
	 */

	/* method obsolete with 5.1 release
	public void eupdateElementsType(@Scanable(type=ITypes.Managers.connections) String connection, String dimension, String[] names, String type) throws RuntimeException  {
		IOLAPConnection olapConn = null;
		try {
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);//register the connection, if the method does not close the connection itself
			IDatabase db = olapConn.getDatabase(false,true);
			IDimension dim = db.getDimensionByName(dimension);
			if (dim==null) {
				throw new RuntimeException("Dimension "+dimension+" does not exist.");
			}
			dim.setCacheTrustExpiry(3000);
			IElement[] elements = new IElement[names.length];
			for (int i=0; i<names.length; i++) {
				IElement elem = dim.getElementByName(names[i], false);
				if (elem==null)  
					throw new RuntimeException("Element "+names[i]+" does not exist.");					
				elements[i] = elem;	
			}	
			dim.setCacheTrustExpiry(0);
			ElementType etype = null;
			if(type.equals("string")){
				etype = ElementType.ELEMENT_STRING;
			}else if(type.equals("numeric")){
				etype = ElementType.ELEMENT_NUMERIC;
			}else{
				throw new RuntimeException("Only values \"string\" and \"numeric\" are allowed to use as type.");
			}
			dim.updateElementsType(elements, etype);
		}	
		catch(Exception e) {
			errorHandling("Could not change the type of elements in dimension "+dimension+": " + e.getMessage());
		}finally{
			if(olapConn!=null) 
				olapConn.close();
		}	
	}
	*/
	
	/* method never released 
	 * public void emove(@Scanable(type=ITypes.Managers.connections) String connection, String dimension, String name, int position) throws RuntimeException  {
		IOLAPConnection olapConn = null;
		try {
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);//register the connection, if the method does not close the connection itself
			IDatabase db = olapConn.getDatabase(false,true);
			IDimension dim = db.getDimensionByName(dimension);
			if (dim==null) {
				throw new RuntimeException("Dimension "+dimension+" does not exist.");
			}
			IElement e = dim.getElementByName(name, false);
			if (e==null) {
				throw new RuntimeException("Element " + name + " does not exist.");
			}
			e.move(position);
		}	
		catch(Exception e) {
			errorHandling("Could not move element "+ name + " in dimension "+dimension+": " + e.getMessage());
		}		
	}*/
	
	/* method never released 
	/*public int eposition(@Scanable(type=ITypes.Managers.connections) String connection, String dimension, String name) throws RuntimeException  {
		IOLAPConnection olapConn = null;
		try {
			olapConn = getOlapConnection(connection);
			connections.add(olapConn);//register the connection, if the method does not close the connection itself
			IDatabase db = olapConn.getDatabase(false,true);
			IDimension dim = db.getDimensionByName(dimension);
			if (dim==null) {
				throw new RuntimeException("Dimension "+dimension+" does not exist.");
			}
			IElement e = dim.getElementByName(name, false);
			if (e==null) {
				throw new RuntimeException("Element " + name + " does not exist in  "+dimension+".");
			}
			return e.getPosition();
		}	
		catch(Exception e) {
			errorHandling("Could not move element "+ name + " in dimension "+dimension+": " + e.getMessage());
		}
		return 0;		
	}*/
	
   // Remove this method in forthcoming release as it may be confusing compared to method OLAP.getDatabase()	
	/** Saves the database and all of its cubes in an aggregated form to disk
	 * @param connection name of the palo connection in the ETL project
	 * @return true if the database save has been successful, otherwise false
	 * @throws RuntimeException
	 */
	public boolean saveDatabase(@Scanable(type=ITypes.Managers.connections) String connection) throws RuntimeException {
		log.warn("The method saveDatabase is deprecated. Use methods com.jedox.palojlib.interfaces.IDatabase.save() and com.jedox.palojlib.interfaces.ICube.save() instead.");
		IOLAPConnection olapConn = null;
		try{
			olapConn = getOlapConnection(connection);
			log.debug("Save Jedox database  "+ olapConn.getDatabase());
			//conn.save();
			IDatabase db = olapConn.getDatabase(false,true);
			db.save();
			for(ICube c : db.getCubes()){
				if(c.getType().equals(CubeType.CUBE_ATTRIBUTE) || c.getType().equals(CubeType.CUBE_NORMAL)){
					log.debug("Save cube "+c.getName());
					c.save();
				}
			}
			log.debug("Finished saving database");
		}
		catch(Exception e){
			errorHandling("Saving Jedox database in connection " + connection + " failed: " + e.getMessage());
			return false;
		}
		finally{
			if(olapConn!=null) 
				olapConn.close();
		}
		return true;
	}
	
	public boolean cubeConvert(@Scanable(type=ITypes.Managers.connections) String connection, String cube, int type) {
		if (type!=0 && type!=4) {
			log.error("Cube Type "+type+" not allowed for cube conversion.");
			return false;
		}

		IOLAPConnection olapConn = null;
		try{
			olapConn = getOlapConnection(connection);
			//Connection conn= (Connection) olapConn.open();
			IDatabase db = olapConn.getDatabase(false,true);
			ICube c = db.getCubeByName(cube);
			if(c== null){
				log.error("Cube " + cube +  " does not exist, and therefor can not be converted.");
				return false;
			}

			CubeType convType = (type==4) ? CubeType.CUBE_GPU : CubeType.CUBE_NORMAL;

			if (convType==c.getType()) {
				log.info("No cube conversion necessary.");
				return true;
			}	
			else {
				log.info("Cube conversion to type " + (type==4?"GPU":"Normal") + " started.");	
				c.convert(convType);
				log.info("Cube conversion to type " + (type==4?"GPU":"Normal") + " finished.");					
			}	

		}	
		catch(Exception e){
			log.error("Converting Jedox cube \"" + cube + "\" to type "+type+" failed: " + e.getMessage());
			return false;
		}
		finally{
			if(olapConn!=null) 
				olapConn.close();
		}
		return true;
	}
	
	public void close() {
		connections.disconnect();
	}

}
