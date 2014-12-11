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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.transform;

import com.jedox.etl.components.config.transform.TableTransformConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;

public class FieldList extends TableSource implements ITransform {

	
	private class FieldListProcessor extends Processor {
		
		private Row sourceOutput;
		private Row resultRow;
		private int index=0;
		private String alias;
		
		public FieldListProcessor(Row sourceOutput, String alias) {
			this.sourceOutput = sourceOutput;
			this.alias = alias;
		}
		
		@Override
		protected boolean fillRow(Row row) throws Exception {
			if (index >= sourceOutput.size())
				return false;
			else {
				resultRow.getColumn(alias).setValue(sourceOutput.getColumn(index).getName());
				index++;
				return true;
			}	
		}

		@Override
		protected Row getRow() throws RuntimeException {
			return resultRow;
		}

		@Override
		protected void init() throws RuntimeException {
			resultRow = new Row();
			resultRow.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(alias, null));
		}
		
	}

    private String alias;

	public FieldList() {
		setConfigurator(new TableTransformConfigurator());
	}


	public TableTransformConfigurator getConfigurator() {
		return (TableTransformConfigurator)super.getConfigurator();
	}
	
	
	protected SourceManager getSourceManager() {
		return (SourceManager)getManager(ITypes.Sources);
	}
	
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		Row sourceOutput = getSourceManager().getFirst().getOutputDescription();
		return initProcessor(new FieldListProcessor(sourceOutput, alias),Facets.OUTPUT);
	}	
	
	
	public void init() throws InitializationException {
		super.init();
		try {
			SourceManager manager = new SourceManager();
			manager.addAll(getConfigurator().getSources());
			addManager(manager);
			alias = getParameter("alias","Column");
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}	
	}

}
