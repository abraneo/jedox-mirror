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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.aliases;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import java.util.TreeSet;

import com.jedox.etl.core.component.Component;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.alias.AliasConfigurator;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.Row;


/**
 * Helper Class, that maps column indices to names, which can be used to address a column.
 * Helpful, when several Datasource share a single mapping.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class AliasMap extends Component implements IAliasMap {
	
	private HashMap<String, AliasMapElement> map = new HashMap<String, AliasMapElement>();
	//private HashMap<String, String> aliases = new HashMap<String, String>();
	private static final Log log = LogFactory.getLog(AliasMap.class);
	
	public AliasMap() {
		setConfigurator(new AliasConfigurator());
	}
	
	public AliasConfigurator getConfigurator() {
		return (AliasConfigurator) super.getConfigurator();
	}
	
	public void map(AliasMapElement element) {
		String name = element.getName();
		AliasMapElement m = map.get(name);
		//do nothing if alias for this column is already present.
		if ((m != null) && (m.getColumn() == element.getColumn())) 
			return;
		//give an error, when the same alias denotes 2 different columns.
		if ((m != null) && (m.getColumn() != element.getColumn())) 
			log.error("Duplicate Alias for different Columns: "+element.getColumn());
		//give an error, when two different aliases denotes the same column.
		if (!getAlias(element.getColumn(),name).equals(name)) 
			log.error("Different Aliases reference the same column: "+element.getColumn());
		//add the new alias
		map.put(name, element);
	}
	
	public AliasMapElement getElement(String name) {
		AliasMapElement m = map.get(name);
		if (m == null) log.warn("Alias "+name+" not found.");
		return m;
	}
	
	public boolean hasAlias(String alias) {
		return map.keySet().contains(alias);
	}
	
	public int getCol(String alias) {
		AliasMapElement m = map.get(alias);
		int col = 0;
		if (m != null) {
			col = m.getColumn();
		}
		if (col == 0) log.warn("Alias "+alias+" not found.");
		return col;
	}
	
	public Set<String> getAliases() {
		return map.keySet();
	}
	
	public Set<String> getAliasesOrigin() {
		Set<String> origins = new HashSet<String>();
		for(String key:map.keySet()){
			origins.add(map.get(key).getOrigin());
		}
		return origins;
	}
	
	public int getAliasCount() {
		return map.values().size();
	}
	
/*	
	public boolean hasDefaults() {
		for (AliasMapElement elem : map.values()) {			
			if (elem.getDefaultValue()!=null)
				return true;
		}
		return false;
	}
*/
	
	/**
	 * gets a clone of this alias map
	 */
	public AliasMap clone() {
		AliasMap am = new AliasMap();
		for (AliasMapElement m : map.values()) {
			am.map(m.clone());
		}
		am.setName(getName());
		//am.aliases = aliases;
		am.setConfigurator(getConfigurator());
		return am;
	}
	
	private String getDefaultAlias(String defaultAlias, int column) {
		if (defaultAlias == null) return null;
		if (defaultAlias.equals("")) return "constant"+String.valueOf(column);
		return defaultAlias;
	}
	
	public String getAlias(int column, String defaultAlias) {
		for (String key : getAliases()) {
			AliasMapElement m = map.get(key);
			if (m.getColumn() == column) {
				//prefer external default instead of internal
				if (m.getInternalDefaultName().equals(m.getName())) {
					return getDefaultAlias(defaultAlias,column);
				}
				return key;
			}
		}
		return getDefaultAlias(defaultAlias, column);
	}

	public boolean hasAlias(int column) {
		for (String key : getAliases()) {
			AliasMapElement m = map.get(key);
			if (m.getColumn() == column) {
				return true;
			}
		}
		return false;
	}	
	
	/**
	 * joins this alias map with the alias map provided.
	 * @param joinMap the alias map to join with this one.
	 * @return the alias names of the new map ordered by their corresponding column numbers.
	 */
	public String[] join(AliasMap joinMap) {
		TreeSet<AliasMapElement> set = new TreeSet<AliasMapElement>();
		set.addAll(map.values());
		int offset = getAliasCount();
		for (String alias: joinMap.getAliases()) {
			AliasMapElement m = joinMap.getElement(alias).clone();
			if (map.get(m.getName()) == null) {
				m.setColumn(m.getColumn()+offset);
				set.add(m);
				map(m);
			}
		}
		Iterator<AliasMapElement> iterator = set.iterator();
		String[] names = new String[set.size()];
		for (int i=0; iterator.hasNext(); i++) {
			AliasMapElement m = iterator.next();
			names[i] = m.getName();
			m.setColumn(i+1);
			//System.err.println(m.getName() + " "+ m.getColumn() +" "+ m.getDefaultValue());
		}
		return names;
	}
	
	/**
	 * gets the alias names ordered by their corresponding column numbers.
	 * @return the alias names
	 */
	public String[] getNames() {
		TreeSet<AliasMapElement> set = new TreeSet<AliasMapElement>();
		set.addAll(map.values());
		Iterator<AliasMapElement> iterator = set.iterator();
		String[] names = new String[set.size()];
		for (int i=0; iterator.hasNext(); i++) {
			AliasMapElement m = iterator.next();
			names[i] = m.getName();
		}
		return names;	
	}
	
	public boolean hasDefaultValues() {
		for(String key:map.keySet()){
			if(map.get(key).getDefaultValue()!=null){
				return true;
			}
		}
		return false;
	}
	
	public void shift(int offset) {
		for (AliasMapElement m : map.values()) {
			m.setColumn(m.getColumn()+offset);
		}
	}
	
	public Row getOutputDescription() throws RuntimeException {
		Row row = new Row();
		for (String name : getNames()) {
			Column c = new Column(name);
			c.setDefaultValue(getElement(name).getDefaultValue());
			row.addColumn(c);
		}
		return row;
	}
	
	public void init() throws InitializationException {
		try {
			List<AliasMapElement> elements = getConfigurator().getElements();
			for (AliasMapElement e : elements)
				map(e);
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
		
	}

	@Override
	public boolean isEmpty() {
		return map.keySet().isEmpty();
	}
}