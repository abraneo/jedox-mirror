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
 *   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
 */
package com.jedox.etl.components.prototype;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.TreeSet;

import com.jedox.etl.components.prototype.FileToOlapModel.ColumnRole;
import com.jedox.etl.components.prototype.FileToOlapModel.Coordinate;
import com.jedox.etl.core.component.ConfigurationException;

public class Dimension {

	private class Level {
		Level (String nameref) {
			this.nameref=nameref;
		}
		Level () {}
		
		public String nameref;
		public List<Coordinate> attributes = new ArrayList<Coordinate>();
		public Integer concatenatedFromLevel;
		public Integer columnId;
	}


	private SortedMap<Integer,Level> levelMap = new TreeMap<Integer,Level>();		
	private ColumnRole role = ColumnRole.DIM;

	public void putLevel(Integer index, String nameref, Integer columnId) {
		Level l = levelMap.get(index);
		if (l==null){
			l = new Level(nameref);
			levelMap.put(index, l);
		}else
			l.nameref=nameref;
		
		l.columnId = columnId;
		// TODO discuss whether we should keep this logic, adding a level with the same index should be an error
	}

	public void addAttribute (Integer index, Coordinate coord){
		Level l = levelMap.get(index);
		if (l==null) {
			l=new Level();
			levelMap.put(index, l);
		}
		l.attributes.add(coord);
	}

	public List<String> getLevelNamerefs() {
		List<String> resultList = new ArrayList<String>();
		for (Level l : levelMap.values()) {
			resultList.add(l.nameref);
		}
		return resultList;
	}
	
	public List<Integer> getAllColumnIds() {
		List<Integer> resultList = new ArrayList<Integer>();
		for (Level l : levelMap.values()) {
			resultList.add(l.columnId);
			for(Coordinate c:l.attributes)
				if(c.columnId>0)
					resultList.add(c.columnId);
		}
		return resultList;
	}

	
	public String getBaseLevel() {
		return levelMap.get(levelMap.lastKey()).nameref;
	}	
	
	public int getBaseLevelColumnId() {
		return levelMap.get(levelMap.lastKey()).columnId;
	}

	public void addGenericAttribute() {
		if (hasConcatenatedLevel()) {
			String attributeName="Name";
			if (getAttributesNames().contains(attributeName)) {
				attributeName="#Name";
			}
			for (Integer index : levelMap.keySet()) {
				Coordinate coord = new Coordinate(attributeName,levelMap.get(index).nameref, -1);
				addAttribute(index,coord);
			}	
		}
	}	
		
		/*			
			if (!concatenatedFromLevel.isEmpty()) {
				String attributeName="Name";
				if (getAttributesNames().contains(attributeName)) {
					attributeName="#Name";
				}
				for (Integer index : levelMap.keySet()) {
					Coordinate coord = new Coordinate(attributeName,levelMap.get(index));
					addAttribute(index,coord);
				}
			}
		 */			


	public List<List<Coordinate>> getAttributesForLevel() {
		List<List<Coordinate>> resultList = new ArrayList<List<Coordinate>>();
		for (Level l : levelMap.values()) {
			resultList.add(l.attributes);
		}
		return resultList;
	}

	public Set<String> getAttributesNames() {
		Set<String> attributeNames = new TreeSet<String>();
		for (Level l : levelMap.values()) {
			for (Coordinate coord : l.attributes)
				attributeNames.add(coord.name);
		}
		return attributeNames;
	}

	public void setRole(ColumnRole role) {
		this.role=role;
	}

	public ColumnRole getRole() {
		return role;
	}

	public void setConcatenate(Integer index, Integer level) {
		levelMap.get(index).concatenatedFromLevel=level;
	}

	public boolean hasConcatenatedLevel() {
		for (Level l : levelMap.values()) {
			if (l.concatenatedFromLevel!=null)
				return true;
		}
		return false;		
	}

	
	public List<String> getConcatsForLevel(String nameref) throws ConfigurationException {
		List<String> resultList = new ArrayList<String>();
		Level level=null;
		Integer index=null;
		for (Integer i : levelMap.keySet()) {			
			if (levelMap.get(i).nameref.equals(nameref)) {
				level=levelMap.get(i);
				index=i;
				break;
			}			
		}
		if (level.concatenatedFromLevel==null)
			return resultList;
		
		for (Integer i : levelMap.keySet()) {
			if (level.concatenatedFromLevel<=i && i<=index) {
				resultList.add(levelMap.get(i).nameref);
			}						
		}
		if (resultList.size()<2)
			throw new ConfigurationException("Invalid concatenation level "+level.concatenatedFromLevel+" for column "+nameref);
		return resultList;			
	}
	
	public void checkConsistency() throws ConfigurationException {
		for (Level l : levelMap.values()) {
			if (l.nameref==null)
				if (l.attributes.isEmpty())
					throw new ConfigurationException("Dimension without name (should never happen)");
				else
					throw new ConfigurationException("Attribute column "+l.attributes.get(0).nameref+" has no corresponding level column.");						
		}
		
	}

}


