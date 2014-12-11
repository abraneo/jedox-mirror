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
import com.jedox.etl.components.config.extract.DimensionExtractConfigurator;
import com.jedox.etl.components.config.extract.DimensionFilterDefinition;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.extract.IExtract;

import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.node.treecompat.PaloAttribute;

import com.jedox.etl.core.source.TreeSource;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;

/**
 *
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class DimensionExtract extends TreeSource implements IExtract {

	private IDimension dimension;
	private String dimensionName;
	private DimensionFilter filter;
	private DimensionFilterDefinition definition;
	private boolean withAttributes;

	private static final Log log = LogFactory.getLog(DimensionExtract.class);

	public DimensionExtract() {
		setConfigurator(new DimensionExtractConfigurator());	
	}

	public DimensionExtractConfigurator getConfigurator() {
		return (DimensionExtractConfigurator)super.getConfigurator();
	}

	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		throw new RuntimeException("OLAP connection is needed for extract "+getName()+".");
	}
	
	protected IDimension getDimensionObj() throws RuntimeException{

		com.jedox.palojlib.interfaces.IConnection con = (com.jedox.palojlib.interfaces.IConnection) getConnection().open();
		IDatabase d = con.getDatabaseByName(getConnection().getDatabase());
		if (d == null) {
			throw new RuntimeException("Database "+getConnection().getDatabase()+" not found. Failed to read Dimension "+getName());
		}

		dimension = d.getDimensionByName(dimensionName);
		if (dimension == null) {
			throw new RuntimeException("Dimension "+dimensionName+" not found in database "+getConnection().getDatabase()+".");
		}
		return dimension;
	}

	protected ITreeManager buildTree() throws RuntimeException {
		log.info("Data retrieval from Extract "+getName());		
		IDimension dimension = getDimensionObj();
		//getTreeManager().setName(dimension.getName());
		//filter = new DimensionFilter_exp(dimension);
		//filter.configure(definition);
		filter = new DimensionFilter(dimension,definition,withAttributes);
		if(!filter.isEmpty())
			filter.configure();
		ITreeManager manager = new TreeManagerNG(dimension.getName());
		if (withAttributes) manager.addAttributes(dimension.getAttributes(), true);
		manager.addElements(filter.getElements(filter.getAccepted().toArray(new String[filter.getAccepted().size()])), true);
		//manager.retainElements(filter.getAccepted().toArray(new String[filter.getAccepted().size()]));
		setTreeManager(manager);
		return manager;
	}

	public void setDimensionName(String dimensionName) {
		this.dimensionName = dimensionName;
	}

	public void setWithAttributes(boolean withAttributes) {
		this.withAttributes = withAttributes;
	}

	public Row getAttributes() throws RuntimeException {

		Row result = new Row();
		IDimension dimension = getDimensionObj();
		
		if(withAttributes) for (IAttribute a : dimension.getAttributes()) {
			result.addColumn(new PaloAttribute(a));
		}
		
		return result;

	}

	public void init() throws InitializationException {
		try {
			super.init();
			definition = getConfigurator().getFilterDefinition_exp();
			setDimensionName(getConfigurator().getDimensionName());
			setWithAttributes(getConfigurator().getWithAttributes());
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
}
