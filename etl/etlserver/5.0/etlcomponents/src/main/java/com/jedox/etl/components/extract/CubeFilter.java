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
package com.jedox.etl.components.extract;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition.ConditionExtended;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.HashMap;
/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class CubeFilter {

	private HashMap<String, IDimension> dimensionLookup = new HashMap<String, IDimension>();
	private Vector<IDimension> dimensions = new Vector<IDimension>();
	private HashMap<String,DimensionFilter> filters = new HashMap<String,DimensionFilter>();
	private String cubeName;
	private ICube cube;
	private boolean isFiltered = true;
	private static final Log log = LogFactory.getLog(CubeFilter.class);

	public CubeFilter(ICube cube) {
		this.cube = cube;
		cubeName = cube.getName();
		IDimension[] dims = cube.getDimensions();
		for (IDimension dim : dims) {
			dimensionLookup.put(dim.getName(), dim);
			dimensions.add(dim);
		}
	}

	public ICube getCube() {
		return cube;
	}

	public IDimension[] getFilteredDimensions() {
		List<IDimension> filteredDims = new ArrayList<IDimension>(); 
		for (IDimension dim : dimensions) {
			if (filters.get(dim.getName()) != null) {
				filteredDims.add(dim);
			}
		}
		return filteredDims.toArray(new IDimension[filteredDims.size()]);
		
	}
	
	public IElement[][] getBasisElements () {
		IElement[][] basis = new IElement[dimensions.size()][];
		int index = 0;
		for (IDimension dim : dimensions) {
			DimensionFilter filter = filters.get(dim.getName());
			if (filter == null) {
				basis[index] = null;
			}
			else  {
				String [] coordinates = filter.process();
				/* Dimension is empty. No basis available, Resultset will be empty. */
				if (coordinates.length == 0) {
					log.warn("No elements apply to filter criteria for dimension "+filter.getName()+". Result set of cube "+cubeName+" will be empty.");
					return null;
				}
				String coordinatesString = "Filtered elements dimension " + filter.getName() + ": ";
				for(String s:coordinates)
					coordinatesString += s.concat(",");
				log.debug(coordinatesString);
				basis[index] = filter.getElements(coordinates);
			}
			index++;
		}
		return basis;
	}
	
	public DimensionFilter addDimensionFilter(String dimension,DimensionFilterDefinition dfd) throws RuntimeException {
		IDimension dim = dimensionLookup.get(dimension);
		if (dim == null) {
			throw new RuntimeException("Failed to find Dimension "+dimension+" in Cube "+cubeName+".");
		}
		boolean withAttributes = false;
		for (ConditionExtended c : dfd.getConditionsExtended()) {
			if (c.getSearchAttribute() != null) withAttributes = true;
		}
		DimensionFilter filter = new DimensionFilter(dim,dfd,withAttributes);
		filters.put(filter.getName(),filter);
		return filter;
	}
	
	public boolean isFiltered () {
		return isFiltered;
	}

	public void configure(List<DimensionFilterDefinition> definitions) throws RuntimeException {
		isFiltered = !definitions.isEmpty();
		for (DimensionFilterDefinition d : definitions) {
			DimensionFilter filter = addDimensionFilter(d.getName(),d);
			filter.configure();
		}
	}
}
