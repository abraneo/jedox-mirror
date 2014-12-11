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
@author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.config.source;

import java.util.List;
import java.util.Properties;


import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.BasicConfigurator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.source.filter.AlphaRangeEvaluator;
import com.jedox.etl.core.source.filter.Conditions;
import com.jedox.etl.core.source.filter.EmptyEvaluator;
import com.jedox.etl.core.source.filter.EqualityEvaluator;
import com.jedox.etl.core.source.filter.IEvaluator;
import com.jedox.etl.core.source.filter.RangeEvaluator;
import com.jedox.etl.core.source.filter.RegexEvaluator;
import com.jedox.etl.core.source.filter.RowFilter;
import com.jedox.etl.core.source.filter.ScriptEvaluator;
import com.jedox.etl.core.source.filter.Conditions.Modes;
import com.jedox.etl.core.source.filter.RowFilter.Operators;

/**
 * Filter Configurator class used for filtered Sources
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class FilterConfigurator extends BasicConfigurator {
	
	private IContext context;

	public FilterConfigurator(IContext context, Properties parameters) {
		this.context = context;
		addParameter(context.getParameter());
		addParameter(parameters);
	}
	
	public FilterConfigurator() {
	}

	/**
	 * gets the configured filter object from XML
	 * @param filter
	 * @return the filter object
	 * @throws ConfigurationException
	 */
	public RowFilter getFilter(Element filter) throws ConfigurationException {
		RowFilter sourcefilter = null;
		if (filter != null) {
			sourcefilter = new RowFilter(filter.getAttributeValue("type","AND"));
			for (Element input : getChildren(filter,"input")) {
				String alias = input.getAttributeValue("nameref");
				List<?> ops = input.getChildren();
				if( ((Element) ops.get(0)).getAttributeValue("type").equals("deny")){
					sourcefilter.getConditions().addCondition("accept",new RegexEvaluator("."),alias);
					sourcefilter.getConditions().addCondition("accept",new RegexEvaluator("^$"),alias);
				}
				for (int i=0; i<ops.size(); i++) {
					Element op = (Element) ops.get(i);
					IEvaluator evaluator = getEvaluator(op);
					sourcefilter.getConditions().addCondition(op.getAttributeValue("type"), evaluator, alias);
				}
			}
			for (Element script : getChildren(filter,"script")) {
				String engineName = script.getAttributeValue("type", "groovy");
				sourcefilter.getConditions().addCondition(Modes.ACCEPT.toString(), new ScriptEvaluator(context,script.getTextTrim(),engineName), Conditions.scriptKey);
			}
		}
		return sourcefilter;
	}

	/**
	 * @param op
	 * @return
	 * @throws ConfigurationException
	 */
	public IEvaluator getEvaluator(Element op) throws ConfigurationException {
		Operators operator = Operators.valueOf(op.getAttributeValue("operator"));
		String value = op.getAttributeValue("value");
		
		IEvaluator evaluator = null;
		switch(operator) {
			case equal:
				evaluator = new EqualityEvaluator(value);
				break;
			case like:
				evaluator = new RegexEvaluator(value);
				break;
			case inRange:
				evaluator = new RangeEvaluator(value);
				break;
			case inAlphaRange:
				evaluator = new AlphaRangeEvaluator(value);
				break;								
			case between:    //obsolete
				evaluator = new RangeEvaluator(value);
				break;
			case alpha_range:   //obsolete
				evaluator = new AlphaRangeEvaluator(value);
				break;
			case isEmpty:
				evaluator = new EmptyEvaluator(value);								
				break;
			default:
				throw new ConfigurationException("Filter operator " + operator + " is not defined.");
		}
		return evaluator;
	}

}
