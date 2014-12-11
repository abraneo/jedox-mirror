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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.extract;


//import org.palo.api.Connection;
//import org.palo.api.Database;
//import org.palo.api.Dimension;
//import org.palo.api.subsets.Subset2;
//import org.palo.api.subsets.SubsetHandler;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.components.config.extract.SubsetExtractConfigurator;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.extract.IExtract;
// import com.jedox.etl.components.connection.OLAPLegacyConnection;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;

/**
 *
 *	@author Kais Haddadin
 *
 */
public abstract class SubsetLegacyExtract extends TableSource implements IExtract  {

/*
	private class SubsetProcessor extends Processor {

		// These rules are the rules stored in the cube only
		private Row row;
		private int count;// the count of the rules

		*//**
		 * constructor that rules from the cube and set the columns names
		 * @throws RuntimeException
		 *//*
		public SubsetProcessor() throws RuntimeException {
			try {
				row = PersistenceUtil.getColumnDefinition(getAliasMap(),getColumnsAsString(getColumns()));
				count = 0;
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to get subset from dimension "+dimName+": "+e.getMessage());
			}
		}

		*//**
		 * read the rules and fill them in a row
		 *//*
		protected boolean fillRow(Row row) throws Exception {

			if (subsetExport!= null &&  subsetExport.length > count){
				count++;
				Subset2 subset = subsetExport[count-1];
				row.getColumn(0).setValue(subset.getName());
				row.getColumn(1).setValue(subset.getDefinition());
				return true;

			}
			else{
				//finished ... do some cleanup
				count = 0;
				return false;
			}

		}

		*//**
		 * get row
		 *//*
		protected Row getRow() {
			return row;
		}
	}

	private String dimName;
	private Subset2[] subsetExport;

	*//**
	 * constructor
	 *//*
	public SubsetLegacyExtract() {
		setConfigurator(new SubsetExtractConfigurator());
	}

	*//**
	 * get the configurator
	 *//*
	public SubsetExtractConfigurator getConfigurator() {
		return (SubsetExtractConfigurator)super.getConfigurator();
	}

	*//**
	 * get columns as string
	 * @param coordinates the column names
	 * @return one string that contains all strings
	 *//*
	private String getColumnsAsString(String[] coordinates) {
		StringBuffer sb = new StringBuffer();
		for (int i=0; i<coordinates.length; i++) {
			sb.append(coordinates[i]+", ");
		}
		sb.deleteCharAt(sb.length()-1);
		return sb.toString();
	}

	*//**
	 * get columns names
	 * @return array of string that represent the columns names
	 *//*
	private String[] getColumns() {
		String [] columns = {"Name","Definition"};
		return columns;
	}

	*//**
	 * get the connection
	 *//*
	public OLAPLegacyConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof OLAPLegacyConnection))
			return (OLAPLegacyConnection) connection;
		throw new RuntimeException("OLAP connection is needed for this source: "+getName()+". Connection set is of type: "+ connection.toString());
	}


	*//**
	 * get the dimension subsets
	 * @return array of Subsets
	 * @throws RuntimeException
	 *//*
	private Subset2[] getDimensionSubsets() throws RuntimeException{

		Connection con = getConnection().open();
		String database = getConnection().getDatabase();

		Database d = con.getDatabaseByName(database);
		if (d == null) {
			throw new RuntimeException("Database "+database+" not found in connection "+getConnection().getName());
		}
		Dimension dim = d.getDimensionByName(dimName);
		if (dim == null) {
			throw new RuntimeException("Dimension "+ dimName+" not found in database "+database);
		}

		 SubsetHandler sh = dim.getSubsetHandler();
		return  sh.getSubsets();
	}

	*//**
	 * get a specific number of rows
	 *//*
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor result = new SubsetProcessor();
		result.setLastRow(size);
		return result;
	}

	*//**
	 * initialize the source
	 *//*
	public void init() throws InitializationException {
		try {
			super.init();
			dimName = getConfigurator().getDimName();
			if(dimName!= null)
				subsetExport =  getDimensionSubsets();

		}
		catch (Exception e) {
			invalidate();
			throw new InitializationException(e);
		}
	}
*/ 
}
