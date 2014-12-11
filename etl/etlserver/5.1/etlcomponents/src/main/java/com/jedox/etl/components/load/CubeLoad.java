/**
//*   @brief <Description of Class>
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
package com.jedox.etl.components.load;

import java.math.BigInteger;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IRule;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.ICube.SplashMode;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.etl.components.config.load.CubeConfigurator;
import com.jedox.etl.components.config.load.CubeConfigurator.DefaultType;
import com.jedox.etl.core.aliases.AliasMap;
import com.jedox.etl.core.aliases.AliasMapElement;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.AliasProcessor;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;

/**
 * Exporter Class for Generating a Palo Cube.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CubeLoad extends AbstractOlapLoad implements ILoad {
	
	protected class BulkWriter extends Thread {
		
		public BulkWriter(ICube cube, ArrayList<IElement[]> elements, ArrayList<Object> values, int exported) {
			super();
			this.setName(Thread.currentThread().getName());
			this.elements = elements;
			this.values = values;
			this.cube = cube;
			this.exported = exported;
		}
		
		private ArrayList<IElement[]> elements;
		private ArrayList<Object> values;
		private ICube cube;
		private int exported;
	
		private void execute() {
			if (values.size() > 0) {
				IElement[][] e = elements.toArray(new IElement[elements.size()][]);
				Object[] v = values.toArray(new Object[values.size()]);
				try {
					cube.loadCells(e, v,cube.getCellLoadContext(splashMode, bulkSize, aggregate, useEventProcessor),null);
				}
				catch (Exception ex) {
					aggLog.error("Failed to load Cube " + getCubeName() + ": " + ex.getMessage());
				} catch(OutOfMemoryError e1){
					System.err.println("Failed to load Cube " + getCubeName() + ": " + e1.getMessage());
					log.fatal("Failed to load Cube " + getCubeName() + ": " + e1.getMessage());
					throw e1;
				}
				if (bulkSize > 1)
					log.info("Records loaded to Cube: "+exported);
			}
		}
		
		public synchronized void run() {
			log.debug("Started bulk load thread.");
			execute();
			elements.clear();
			values.clear();			
			log.debug("Finished bulk load thread.");
		}	
		
	}
	
	protected class BulkAggregator {
		private int count = 0;
		private int exported = 0;
		
		private ICube cube;
		private ArrayList<IElement[]> elements = new ArrayList<IElement[]>();
		private ArrayList<Object> values = new ArrayList<Object>();
		private BulkWriter bulkWriter;

		public BulkAggregator(ICube cube) {
			this.cube = cube;
		}
		
		public void waitForCompletion() {
			if (bulkWriter != null) {
				log.debug("Waiting for bulkwriter.");
				synchronized(bulkWriter) {
					while (bulkWriter.isAlive()) {
						try {
							bulkWriter.wait();
						} catch (InterruptedException ie) {}
					}
				}
				log.debug("Waiting finished.");
			}
		}
		
		// Sequential execution
		private void executeSequential() {
			if (values.size() > 0) {
				IElement[][] e = elements.toArray(new IElement[elements.size()][]);
				Object[] v = values.toArray(new Object[values.size()]);
				try {
					cube.loadCells(e, v,cube.getCellLoadContext(splashMode, bulkSize, aggregate, useEventProcessor),null);
				}
				catch (Exception ex) {
					aggLog.error("Failed to load Cube " + getCubeName() + ": " + ex.getMessage());
				}
				if (bulkSize > 1)
					log.info("Records loaded to Cube: "+exported);
			}
		}
		
		public void flush() {
			waitForCompletion();
			log.info("Sending load request to Olap Server");
			exported += values.size();			
			if (multiThreaded) {
				bulkWriter = new BulkWriter(cube,elements,values,exported);
				bulkWriter.start();				
			} else {
				executeSequential();
			}
			elements = new ArrayList<IElement[]>();
			values = new ArrayList<Object>();
			count = 0;
		}

		private void addInternal(IElement[] coordinates, Object value) {
			elements.add(coordinates);
			values.add(value);
			count ++;
			if (count >= bulkSize) {
				flush();
			}
		}

		private String printCoordinates(IElement[] coordinates) {
			StringBuffer result = new StringBuffer("(");
			for (IElement e : coordinates)
				result.append(e.getName()+",");
			result.deleteCharAt(result.length()-1);
			result.append(")");
			return result.toString();
		}

		public void add(Row dimensionRow,IElement[] coordinates, Object value, boolean numeric, boolean consolidated) {
			if ((coordinates != null) && (value != null)) {
				//test if value must be numeric
				if (numeric && !consolidated) {
					if (!((aggregate) && (value.toString().equals("0")))) { //only write a vector when not aggregating zeros.
						try {
							Double.valueOf(value.toString());
							addInternal(coordinates,value);
						}
						catch (NumberFormatException e) {
							aggLog.warn("Could not insert value "+value.toString()+" at path "+printCoordinates(coordinates)+" in cube " + getCubeName() + ": Value must be numeric.");
						}
					}
				}
				else
					addInternal(coordinates,value);
			}
		}
	}

	protected class ConsoldiateDefaultHandler{

		//Hashmaps needed to load default consolidate 
		private HashMap<String,ArrayList<IElement>> elementsAdded = new HashMap<String,ArrayList<IElement>>();
		private HashMap<String,IElement> dimensionsDefaults = new HashMap<String,IElement>();
		private HashMap<String,IElement[]> dimensionsDefaultChildren = new HashMap<String,IElement[]>();
		private HashMap<String,Double[]> dimensionsDefaultWeights = new HashMap<String,Double[]>();

		public void addElement(IDimension dim,IElement defaultElement, IElement newlyBuiltElement){

			ArrayList<IElement> elementsToConsolidate = handler.elementsAdded.get(dim.getName());
			if(elementsToConsolidate==null){
				elementsToConsolidate = new ArrayList<IElement>();
			}
			elementsToConsolidate.add(newlyBuiltElement);
			elementsAdded.put(dim.getName(), elementsToConsolidate);

			IElement dimensionDefault = dimensionsDefaults.get(dim.getName());
			if(dimensionDefault==null){
				dimensionsDefaults.put(dim.getName(), defaultElement);
				IElement[] children = defaultElement.getChildren();
				dimensionsDefaultChildren.put(dim.getName(), children);
				ArrayList<Double> weights = new ArrayList<Double>();
				for(IElement oldChild:children){
					weights.add(oldChild.getWeight(defaultElement));
				}
				dimensionsDefaultWeights.put(dim.getName(),weights.toArray(new Double[weights.size()]));
			}
		}

		public void loadConsolidations() throws RuntimeException {
			int count=0;
			for(String dimensionName:elementsAdded.keySet()){
				IDimension dim = getConnection().getDatabase(false,true).getDimensionByName(dimensionName);
				ArrayList<IConsolidation> updatedConsolidations = new ArrayList<IConsolidation>();
				IElement defaultElement = dimensionsDefaults.get(dimensionName);

				for(IElement newChild:elementsAdded.get(dimensionName)){
					updatedConsolidations.add(dim.newConsolidation(defaultElement , newChild,1.0));
					count=count+1;
				}

				//old Consolidation for the default element should not be lost
				IElement [] children = dimensionsDefaultChildren.get(dimensionName);
				Double [] weights = dimensionsDefaultWeights.get(dimensionName);
				for(int i=0;i<children.length;i++) {
					updatedConsolidations.add(dim.newConsolidation(defaultElement , children[i] ,weights[i]));
				}		
				dim.updateConsolidations(updatedConsolidations.toArray(new IConsolidation[updatedConsolidations.size()]));
			}
			if (count!=0) {
				log.info("New elements consolidated under default element "+defaultValue+": "+count);
			}	
		}
	}
	private static final Log log = LogFactory.getLog(CubeLoad.class);
	private MessageHandler aggLog = new MessageHandler(log);
	private SplashMode splashMode = SplashMode.SPLASH_NOSPLASHING;
	private LinkedHashMap<String,Integer> coordinateMap;
	private LinkedHashMap<String,Integer> coordinateToRowMap;
	private int bulkSize;
	private String cubeName;
	private boolean aggregate = false;
	private BigInteger cellsFilled;
	private int countCleared = 0;
	private DrillthroughLoader drillthroughLoader = new DrillthroughLoader();
	private DefaultType defaultType;
	private String defaultValue;
	private boolean doDeactivateRules;
	private boolean useEventProcessor;
	private boolean useCompleteLocking;
	private boolean convertedFromGPU = false;
	private boolean multiThreaded = true;
	private HashMap<String, IDimension> dimLookupMap = new HashMap<String, IDimension>();
	private List<IDimension> dimensionList = new ArrayList<IDimension>();
	private ConsoldiateDefaultHandler handler = new ConsoldiateDefaultHandler(); 

	public CubeLoad() {
		setConfigurator(new CubeConfigurator());
	}

	public CubeConfigurator getConfigurator() {
		return (CubeConfigurator)super.getConfigurator();
	}

	protected String getCubeName() {
		return cubeName;
	}

	/**
	 * Checks if a column is considered as a cube dimension. Skips drilltrhough columns and columns with no name in (optional) dimension mapping
	 * @param name
	 * @return
	 */
	private boolean hasDimension(String name) {
		if (drillthroughLoader.containAnnex(name.toUpperCase()))
			return false;
		else
			return (getConfigurator().getDimensions().isEmpty() || getConfigurator().getDimensions().containsValue(name));
	}
	
	/**
	 * maps the Names of the columns of the result set to dimensions
	 * @param database the database, where the dimensions are stored
	 * @param meta the MetaData describing the columns
	 * @return the Array of Dimensions
	 */
	private IDimension[] getDimensions(IDatabase database, Row meta) throws InitializationException {
		LinkedList<IDimension> dimensions = new LinkedList<IDimension>();
		coordinateToRowMap = new LinkedHashMap<String,Integer>();
		for (int i=0; i< meta.size()-1; i++) {
			//do not resolve info columns
			if (hasDimension(meta.getColumn(i).getName())) {
				//don't call getDimensionByName because of case problem in metadata (case-insensitive)
				//Dimension dim = database.getDimensionByName(meta.getColumnName(i));
				IDimension dim = null;
				IDimension[] dims = database.getDimensions();
				for (IDimension adim: dims) {
					if (adim.getName().equalsIgnoreCase(meta.getColumn(i).getName())) {
						dim = adim;
						break;
					}
				}
				if (dim == null) {
					throw new InitializationException("Dimension "+meta.getColumn(i).getName()+" does not exist.");
				}
				if (!dimensions.contains(dim)) dimensions.add(dim);
				coordinateToRowMap.put(dim.getName().toLowerCase(), i);
			}
		}
		return dimensions.toArray(new IDimension[dimensions.size()]);
	}

	private ICube addCube(IDatabase d, IDimension[] dims) throws InitializationException {
		ICube c = null;
		//add new cube
		if (dims == null)
			throw new InitializationException("Dimensions needed for load not found in database.");
		else {
			log.info("Creating cube "+getCubeName());
			c = d.addCube(getCubeName(), dims);
		}
		return c;
	}

	protected void buildCoordinateMap(ICube c, IDimension[] etldims) throws InitializationException {
		coordinateMap = new LinkedHashMap<String,Integer>();
		IDimension[] cubedims = c.getDimensions();
		//check for correct number of dimensions
		if (getMode().equals(Modes.DELETE)) {
			if (cubedims.length < etldims.length)
				throw new InitializationException("Number of Source Columns must not exceed number of Cube Dimensions!");
			if (etldims.length == 0)
				throw new InitializationException("At least one Source column necessary!");				
		}		
		else { 
			if (cubedims.length != etldims.length)
			 	throw new InitializationException("Number of Dimensions has changed. Please use mode CREATE to recreate cube!");
		}
		for (int i=0; i<cubedims.length; i++) {
			IDimension dim = cubedims[i];
			String dimName = dim.getName().toLowerCase();
			if (!getMode().equals(Modes.DELETE) || getMode().equals(Modes.DELETE) && coordinateToRowMap.get(dimName)!=null) {
				// in case of delete, we don't need to get the elements there for the dimension that are not coming in the source ('*' will be used)
				dim.setCacheTrustExpiry(360000);
				dim.getElements(false);
				dimensionList.add(dim);
				//also build dimLookup to gain performance
				dimLookupMap.put(dimName, dim);
			}
			coordinateMap.put(dimName, i);
		}
		//check if dimensions match
		for (IDimension dim : etldims) {
			if (!coordinateMap.containsKey(dim.getName().toLowerCase()))
				throw new InitializationException("Dimension "+dim.getName()+" is not part of existing cube "+c.getName()+". Please use CREATE to recreate cube!");
		}
	}

	/**
	 * Internal initialization
	 *
	 */
	protected ICube initInternal(Row row) throws InitializationException {
		ICube c = null;
		try {
			IDatabase d = getConnection().getDatabase(false,true);
			IDimension[] dims = getDimensions(d,row);
			switch (getMode()) {
			case CREATE: {
				c = d.getCubeByName(getCubeName());
				//remove cube if present
				if (c != null) {
					log.info("Deleting cube "+c.getName());						
					d.removeCube(c);
				}
				c = addCube(d,dims);
				break;
			}
			case ADD : 
			case UPDATE:
			case INSERT: {
				//get cube and create if not present
				c = d.getCubeByName(getCubeName());
				if (c == null)
					c = addCube(d,dims);
				break;
			}
			case DELETE: {
				//just get cube
				c = d.getCubeByName(getCubeName());
				if (c == null)
					throw new InitializationException("Cube does not exist.");				
				break;				
			}
			default: {
				log.error("Unsupported mode in load "+getName());
			}
			}
			cellsFilled=c.getNumberOfFilledCells();
			buildCoordinateMap(c, dims);
			//clear cells if in mode update
			if (getMode().equals(Modes.UPDATE))
				c.clear();
		}
		catch (Exception e) {
			throw new InitializationException("Failed to initialize cube "+getCubeName()+": "+e.getMessage());
		}
		return c;
	}

	protected IElement[] getCoordinateElements(Row row, ICube c) {
		boolean allFound=true;
		int size = row.size();
		IElement[] elements = new IElement[size];
		for (int j = 0; j< size; j++) {
			IColumn column = row.getColumn(j);
			String elementName = column.getValueAsString();
			elements[j] = dimensionList.get(j).getElementByName(elementName,false);
			//elements[j] = dimLookupMap.get(column.getName().toLowerCase()).getElementByName(elementName,false);
			if (elements[j] == null) {
				// if no handling for missing elements is required, simply warn the user
				if(defaultType.equals(DefaultType.WARNING)){
					if (elementName.isEmpty())						
						aggLog.warn("Empty coordinate in dimension "+column.getName());
					else
						aggLog.warn("Coordinate "+ elementName+" not found in dimension "+column.getName());				
					allFound=false;
				}else{
					// handle the missing element otherwise
					elements[j] = handleMissing(column.getName().toLowerCase(),elementName,column.getName());
					if(elements[j] == null)
						allFound=false;
				}
			}
		}
		if (allFound)
			return elements;
		else
			return null;
	}

	/**
	 * According to default mode handle the missing element. If default mode is "defaultBase", the dimension element is
	 * replaced by the "default" element. If the mode is "defaultConsolidate" then the missing dimension element is created
	 * and consolidated under default element
	 * @param dimensionName the name of the dimension
	 * @param missingElement the name of the element that is missed in that dimension
	 * @return the element that will be replace the missing element
	 */
	private IElement handleMissing(String dimensionName, String missingElement, String columnName){
		if (defaultValue == null) {
			aggLog.warn("Default Element not set.");
			return null;
		}
		String resolvedValue = defaultValue.replace("?", columnName);
		IElement defaultElem = dimLookupMap.get(dimensionName).getElementByName(resolvedValue,false);

		if(defaultElem == null){
			aggLog.warn("Coordinate "+missingElement+" and default element "+resolvedValue+ " not found in dimension " + dimensionName + ". Ensure that the default element exists in this dimension.");
			return null;
		}
		if(defaultType.equals(DefaultType.MAPTODEFAULT)){
			aggLog.info("In Dimension " + dimensionName + ", element " + missingElement  + " does not exist. It will be replaced with element " + defaultValue + ".");
			return defaultElem;
		}
		else {
			if (missingElement.isEmpty()) {						
				aggLog.warn("Empty coordinate in dimension "+dimensionName);
				return null;
			}	

			IDimension dim = dimLookupMap.get(dimensionName);

			IElement newlyBuiltElement = dim.addBaseElement(missingElement, ElementType.ELEMENT_NUMERIC);
			//dim.setCacheTrustExpiry(36000);
			handler.addElement(dim, defaultElem, newlyBuiltElement);							
			log.info("Element " + missingElement + " is added to dimension " + dimensionName + ".");
			return newlyBuiltElement;
		}
	}

	protected String getValue(Row row, Modes mode, boolean numeric) {
		String value = row.getColumn(row.size()-1).getValueAsString();
		if (numeric) {
			if (mode.equals(Modes.DELETE))
				return "0";
			else
				return value.trim().isEmpty() ? "0" : value.replace(",", ".");
		}
		else {
			if (mode.equals(Modes.DELETE))
				return "";
			else
				return value;
		}
	}

	protected boolean isNumericCell (IElement[] coordinates) {
		if (coordinates == null)
			return false;
		for (IElement e : coordinates)
			if (e.getType() == ElementType.ELEMENT_STRING) {
				return false;
			}
		return true;
	}
	
	protected boolean isConsolidatedCell (IElement[] coordinates) {
		if (coordinates == null)
			return false;
		for (IElement e : coordinates)
			if (e==null || e.getType() == ElementType.ELEMENT_CONSOLIDATED) {
				return true;
			}
		return false;
	}
	
	protected int getFirstConsolidatedIndex (IElement[] coordinates) {
		for (int i=0; i<coordinates.length; i++) {
			if (coordinates[i]==null || coordinates[i].getType() == ElementType.ELEMENT_CONSOLIDATED) {
				return i;
			}
		}	
		return -1;
	}
	
	private void clearConsolidatedCell(ICube c, IElement[] coordinates) {
		IElement[][] area = new IElement[coordinates.length][];
		int index = 0;
		for (IElement coord : coordinates) {
			IElement[] elements = { coord };
			if(coord!=null){
				area[index] = elements;
			}
			index++;
		}
		try {
			c.clearCells(area);
		}
		catch (Exception ex) {
			aggLog.warn("Failed to delete cell in Cube " + getCubeName() + ": " + ex.getMessage());
		}
		countCleared++;
	}

	protected void exportData(ICube c, IProcessor rows, Modes mode) throws RuntimeException {
		//do we have to persist for a drillthrough?
		drillthroughLoader.addDrillthroughPersistor();
		
		BulkAggregator aggregator = new BulkAggregator(c);
		long coordinateTime = 0;
		
		Row inputRow = rows.next();
		if (inputRow != null) {
			//build dimension Row, which holds columns in order of the dimensions of the cube
			Row dimensionRow = new Row();
			Iterator<String> iter = coordinateMap.keySet().iterator();
			while(iter.hasNext()) {
				String coordinate=iter.next();
				// For Deletion columns may be missing in the source. The whole cube slice will be cleared in this case
				if (!(getMode().equals(Modes.DELETE) && coordinateToRowMap.get(coordinate)==null)) {
					IColumn column = inputRow.getColumn(coordinateToRowMap.get(coordinate));
					dimensionRow.addColumn(column);
				}
			}
			while ((inputRow != null) && (isExecutable())) {
				//if (row.getColumn(row.size()-1).getValueAsString().equals("0")) continue;
				long start = System.currentTimeMillis();
				IElement[] coordinates = getCoordinateElements(dimensionRow, c);
				coordinateTime += System.currentTimeMillis() - start;
				boolean numeric = isNumericCell(coordinates);
				boolean consolidated = isConsolidatedCell(coordinates);
				
				if (splashMode==SplashMode.SPLASH_NOSPLASHING && numeric && consolidated) {
					int index=getFirstConsolidatedIndex(coordinates);
					aggLog.warn("Could not write cube cell: Element "+coordinates[index].getName()+" in dimension " +  dimensionRow.getColumn(index).getName() + " is consolidated and Splashing is disabled.");						
				}
				else if (mode.equals(Modes.DELETE) && (dimensionList.size() < coordinateMap.size() || consolidated)) {
					if (dimensionList.size() < coordinateMap.size()) { //expand to full coordinates including null values.  											
						IElement[] fullCoordinates = new IElement[coordinateMap.size()];
						for (int i=0; i<dimensionList.size(); i++) {
							IDimension dim = dimensionList.get(i);
							fullCoordinates[coordinateMap.get(dim.getName().toLowerCase())] = coordinates[i];
						}
						coordinates = fullCoordinates;
					}
					clearConsolidatedCell(c, coordinates);
				}
				else {
					start = System.currentTimeMillis();
					String Value = getValue(inputRow, mode, numeric);
					start = System.currentTimeMillis();
					aggregator.add(dimensionRow,coordinates, Value, numeric, consolidated);
				}
				inputRow = rows.next();
			}
			aggregator.flush();
			aggregator.waitForCompletion();
			log.debug("Coordinate resulution took ms: "+coordinateTime);
			if (countCleared>0) {
				log.info("Consolidated cells cleared in Cube: "+countCleared);
			}
		}
	}
	
	
	protected IProcessor applyDimensionAliases(IProcessor processor) throws RuntimeException {
		if (!getConfigurator().getDimensions().isEmpty()) {
			AliasMap map = new AliasMap();
			Map<String,String> dimMap = getConfigurator().getDimensions();
			for (String input : dimMap.keySet()) {
				IColumn c = processor.current().getColumn(input);
				if (c != null) {
					AliasMapElement me = new AliasMapElement(dimMap.get(input),processor.current().indexOf(c)+1);
					map.map(me);
				} else {
					log.warn("Column "+input+" does not exist in source. Dimension mapping is ignored.");
				}
			}
			return initProcessor(new AliasProcessor(processor,map),Facets.HIDDEN);
		} else {
			return processor;
		}
	}

	public void executeLoad() {
		aggLog = new MessageHandler(log);
		log.info("Starting load "+getName()+" of Cube "+getCubeName());
		int lockId = -1;
		ICube c = null;
		
		try {
			IProcessor rows = getProcessor();
			if (rows.current()==null) {
				log.info("No data loaded to cube "+getCubeName()+". Source "+getCubeName() +" is empty.");
				return;
			}
			rows = applyDimensionAliases(rows);
			c = initInternal(rows.current());
			
			drillthroughLoader.initInternal(rows, getDatabaseName(), getCubeName(),getMode());
			
			// deactivate rules before cube load, may enhance the performance
			//boolean isOldRulesAPI = getConnection().isOlderVersion(5, 0, 4098);	
			ArrayList<IRule> deactivatedRules = new ArrayList<IRule>();
			if(doDeactivateRules) 
				deactivatedRules = deactivateRules(c);
	
			//convert cube GPU to normal when writing
			if(c.getType().equals(CubeType.CUBE_GPU)){
				log.info("Cube " + c.getName() + " is from type GPU, it will be converted.");
				c.convert(CubeType.CUBE_NORMAL);convertedFromGPU=true;
				log.info("Cube " + c.getName() + " is converted to normal cube.");
			}
	
			//lock if the cube if needed
			if(useCompleteLocking == true){
				// this should change the lockId
				lockId = tryToLock(c);
			}
			
			exportData(c, rows, getMode());
			
			// try to unlock the cube, only works when lockId does not equals -1
			lockId = tryToUnlock(lockId, c);
	
			//consolidate the new element under the default element (if needed)
			handler.loadConsolidations();
	
			//convert cube GPU back when needed
			if(convertedFromGPU){
				log.info("Cube " + c.getName() + " will be converted back to GPU.");
				c.convert(CubeType.CUBE_GPU);convertedFromGPU=false;
				log.info("Cube " + c.getName() + " is converted back to GPU.");
			}
			// reactivate rules after cube load
			reactivateRules(c, deactivatedRules);
	
			//do cleanup of dimension caches
			for (IDimension dim : dimLookupMap.values()) {
				dim.setCacheTrustExpiry(0);
			}
			dimLookupMap.clear();
	
			log.info("Finished load "+getName()+" of Cube "+getCubeName()+". Filled cells changed from "+cellsFilled+" to "+c.getNumberOfFilledCells());
	
		}
		catch (Exception e) {
			log.error("Cannot import Data into Cube "+getCubeName()+": "+e.getMessage());
			log.debug("",e);
		}
		//catch(OutOfMemoryError e1){ 
		//	log.fatal("Cannot import Data into Cube "+getCubeName()+": "+e1.getMessage());
		//	throw e1;
		//}
		finally{
			// try to again to unlock the cube, only works when lockId does not equals -1
			// this important in case an exception occured after locking and before unlocking
			if(c!=null){tryToUnlock(lockId, c);}
		}
	}

	/**
	 * @param c
	 * @return
	 */
	private int tryToLock(ICube c) {
		log.info("Locking cube "+cubeName+" for write access.");
		int lockId = -1;
		try {
			lockId = c.lockComplete();
		} catch (Exception e) {
			log.error("Error while trying to lock the cube: " + e.getMessage());
		}
		return lockId;
	}

	/**
	 * @param lockId
	 * @param c
	 * @return 
	 */
	private int tryToUnlock(int lockId, ICube c) {
		if(lockId != -1){
			log.info("Releasing lock for cube "+cubeName+".");
			try {
				c.commitLock(lockId);
			} catch (Exception e) {
				log.error("Error while trying to commit the cube and free it's lock:" + e.getMessage());
			}
		}
		return -1;// always
	}

	/**
	 * needs properties: connection, database, root, mode
	 */
	public void init() throws InitializationException {
		try {
			super.init();
			switch (getConfigurator().getSplashMode()) {
			case DEFAULT: splashMode = SplashMode.SPLASH_DEFAULT; break;
			case ADD: splashMode = SplashMode.SPLASH_ADD; break;
			case SET: splashMode = SplashMode.SPLASH_SET; break;
			case DISABLED: splashMode = SplashMode.SPLASH_NOSPLASHING; break;
			default: splashMode = SplashMode.SPLASH_DEFAULT;
			}
			bulkSize = getConfigurator().getBulkSize();
			cubeName = getConfigurator().getCubeName();
			aggregate = !(getMode().equals(Modes.INSERT) || getMode().equals(Modes.DELETE));
			if ((splashMode.equals(SplashMode.SPLASH_SET) || splashMode.equals(SplashMode.SPLASH_ADD))  && aggregate)
				throw new InitializationException("SplashMode " + (splashMode.equals(SplashMode.SPLASH_SET)?"Set" : "Add") + " can only be used with Mode INSERT or DELETE");

			drillthroughLoader.init(getConfigurator().getDrillthroughDescription());
			if (getConfigurator().getDrillthroughDescription()!=null && splashMode!=SplashMode.SPLASH_NOSPLASHING) {
				throw new InitializationException("Cube Load with Drillthrough should only be used with SplashMode DISABLED.");
			}
			defaultType = getConfigurator().getDefaultType();
			defaultValue = getConfigurator().getDefaultValue();
			if (getConfigurator().getDrillthroughDescription()!=null && (defaultType.equals(DefaultType.MAPTODEFAULT))) {
				throw new InitializationException("Cube Load with Drillthrough can not be used with default type +"+DefaultType.MAPTODEFAULT+".");
			}			
			doDeactivateRules = getConfigurator().getDeactivateRules();
			useEventProcessor = getConfigurator().useEventProcessor();
			useCompleteLocking = getConfigurator().getCompleteLocking();
			multiThreaded = getConfigurator().getParameter("separateWriteThread", "true").equalsIgnoreCase("true");
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}