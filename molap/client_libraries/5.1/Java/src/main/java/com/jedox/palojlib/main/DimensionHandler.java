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
 *   You may obtain a copy of the License at
*
 *   If you are developing and distributing open source applications under the
 *   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 *   ISVs, and VARs who distribute Palo with their products, and do not license
 *   and distribute their source code under the GPL, Jedox provides a flexible
 *   OEM Commercial License.
 *
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.jedox.palojlib.managers.HttpHandlerManager;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.interfaces.ICube.SplashMode;
import com.jedox.palojlib.interfaces.IElement.*;
import com.jedox.palojlib.util.Helpers;

/**
 * handler used by {@link Dimension} to make request on olap server
 * @author khaddadin
 *
 */
public final class DimensionHandler{

	private static String DIMENSION_INFO_REQUEST = "/dimension/info?";
	private static String DIMENSION_RENAME_REQUEST = "/dimension/rename?";
	private static String DIMENSION_ELEMENTS_REQUEST = "/dimension/elements?";
	private static String DIMENSION_ELEMENT_REPLACE_BULK_REQUEST = "/element/replace_bulk?";
	//private static String DIMENSION_ELEMENT_CREATE_BULK_REQUEST = "/element/create_bulk?";
	//private static String DIMENSION_ELEMENT_CREATE_REQUEST = "/element/create?";
	private static String DIMENSION_ELEMENT_REPLACE_REQUEST = "/element/replace?";
	private static String DIMENSION_ELEMENT_DESTROY_BULK_REQUEST = "/element/destroy_bulk?";
	private static String DIMENSION_ELEMENT_MOVE_BULK_REQUEST = "/element/move_bulk?";
	private static String ELEMENT_INFO_REQUEST = "/element/info?";
	private final String contextId;
	private final CubeHandler cubehandler;
	protected boolean withPermission = false;

	protected DimensionHandler(String contextId) throws PaloException, PaloJException{
		this.contextId = contextId;
		cubehandler = new CubeHandler(contextId);
	}

	protected DimensionInfo getInfo(int databaseId,int dimensionId) throws PaloException, PaloJException{

		StringBuilder DIMENSION_INFO_REQUEST_BUFFER = new StringBuilder(DIMENSION_INFO_REQUEST);
		StringBuilder currentRequest = DIMENSION_INFO_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(dimensionId);
		String [][]response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		return new DimensionInfo(response[0][7],response[0][8],response[0][3],response[0][4],response[0][5],response[0][10],response[0][2]);
	}
	
	public IElement getSingleElement(Database database,int dimensionId,String name, boolean withAttributes) {

		StringBuilder ELEMENT_INFO_REQUEST_BUFFER = new StringBuilder(ELEMENT_INFO_REQUEST);
		StringBuilder currentRequest = ELEMENT_INFO_REQUEST_BUFFER.append("database=").append(database.getId()).append("&dimension=").append(dimensionId).append("&name_element=").append(Helpers.urlEncode(name));
		String[][] response;
		try {
			response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		} catch (Exception e) {
			return null;
		}
		int elementId = Integer.parseInt(response[0][0]);
		ElementType type = parseType(Integer.parseInt(response[0][6]));
		
		Element e = new Element(contextId, database, dimensionId, elementId, name, type,0,new int[0],new int[0],new double[0],null,null,-1,null,true);
		
		if(withAttributes){

			HashMap<String,Cell> cellsMap = new HashMap<String, Cell>();
			Dimension dimension = database.getDimensionById(dimensionId);
			int attributesNum = dimension.attributesIdMap.size();
			
			//if this dim has an attribute dimension
			if(attributesNum != 0){
				cellsMap= cubehandler.extractCellsMap(database.getId(), dimension.getDimensionInfo().getAttributeCubeId(), CellsExportType.BOTH, 10000, true, false, true, new Element[][]{null,new Element[]{e}},database.getCubeById(dimension.getDimensionInfo().getAttributeCubeId()).getDimensions());
				database.getCubeById(dimension.getDimensionInfo().getAttributeCubeId()).getCubeInfo();
				HashMap<String,Object> attributesValues = new HashMap<String, Object>();
				
				for(Element att:dimension.attributesIdMap.values()){
					int[] path = {att.getId(),elementId};
					String attName = att.getName();
					Cell cell = cellsMap.get(Helpers.getConcatFromArray(path));// the form e.g. 1,32
					boolean isStringElement = type.equals(ElementType.ELEMENT_STRING);
					if(cell != null)
						attributesValues.put(attName, cell.getValue());
					else if(att.getType().equals(ElementType.ELEMENT_STRING) || isStringElement)
						attributesValues.put(attName, "");
					else
						attributesValues.put(attName, 0);
				}
				
				e.attributeValues = attributesValues;
			}
		}
		
		
		return e;
		
	}

	protected void removeConsolidations(int databaseId,int dimensionId,IElement[] elements) throws PaloException, PaloJException{
		StringBuilder DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_NUMERIC_BUFFER = new StringBuilder(DIMENSION_ELEMENT_REPLACE_BULK_REQUEST );
		StringBuilder DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_STRING_BUFFER = new StringBuilder(DIMENSION_ELEMENT_REPLACE_BULK_REQUEST );
		StringBuilder stringElementsId = new StringBuilder();
		StringBuilder numericElementsId = new StringBuilder();
		for(IElement ie:elements ){
			Element e = (Element) ie;
			if(e.getType().equals(ElementType.ELEMENT_NUMERIC ) || e.getType().equals(ElementType.ELEMENT_CONSOLIDATED))
				numericElementsId = numericElementsId.append(e.getId()).append(",");
			else
				stringElementsId = stringElementsId.append(e.getId()).append(",");
		}

		if(numericElementsId.length()  != 0){
			numericElementsId.deleteCharAt(numericElementsId.length()-1);
			StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_NUMERIC_BUFFER.append("database=").append(databaseId).append("&dimension=").append(dimensionId).append("&elements=").append(numericElementsId.toString()).append("&type=1");
			HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		}

		if(stringElementsId.length()  != 0){
			stringElementsId.deleteCharAt(stringElementsId.length()-1);
			StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_STRING_BUFFER.append("database=").append(databaseId).append("&dimension=").append(dimensionId).append("&elements=").append(stringElementsId.toString()).append("&type=2");
			HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		}

	}

	public void removeAttributes(int databaseId, int attributeDimensionId,int[] ids) throws PaloException, PaloJException {

		StringBuilder DIMENSION_ELEMENT_DESTROY_BULK_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENT_DESTROY_BULK_REQUEST );

		StringBuilder elementsId = new StringBuilder();
		for(int id:ids)
			elementsId = elementsId.append(id).append(",");

		if(elementsId.length()  != 0){
			elementsId.deleteCharAt(elementsId.length()-1);
			StringBuilder currentRequest = DIMENSION_ELEMENT_DESTROY_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(attributeDimensionId).append("&elements=").append(elementsId.toString());
			HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);

		}

	}

	public void removeElements(int databaseId, int dimensionId, int[] ids) throws PaloException, PaloJException {

		StringBuilder DIMENSION_ELEMENT_DESTROY_BULK_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENT_DESTROY_BULK_REQUEST );

		StringBuilder elementsId = new StringBuilder();
		for(int id:ids)
			elementsId = elementsId.append(id).append(",");

		if(elementsId.length()  != 0){
			elementsId.deleteCharAt(elementsId.length()-1);
			StringBuilder currentRequest = DIMENSION_ELEMENT_DESTROY_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(dimensionId).append("&elements=").append(elementsId.toString());
			HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		}

	}

	protected void setElementsWithAttributes(int id, Dimension dimension, Database database) throws PaloException, PaloJException{

		StringBuilder DIMENSION_ELEMENTS_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENTS_REQUEST);
		int major = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).majorVersion;
		int minor = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).minorVersion;
		boolean withMode = (major>=6 || (major>=5 && minor>=1));
		StringBuilder currentRequest = DIMENSION_ELEMENTS_REQUEST_BUFFER.append("database=").append(database.getId()).append("&dimension=").append(id).append((withPermission?"&show_permission=1":"")).append((withMode?"&mode=1":""));
		
		String [][]response =  HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);

		/**the attribute should be read */
		boolean hasAttributes = true;
		HashMap<String,Cell> cellsMap = new HashMap<String, Cell>();
		int attributesNum = dimension.attributesIdMap.size();
		int dimensionToken = dimension.getDimensionInfo().getToken();

		//if this dim has an attribute dimension
		if(attributesNum != 0){
			cellsMap= cubehandler.extractCellsMap(database.getId(), dimension.getDimensionInfo().getAttributeCubeId(), CellsExportType.BOTH, 100000, true, false, true, new Element[][]{null,null},database.getCubeById(dimension.getDimensionInfo().getAttributeCubeId()).getDimensions());
			database.getCubeById(dimension.getDimensionInfo().getAttributeCubeId()).getCubeInfo();

		}else{
			hasAttributes = false;
		}
		
		int i=0;
		boolean hiddenFiltered = false;
		if(withMode){
			i++;
			if(response[0][0].equals("0"))
				hiddenFiltered= true;
		}
			

		for(;i<response.length;i++){

			int elementId = Integer.parseInt(response[i][0]);
			String name = response[i][1];
			int position = Integer.parseInt(response[i][2]);
			ElementType type = parseType(Integer.parseInt(response[i][6]));
			String[] parentsIds = (response[i][8].equals("")?new String[0]:(response[i][8]).split(","));
			int[] parentsIdsInt = new int[parentsIds.length];
			for(int j=0;j<parentsIds.length;j++){
				parentsIdsInt[j] = Integer.parseInt(parentsIds[j]);
			}
			String[] childrenIds = (response[i][10].equals("")?new String[0]:(response[i][10]).split(","));
			String[] weights = ((response[i][11].equals(""))?new String[0]:(response[i][11]).split(","));
			int[] childrenIdsInt = new int[childrenIds.length];
			double[] weightsDouble = new double[weights.length];
			
			// The number of children and weight should be the same
			for(int j=0;j<childrenIds.length;j++){
				dimension.hasConsolidatedElements = true;
				childrenIdsInt[j] = Integer.parseInt(childrenIds[j]);
				weightsDouble[j] = Double.parseDouble(weights[j]);
			}
			

			HashMap<String,Object> attributesValues = new HashMap<String, Object>();
			boolean isStringElement = type.equals(ElementType.ELEMENT_STRING);
			
			if(hasAttributes){
				for(Element att:dimension.attributesIdMap.values()){
					int[] path = {att.getId(),elementId};
					String attName = att.getName();
					Cell cell = cellsMap.get(Helpers.getConcatFromArray(path));// the form e.g. 1,32
					if(cell != null)
						attributesValues.put(attName, cell.getValue());
					else if(att.getType().equals(ElementType.ELEMENT_STRING) || isStringElement)
						attributesValues.put(attName, "");
					else
						attributesValues.put(attName, 0);
				}
			}
			Element e = new Element(contextId, database,id,elementId,name,type,position,parentsIdsInt,childrenIdsInt,weightsDouble,attributesValues,dimension.attributesNameMap,dimensionToken,response[i][12],hiddenFiltered);
			dimension.elementsIdMap.put(elementId,e);
			dimension.elementsNameMap.put(name.toLowerCase(),e);
		}
	}

	private ElementType parseType(int typeInt) {
		ElementType type = ElementType.ELEMENT_NUMERIC;
		switch(typeInt){
			case 2: type=ElementType.ELEMENT_STRING;break;
			case 4: type=ElementType.ELEMENT_CONSOLIDATED;break;
		}

		return type;
	}
	
	public void updateConsolidations(int databaseId, int id, IConsolidation[] consolidations) throws PaloException, PaloJException {
		StringBuilder DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENT_REPLACE_BULK_REQUEST );
		StringBuilder elementsId = new StringBuilder();
		StringBuilder childrenId = new StringBuilder();
		StringBuilder weightsLists = new StringBuilder();

		HashMap <Element, List <Element>> childMap = new HashMap<Element, List<Element>>();
		HashMap <Element, List <Double>> weightMap = new HashMap<Element, List<Double>>();
		for (IConsolidation c: consolidations) {
			if (!childMap.containsKey(c.getParent())) {
				childMap.put((Element)c.getParent(), new ArrayList<Element>());
				weightMap.put((Element)c.getParent(), new ArrayList<Double>());
			}
			if(c.getChild()!=null){
				childMap.get((Element)c.getParent()).add((Element)c.getChild());
				weightMap.get((Element)c.getParent()).add(c.getWeight());
			}
		}

		ArrayList<Element> elementsWithNoChildren = new ArrayList<Element>();
		
		for(Element element: childMap.keySet()){

			List<Element> childrenList = childMap.get(element);
			if(childrenList.size()!=0){
				elementsId.append(element.getId()).append(",");
				List<Double> weightList = weightMap.get(element);
				for(int j=0;j<childrenList.size()-1;j++){
					childrenId.append(childrenList.get(j).getId()).append(",");
					weightsLists.append(weightList.get(j)).append(",");
				}
				childrenId.append(childrenList.get(childrenList.size()-1).getId()).append(":");
				weightsLists.append(weightList.get(childrenList.size()-1)).append(":");
	
			}else{
				elementsWithNoChildren.add(element);
			}
		}
		
		childMap.clear();
		weightMap.clear();
		
		if(elementsId.length()>0)
			elementsId.deleteCharAt(elementsId.length()-1);
		
		if(childrenId.length()>0){
			childrenId.deleteCharAt(childrenId.length()-1);
			weightsLists.deleteCharAt(weightsLists.length()-1);
		}

		StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&elements=").append(elementsId).append("&children=").append(childrenId).append("&weights=").append(weightsLists).append("&type=4");
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
		if(elementsWithNoChildren.size()!=0){
			removeConsolidations(databaseId, id, elementsWithNoChildren.toArray(new Element[0]));
		}

	}
	
	public void addElements(int databaseId, int id, String[] names, ElementType[] types) throws PaloException, PaloJException {
		
		int major = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).majorVersion;
		int minor = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).minorVersion;
		//starting 5.1 use the new types parameter
		boolean withTypes = (major>=6 || (major>=5 && minor>=1));
		StringBuilder DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENT_REPLACE_BULK_REQUEST);
		
		if(withTypes){
			StringBuilder elementsBuffer = new StringBuilder();
			StringBuilder typesBuffer = new StringBuilder();
			for(int i=0;i<names.length;i++){
				String name = Helpers.urlEncode(Helpers.addDoubleQuotes(names[i]));
				elementsBuffer.append(name).append(",");
				if(types[i].equals(ElementType.ELEMENT_NUMERIC) || types[i].equals(ElementType.ELEMENT_CONSOLIDATED))
					typesBuffer.append("1").append(",");
				else
					typesBuffer.append("2").append(",");
			
			}
			
			if(elementsBuffer.length()> 0){
				elementsBuffer.deleteCharAt(elementsBuffer.length()-1);
				typesBuffer.deleteCharAt(typesBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&name_elements=").append(elementsBuffer.toString()).append("&types=").append(typesBuffer.toString());
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
			
		}else{

			StringBuilder numericElementsBuffer = new StringBuilder();
			StringBuilder stringElementsBuffer = new StringBuilder();
			for(int i=0;i<names.length;i++){
				String name = Helpers.urlEncode(Helpers.addDoubleQuotes(names[i]));
				if(types[i].equals(ElementType.ELEMENT_NUMERIC) || types[i].equals(ElementType.ELEMENT_CONSOLIDATED))
					numericElementsBuffer.append(name).append(",");
				else
					stringElementsBuffer.append(name).append(",");
			
			}
			
			if(numericElementsBuffer.length()> 0){
				numericElementsBuffer.deleteCharAt(numericElementsBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&name_elements=").append(numericElementsBuffer.toString()).append("&type=").append("1");
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
			
			if(stringElementsBuffer.length()> 0){
				stringElementsBuffer.deleteCharAt(stringElementsBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&name_elements=").append(stringElementsBuffer.toString()).append("&type=").append("2");
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
		}
	}
	
	public void moveElements(int databaseId, int id, IElement[] elements, Integer[] positions) {
		
		if(positions.length!=elements.length)
			throw new PaloJException("Positions and elements arrays should have the same length");
		
		if(positions.length<1)
			throw new PaloJException("Positions array length should be at least one.");
		
		int major = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).majorVersion;
		int minor = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).minorVersion;
		int buildNumber = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).buildNumber;
		//starting 5.1.5359 use the new types parameter
		boolean withMoveBulk = (major>=6 || (major>=5 && minor>=1 && buildNumber>5359 ));
		if(positions.length>1 && withMoveBulk){
			StringBuilder DIMENSION_ELEMENT_MOVE_BULK_REQUEST_BUILDER = new StringBuilder(DIMENSION_ELEMENT_MOVE_BULK_REQUEST);
			StringBuffer elementsBuffer = new StringBuffer();
			StringBuffer positionsBuffer = new StringBuffer();
			for(int i=0;i<elements.length;i++){
				elementsBuffer.append(((Element)elements[i]).getId()).append(",");
				positionsBuffer.append(positions[i]).append(",");
			}
			
			if(elementsBuffer.length()> 0){
				elementsBuffer.deleteCharAt(elementsBuffer.length()-1);
				positionsBuffer.deleteCharAt(positionsBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_MOVE_BULK_REQUEST_BUILDER.append("database=").append(databaseId).append("&dimension=").append(id).append("&elements=").append(elementsBuffer.toString()).append("&positions=").append(positionsBuffer.toString());
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
			
		}else{
			// move it in single mode
			for(int i=0;i<elements.length;i++){
				elements[i].move(positions[i]);
			}
		}
		
		
	}
	
	public void addElements(int databaseId, int id, IElement[] elements) throws PaloException, PaloJException {
		
		int major = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).majorVersion;
		int minor = HttpHandlerManager.getInstance().getHttpHandlerInfo(contextId).minorVersion;
		//starting 5.1 use the new types parameter
		boolean withTypes = (major>=6 || (major>=5 && minor>=1));
		StringBuilder DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENT_REPLACE_BULK_REQUEST);
		
		if(withTypes){
			StringBuilder elementsBuffer = new StringBuilder();
			StringBuilder typesBuffer = new StringBuilder();
			for(int i=0;i<elements.length;i++){
				String name = Helpers.urlEncode(Helpers.addDoubleQuotes(elements[i].getName()));
				elementsBuffer.append(name).append(",");
				if(elements[i].getType().equals(ElementType.ELEMENT_NUMERIC) || elements[i].getType().equals(ElementType.ELEMENT_CONSOLIDATED))
					typesBuffer.append("1").append(",");
				else
					typesBuffer.append("2").append(",");
			
			}
			
			if(elementsBuffer.length()> 0){
				elementsBuffer.deleteCharAt(elementsBuffer.length()-1);
				typesBuffer.deleteCharAt(typesBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&name_elements=").append(elementsBuffer.toString()).append("&types=").append(typesBuffer.toString());
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
			
		}else{

			StringBuilder numericElementsBuffer = new StringBuilder();
			StringBuilder stringElementsBuffer = new StringBuilder();
			for(int i=0;i<elements.length;i++){
				String name = Helpers.urlEncode(Helpers.addDoubleQuotes(elements[i].getName()));
				if(elements[i].getType().equals(ElementType.ELEMENT_NUMERIC) || elements[i].getType().equals(ElementType.ELEMENT_CONSOLIDATED))
					numericElementsBuffer.append(name).append(",");
				else
					stringElementsBuffer.append(name).append(",");
			
			}
			
			if(numericElementsBuffer.length()> 0){
				numericElementsBuffer.deleteCharAt(numericElementsBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&name_elements=").append(numericElementsBuffer.toString()).append("&type=").append("1");
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
			
			if(stringElementsBuffer.length()> 0){
				stringElementsBuffer.deleteCharAt(stringElementsBuffer.length()-1);
				StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_BULK_REQUEST_BUFFER.append("database=").append(databaseId).append("&dimension=").append(id).append("&name_elements=").append(stringElementsBuffer.toString()).append("&type=").append("2");
				HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
			}
		}
	}
	
	public Element addBaseElement(Database database, int id, String name, ElementType elementType) throws PaloJException, PaloException {
		
		StringBuilder DIMENSION_ELEMENT_REPLACE_REQUEST_BUFFER = new StringBuilder(DIMENSION_ELEMENT_REPLACE_REQUEST);
		name = Helpers.urlEncode(name);
		if(!elementType.equals(ElementType.ELEMENT_NUMERIC) && !elementType.equals(ElementType.ELEMENT_STRING))
			throw new PaloJException("only numeric and string type are allowed in this method.");

		StringBuilder currentRequest = DIMENSION_ELEMENT_REPLACE_REQUEST_BUFFER.append("database=").append(database.getId()).append("&dimension=").append(id).append("&name_element=").append(name).append("&type=" + (elementType.equals(ElementType.ELEMENT_NUMERIC)?1:2));
		String[][] response = HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
		int elementId = Integer.parseInt(response[0][0]);
		String elementname = response[0][1];
		int position = Integer.parseInt(response[0][2]);
		return new Element(contextId, database,id,elementId,elementname,elementType,position,new int[0],new int[0],new double[0],new HashMap<String, Object>(),new HashMap<String, Element>(),-1,null,true);
	}

	public void addAttributeValues(Database database, int attributeCubeId,int attributeDimId, int id, IElement[] elements, Object[] values) throws PaloException, PaloJException {


		Cube attributeCube = database.getCubeById(attributeCubeId);
		Dimension dim = attributeCube.getDimensionById(attributeDimId);
		Element attribute = dim.getElementById(id);
		Element[][] paths = new Element[elements.length][2];
		for(int i=0;i<elements.length;i++){
			paths[i]= new Element[]{attribute,(Element)elements[i]};
		}
		attributeCube.loadCells(paths, values, new CellLoadContext(SplashMode.SPLASH_DEFAULT, Integer.MAX_VALUE, false, true),null);

	}

	protected void removeAttributeValues(Database database,int attributeCubeId,int attributeDimId, int id, IElement[] elements) throws PaloException, PaloJException {

		Cube attributeCube = database.getCubeById(attributeCubeId);
		Dimension dim = attributeCube.getDimensionById(attributeDimId);
		Element attribute = dim.getElementById(id);
		IElement[][] area =new IElement[2][];
		area[0]= new IElement[]{attribute};
		area[1]=elements;
		attributeCube.clearCells(area);

	}

	public void rename(Database database, int id, String name) throws PaloException, PaloJException {
		StringBuilder RENAME_DIMENSION_REQUEST_BUFFER = new StringBuilder(DIMENSION_RENAME_REQUEST);
		StringBuilder currentRequest = RENAME_DIMENSION_REQUEST_BUFFER.append("dimension=").append(id).append("&database=").append(database.getId()).append("&new_name=").append(name);
		HttpHandlerManager.getInstance().getHttpHandler(contextId).send(currentRequest,true,false);
		
	}

	public void setWithElementPermission(boolean withPermission) {
		this.withPermission = withPermission;
		
	}

}
