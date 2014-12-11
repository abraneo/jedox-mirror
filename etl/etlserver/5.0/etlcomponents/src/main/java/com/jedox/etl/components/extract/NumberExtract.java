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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.extract;

import com.jedox.etl.components.config.extract.NumberExtractConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.Processor;

public class NumberExtract extends TableSource implements IExtract {

	private int start;
	private int end;
	private int step;
	private String alias;

	private class NumberProcessor extends Processor {

		private Row row;
		private int maxCount;// maximum count of lines
		private int currentCount;
		private int currentValue;

		/**
		 * Processor which iterates over numbers
		 * @throws RuntimeException
		 */
		public NumberProcessor(int size) throws RuntimeException {
			try {
				row=new Row();
				row.addColumn(new Column(alias));
				maxCount = (size==0 ? Integer.MAX_VALUE : size);
				currentCount = 1;
				currentValue = start;			
			}
			catch (Exception e) {
				throw new RuntimeException("Failed to initialize the number processor " +getName()+": "+e.getMessage());
			}
		}

		/**
		 * fills in the next value into a row, returns true if next value exists 
		 */
		protected boolean fillRow(Row row) throws Exception {
			if(currentCount> maxCount || currentValue>end){
				return false;
			}
			row.getColumn(0).setValue(currentValue);			
			currentCount++;
			currentValue+=step;
			return true;
		}

		/**
		 * get the current row
		 */
		protected Row getRow() {
			return row;
		}
	}

	public NumberExtract() {
		setConfigurator(new NumberExtractConfigurator());
	}

	public NumberExtractConfigurator getConfigurator() {
		return (NumberExtractConfigurator)super.getConfigurator();
	}	
	
	public void init() throws InitializationException {
		try {
			super.init();
			start = getConfigurator().getStart();
			end = getConfigurator().getEnd();
			step = getConfigurator().getStep();
			alias = getConfigurator().getAlias();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

	@Override
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		NumberProcessor processor = new NumberProcessor(size);
		return processor;
	}
}
