package com.jedox.etl.core.olap4j;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;


import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.olap4j.Cell;
import org.olap4j.CellSetAxis;
import org.olap4j.OlapStatement;
import org.olap4j.CellSet;
import org.olap4j.OlapException;
import org.olap4j.PreparedOlapStatement;
import org.olap4j.metadata.Dimension;
import org.olap4j.metadata.Level;
import org.olap4j.metadata.Member;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;


public class Olap4jCellsExporter implements ICellsExporter {
	
	private class QueryDefinition {
		public String query;
		public String[] slicerNames;
		public String[] uniqueSlicerNames;
		public boolean hasMoreData;
	}
	
	private class Fetcher extends Thread {
		
		public Fetcher() {
			super();
			this.setName(Thread.currentThread().getName());
		}
		
		private CellSet nextCellSet;
		private String[] nextSlicerNames;
		private boolean hasMoreData;
		
		public synchronized void run() {
			log.debug("Starting fetch thread.");
			QueryDefinition def = getNextSliceQuery();
			nextSlicerNames = def.slicerNames;
			hasMoreData = def.hasMoreData;
			nextCellSet = null;
			try {
				nextCellSet = getCellSet(def.query);
			}
			catch (OlapException e) {
				log.error("Error while iterating mdx cell export: "+e.getMessage());
				e.printStackTrace();
			}
			catch (OutOfMemoryError e1) {
				System.err.println("Failed to extract data slice from Cube " + cubewrapper.getCube().getName() + ": " + e1.getMessage());
				log.fatal("Failed to extract data slice from Cube " + cubewrapper.getCube().getName() + ": " + e1.getMessage());
				throw e1;
			}
			log.debug("Finished fetch thread.");
		}
		
		public synchronized CellSet getNextCellSet() {
			return nextCellSet;
		}
		
		public synchronized String[] getNextSlicerNames() {
			return nextSlicerNames;
		}
		
		public boolean hasMoreData() {
			return hasMoreData;
		}
		
	}
	
	
	private CellSet cellSet;
	private int ordinal = 0;
	private CellWrapper cell;
	private boolean skipEmpty;
	private CubeWrapper cubewrapper;
	private CellsExportType exportType;
	private boolean useRules;
	private boolean onlyBases;
	private boolean aggregate;
	private boolean iterating;
	private long iterationCount=0;
	private long iterationSum=1;
	private String[] slicernames;
	private Map<Dimension,Integer> slicerPositions = new HashMap<Dimension, Integer>();
	private Map<Dimension,List<Member>> cellSetMembers = new HashMap<Dimension, List<Member>>();
	private Map<Dimension,List<Member>> slicerMembers = new HashMap<Dimension, List<Member>>();
	private List<Integer> pathOrder;
	private List<String> baseSetAxisQuery;
	private Fetcher fetcher;
	private static final Log log = LogFactory.getLog(Olap4jCellsExporter.class);
	
	public Olap4jCellsExporter(IElement[][] area, ExtendedCellExportContext context,
			CubeWrapper cubewrapper) throws PaloException {
		
		try {
			this.aggregate = true;  
			this.skipEmpty = context.isSkipEmpty();
			this.cubewrapper = cubewrapper;
			this.useRules = context.isUseRules();
			this.onlyBases = context.isOnlyBases();
			this.exportType = context.getCellsExportType();
			//this.iterating = false;
			this.iterating = (context.getBlockSize() > 0) || !context.getSlicerDimensions().isEmpty();
			
			List<Dimension> dimensions = cubewrapper.getCube().getDimensions();
			String query;
			if (!iterating) {
				//apply only cellSetMember keys needed for getDimensions()
				for (int i=0; i<area.length; i++) {
					if (!aggregate || area[i] != null) cellSetMembers.put(dimensions.get(i), null);
				}
				//do direct one shot query
				query = getOneShotQuery(area);
				cellSet = getCellSet(query);
			}
			else {
				if (dimensions.isEmpty()) throw new PaloException("Cube "+cubewrapper.getName()+" has no dimensions.");
				//rewrite area with cube restrictions
				for (int i=0; i<area.length; i++) {
					Dimension dim = dimensions.get(i);
					List<Member> members = null;
					if (!aggregate && area[i] == null) {
						members = applyCubeRestrictions(dim,cubewrapper.getDimensionByName(dim.getName()).getElements(false));
					}
					if (area[i] != null) {
						members = applyCubeRestrictions(dim,area[i]);
					}
					if (members != null) {
						if (members.isEmpty()) 
							log.warn("Restrictions for dimension "+dim.getName()+" returned no members. Result will be empty.");
						if (members.size() == 1) {
							slicerMembers.put(dim, members);
							slicerPositions.put(dim,Integer.valueOf(0));
						} 
						else {
							cellSetMembers.put(dim, members);
						}
					}
				}
				if (!context.getSlicerDimensions().isEmpty()) {
					// add fixed slicer dimensions
					Map <String, Dimension> dimmap = new HashMap<String,Dimension>();
					for (Dimension dim : dimensions) {
						dimmap.put(dim.getName().toLowerCase(), dim);
					}	
					for (String slicer : context.getSlicerDimensions()) {
						if (!dimmap.containsKey(slicer.toLowerCase()))
							log.warn("Fixed slicer "+slicer+" is not a dimension of cube "+cubewrapper.getName()+". It is ignored.");
						addDimensionToSlicer(dimmap.get(slicer.toLowerCase()));
					}  
				}
				else {
					// check if slicing is necessary and shift dimensions to slicer automatically
					while (getMaxCellSetSize() / context.getBlockSize() > getSlicerSize()) {
						if (!shiftSmallestToSlicer()) break;;
					}
				}	
				iterationSum=getSlicerSize();
				if (iterationSum>1) {
					log.info("Very large cellSet is being requested. Splitting request to "+iterationSum+" slices on dimensions "+getSlicerMessage()+". Consider refining your filter.");
				}					
				
				baseSetAxisQuery = getBaseSetAxisQuery();
				QueryDefinition def = getNextSliceQuery();
				slicernames = def.slicerNames;
				query = def.query;
				cellSet = getCellSet(query);
				if (def.hasMoreData) { //there is another slice. fetch it in background
					fetcher = new Fetcher();
					fetcher.start();
				}
			}
		}
		catch (OlapException e) {
			e.printStackTrace();
			throw new PaloException("Failed to execute cell export: "+e.getMessage());
		}		
	}
	
    	
	private long getMaxCellSetSize() {
		long size = 1;
		for (List<Member> members : cellSetMembers.values()) {
			size *= members.size();
		}
		return size;
	}
	
	private long getSlicerSize() {
		long size = 1;
		for (List<Member> members : slicerMembers.values()) {
			size *= members.size();
		}
		return size;
	}

	private String getSlicerMessage() {
		String message="";
		for (Dimension dim : slicerMembers.keySet()) {
			message=message+dim.getName()+" ("+slicerMembers.get(dim).size()+"), ";
		}
		if (message.length()>0)
			return message.substring(0, message.length()-2);
		else
			return "";
	}
	
	private void addDimensionToSlicer(Dimension dim) {
		slicerMembers.put(dim, cellSetMembers.remove(dim));
		slicerPositions.put(dim, Integer.valueOf(0));
	}
	
	
	private boolean shiftSmallestToSlicer() {
		if (!cellSetMembers.isEmpty()) {
			Dimension smallest = cellSetMembers.keySet().iterator().next();
			for (Dimension dim : cellSetMembers.keySet()) {
				List<Member> members = cellSetMembers.get(dim);
				if (members.size() < cellSetMembers.get(smallest).size()) smallest = dim; 
			}
			if (cellSetMembers.get(smallest).size() > 1000) return false; //do not allow dimensions > 100 in slicer.
			addDimensionToSlicer(smallest);
			return true;
		}
		return false;
	}
	
	private CellSet getCellSet(String query) throws OlapException {
		OlapStatement statement = cubewrapper.getCube().getSchema().getCatalog().getDatabase().getOlapConnection().createStatement();
		if (!query.isEmpty()) {
			iterationCount++;
			log.info("Starting MDX-Request " + ((iterationSum>1) ? iterationCount+"/"+iterationSum : ""));
			log.debug("MDX-Statement: "+query);
			CellSet cellSet = statement.executeOlapQuery(query);
			log.debug("Sucessfully got MDX result.");
			return cellSet;
		}
		return null;
	}
	
	//not in use, because it dows not work on all mdx dialects...
	private PreparedOlapStatement getPreparedStatement(String query) throws OlapException {
		if (!query.isEmpty()) {
			iterationCount++;
			log.info("Starting MDX-Request " + ((iterationSum>1) ? iterationCount+"/"+iterationSum : ""));
			return cubewrapper.getCube().getSchema().getCatalog().getDatabase().getOlapConnection().prepareOlapStatement(query);
		}
		return null;
	}
	
	
	private boolean matchesExportType(ElementType elementType, CellsExportType exportType) {
		return true; //deactivate this until element types do work properly.
		/*
		switch (exportType) {
		case ONLY_NUMERIC : return elementType.equals(ElementType.ELEMENT_NUMERIC);
		case ONLY_STRING: return elementType.equals(ElementType.ELEMENT_STRING);
		default: return true;
		}
		*/
	}
	
	private List<Member> applyCubeRestrictions(Dimension dimension, IElement[] elements) {
		List<Member> result = new ArrayList<Member>();
		for (int j=0; j<elements.length; j++) {
			if ((!onlyBases || elements[j].getChildCount() == 0) && (matchesExportType(elements[j].getType(),exportType))) {
				Member m = cubewrapper.getDimensionByName(dimension.getName()).getElementCache().getMembers(elements[j].getName()).get(0);
				if (useRules || !m.isCalculated()) result.add(m);
			}
		}
		return result;
	}
	
	private String getDimensionMembersEnumeration(List<Member> members) {
		StringBuilder buffer = new StringBuilder();
		for (Member m : members) {
			buffer.append(m.getUniqueName()+",");
		}
		buffer.deleteCharAt(buffer.length()-1);
		return buffer.toString();
	}
	
	private String getNonSpecifiedDimensionMembersByMDX(Dimension dimension) throws OlapException {
		StringBuilder buffer = new StringBuilder();
		if (!onlyBases) { //take all memmbers of dimension
			buffer.append("["+dimension.getName()+"].MEMBERS");
		}
		else { //take only leaf members of dimension via mdx function (more performant that enumerating potentially all members explicitly in statement)
			List<Level> levels = dimension.getDefaultHierarchy().getLevels();
			int maxLevel = levels.size()+1;
			for (Member m : dimension.getDefaultHierarchy().getRootMembers()) {
				if (m.getChildMemberCount() > 0) { //root element is no leaf member. Take leafes
					buffer.append("Descendants("+m.getUniqueName()+","+maxLevel+",LEAVES),");
				}
				else { //root element is itself an leaf member
					buffer.append(m.getUniqueName()+",");
				}
			}
			buffer.deleteCharAt(buffer.length()-1);
		}
		return buffer.toString();
	}
	
	private String getOneShotQuery(IElement[][] area) throws OlapException {
		List<Dimension> dimensions = cubewrapper.getCube().getDimensions();
		StringBuilder buffer = new StringBuilder("SELECT ");
		int axisCount = 0;
		for (Dimension d : cellSetMembers.keySet()) {
			int i = dimensions.indexOf(d);
			if (!aggregate || area[i] != null) { //no aggregation. Try to detect suitable members for this dimension.
				if (skipEmpty) buffer.append("NON EMPTY ");
				if (!useRules) buffer.append("STRIPCALCULATEDMEMBERS(");
				buffer.append("{");
				if (area[i] == null) { // no dimension filter present
					buffer.append(getNonSpecifiedDimensionMembersByMDX(dimensions.get(i)));
				}
				else { //take elements from dimension filter.
					buffer.append(getDimensionMembersEnumeration(applyCubeRestrictions(dimensions.get(i), area[i])));
				}
				buffer.append("}");
				if (!useRules) buffer.append(")");
				buffer.append(" ON ");
				buffer.append(String.valueOf(axisCount));
				buffer.append(",");
				axisCount++;
			}
		}
		buffer.deleteCharAt(buffer.length()-1);
		buffer.append(" FROM ["+cubewrapper.getName()+"]");
		return buffer.toString();
	}
	
	private List<String> getBaseSetAxisQuery() {
		List<String> result = new ArrayList<String>();
		int axisCount = 0;
		for (Dimension dim : cellSetMembers.keySet()) {
			List<Member> members = cellSetMembers.get(dim);
			StringBuilder buffer = new StringBuilder();
			if (skipEmpty) buffer.append("NON EMPTY ");
			if (!useRules) buffer.append("STRIPCALCULATEDMEMBERS(");
			buffer.append("{");
			buffer.append(getDimensionMembersEnumeration(members));
			buffer.append("}");
			if (!useRules) buffer.append(")");
			buffer.append(" ON ");
			buffer.append(String.valueOf(axisCount));
			buffer.append(",");
			axisCount++;
			result.add(buffer.toString());
		}
		return result;
		
	}
	
	private synchronized QueryDefinition getNextSliceQuery() {
		QueryDefinition result = new QueryDefinition();
		result.slicerNames = new String[slicerMembers.keySet().size()]; 
		result.uniqueSlicerNames = new String[slicerMembers.keySet().size()]; 
		if (slicerMembers.keySet().size() > 0) {
			int i = 0;
			for (Dimension dimension : slicerMembers.keySet()) {
				Member m = slicerMembers.get(dimension).get(slicerPositions.get(dimension));
				result.slicerNames[i] = m.getName();
				result.uniqueSlicerNames[i] = m.getUniqueName();
				i++;
			}
		}
		StringBuilder buffer = new StringBuilder("SELECT ");
		for (String axisDef : baseSetAxisQuery) {
			buffer.append(axisDef);
		}
		/*
		for (String axisDef : getBaseSetAxisQuery()) {
			buffer.append(axisDef);
		}
		*/
		buffer.deleteCharAt(buffer.length()-1);
		buffer.append(" FROM ["+cubewrapper.getName()+"]");
		
		if (slicerMembers.keySet().size() > 0) {
			buffer.append(" WHERE (");
			int i = 0;
			for (Dimension dimension : slicerMembers.keySet()) {
				Member m = slicerMembers.get(dimension).get(slicerPositions.get(dimension));
				buffer.append(m.getUniqueName()+",");
				i++;
			}
			buffer.deleteCharAt(buffer.length()-1);
			buffer.append(")");
		}
		
		//iterate positions
		for (Dimension dimension : slicerPositions.keySet()) {
			Integer pos = slicerPositions.get(dimension);
			if (pos < slicerMembers.get(dimension).size() - 1) { //increment position count on this dimension if possible.
				slicerPositions.put(dimension,++pos);
				break;
			}
			else {
				slicerPositions.put(dimension,Integer.valueOf(0)); //dimension is iterated. reset it to zero and iterate next dimension
			}
		}
		int sum = 0;
		for (Dimension dimension : slicerPositions.keySet()) { //if all positions are reset to zero again, we are finished.
			sum += slicerPositions.get(dimension);
		}
		result.hasMoreData = sum > 0;
		result.query = buffer.toString();
		return result;
	}
	

	@Override
	public ICell next() {
		//NOTE: This function delivers the actual cell and does not actually fetch the next cell. 
		return cell;
	}

	@Override
	public boolean hasNext() {
		//NOTE: This function also fetches the next cell. So make sure to call it only once
		if (cellSet == null) return cleanup();
		try {
			Object value = null;
			Cell olapCell = null;
			do {
				olapCell = cellSet.getCell(ordinal++);
				value = olapCell.getValue();
			}
			while (skipEmpty && (value == null || value.equals(""))); //final check for nonEmpty since MDX NON EMPTY works on tuples, not on cells and will return empty cells for non empty tuples.
			cell = new CellWrapper(value);
			cell.setPathNames(getOrderedPath(olapCell));
			return true;
		}
		catch (Exception e) {
			if (fetcher != null) {
				synchronized(fetcher) {
					while (fetcher.isAlive()) {
						try {
							fetcher.wait();
						} catch (InterruptedException ie) {}
					}
					try {
						cellSet.close();
					} catch (SQLException e1) {}
					slicernames = fetcher.getNextSlicerNames();
				    cellSet = fetcher.getNextCellSet();	
				    ordinal = 0;
				}
				if (fetcher.hasMoreData()) {
					fetcher = new Fetcher();
					fetcher.start();
				} else fetcher = null;
				return hasNext();
			}
			return cleanup();
		}
	}
	
	private boolean cleanup() {
		cellSetMembers.clear();
		slicerMembers.clear();
		slicerPositions.clear();
		if (baseSetAxisQuery != null)
			baseSetAxisQuery.clear();
		if (cellSet != null) { 
			try {
				cellSet.close();
			} catch (SQLException e) {}
			cellSet = null;
		}
		return false;
	}

	
	private List<Integer> getPathDimensionOrder() {
		if (pathOrder == null) {
			List<Integer> result = new ArrayList<Integer>();
			List<Dimension> dims = new ArrayList<Dimension>();
			for (Dimension d : cubewrapper.getCube().getDimensions()) {
				if (cellSetMembers.keySet().contains(d) || slicerMembers.keySet().contains(d)) {
					dims.add(d);
				}
			}
			Iterator<Dimension> iterator = cellSetMembers.keySet().iterator();
			while (iterator.hasNext()) {
				result.add(dims.indexOf(iterator.next()));
			}
			iterator = slicerMembers.keySet().iterator();
			while (iterator.hasNext()) {
				result.add(dims.indexOf(iterator.next()));
			}
			pathOrder = result;
		}
		return pathOrder;
	}
	
	private String[] getOrderedPath(Cell cell) {
		List<Integer> pathOrder = getPathDimensionOrder();
		List<Integer> coordinates = cell.getCoordinateList();
	    List<CellSetAxis> axes = cell.getCellSet().getAxes();
	    //String[] result = new String[coordinates.size()];
	    String[] result = new String[coordinates.size()+((slicernames == null) ? 0 : slicernames.length)];
	    for (int i=0; i<coordinates.size(); i++) {
	    	Integer coordinate = coordinates.get(i);
	    	result[pathOrder.get(i)] = axes.get(i).getPositions().get(coordinate).getMembers().get(0).getName();
	    }
	    
	    if (slicernames != null) for (int i=0; i<slicernames.length; i++) {
	    	result[pathOrder.get(coordinates.size()+i)] = slicernames[i];
	    }
	    
	    for (String s : result) {
	    	if (s == null) {
	    		log.warn("Path element is null.");
	    	}
	    }
		return result;
	}

	@Override
	public IDimension[] getDimensions() {
		List<IDimension> result = new ArrayList<IDimension>();
		for (Dimension d : cubewrapper.getCube().getDimensions()) {
			if (cellSetMembers.keySet().contains(d) || slicerMembers.keySet().contains(d)) {
				result.add(cubewrapper.getDimensionByName(d.getName()));
			}
		}
		return result.toArray(new IDimension[result.size()]);
	}
	
	}
