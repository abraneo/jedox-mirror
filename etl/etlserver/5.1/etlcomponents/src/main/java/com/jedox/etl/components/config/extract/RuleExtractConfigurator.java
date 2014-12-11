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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.extract;

import java.util.ArrayList;
import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.*;
import com.jedox.palojlib.interfaces.IRule;


public class RuleExtractConfigurator extends TableSourceConfigurator {

	private class Rule extends com.jedox.palojlib.main.Rule implements IRule {

		private Rule(String definition, String comment, boolean active){
			super(0, definition, "", comment, 0, active);
		}				
	}
	
	private String cubeName; // the name of the cube, optional used always with a connection
	private IRule[] staticRules; // unique names for the rules

	/**
	 * do one of these tasks:
	 * 1. read the cube name if the rules source is within a cube
	 * 2. read the static rules if they were given
	 * Note: can not have both!
	 * @throws ConfigurationException
	 */
	protected void setRules() throws ConfigurationException {
		Element ruleselem = getXML().getChild("rules");
		ArrayList<IRule> rules = new ArrayList<IRule>();			
		if (ruleselem != null) {
			for(int i=0;i<ruleselem.getChildren().size();i++){
				Element ruleDef = (Element)ruleselem.getChildren().get(i);
				IRule r = new Rule(ruleDef.getValue(),
								   ruleDef.getAttributeValue("comment"),
								   ruleDef.getAttributeValue("active").equalsIgnoreCase("true"));
				rules.add(r);
			}						
		}
		staticRules = rules.toArray(new Rule[rules.size()]);
	}

	/**
	 * get cube name
	 * @return cube name
	 */
	public String getCubeName() {
		return cubeName;
	}
	/**
	 * get the rules defined statically in the Rule Extract
	 * @return Rules
	 */
	public IRule[] getStaticRules() {
		return staticRules;
	}

	/**
	 * configure the source by configuring the super class, and setting the rules
	 */
	public void configure() throws ConfigurationException {
		super.configure();
		Element cube = getXML().getChild("cube");
		if(cube != null)
			cubeName = cube.getAttributeValue("name");		
		setRules();
	}

}
