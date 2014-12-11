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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.job;

import java.util.ArrayList;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.job.JobConfigurator;
import com.jedox.etl.core.config.source.FilterConfigurator;
import com.jedox.etl.core.execution.IExecutable;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.SourceFactory;
import com.jedox.etl.core.source.ViewSource;
import com.jedox.etl.core.source.filter.IEvaluator;

import org.jdom.Element;
/**
 * 
 * @author kais.haddadin@jedox.com
 *
 */
public class SwitchJobConfigurator extends JobConfigurator {

	private ISource conditionSource;
	private IExecutable conditionExecutable;
	private boolean isConditionSource;
	private String sourceInput;
	private int sourceRow = 1;
	private ArrayList<SwitchClause> switches = new ArrayList<SwitchClause>();
	private FilterConfigurator filterConfig = new FilterConfigurator();
	private IExecutable defaultExecutable;
	
    public class SwitchClause{
        private IEvaluator evaluator;
        private IExecutable resultExecutable;
        
        public SwitchClause(Element condition,Element executableRef) throws ConfigurationException{
        	evaluator = filterConfig.getEvaluator(condition);
        	resultExecutable = (IExecutable)getContext().getComponent(getExecutableLocator(executableRef));
        }
        
        public IEvaluator getEvaluator(){
        	return evaluator;
        }
        
        public IExecutable getResultExecutable(){
        	return resultExecutable;
        }
    }
	
	public String getName() {
		return "switch"; 
	}
		
	public void configure() throws ConfigurationException {
		try {
			setCondition();
			setSwitchs();
			setDefaultLocator();
		} catch (CreationException e) {
			throw new ConfigurationException(e.getMessage());
		}
	}
	
	public IExecutable getDefaultExecutable(){
		return defaultExecutable;
	}

	private void setDefaultLocator() throws ConfigurationException {
		Element defaultElement = getXML().getChild("default");
		Element executableRef = defaultElement.getChild("load");
		if(executableRef==null)
			executableRef = defaultElement.getChild("job");
		if(executableRef!=null){
			defaultExecutable = (IExecutable)getContext().getComponent(getExecutableLocator(executableRef));;
		}	
	}

	private void setSwitchs() throws ConfigurationException {
		Element switchsElement = getXML().getChild("switchs");
		for(Object switchObj:switchsElement.getChildren("switch")){
			Element switchElement = (Element) switchObj;
			Element executableRef = switchElement.getChild("load");
			if(executableRef==null)
				executableRef = switchElement.getChild("job");
			Element condition = switchElement.getChild("condition");
			switches.add(new SwitchClause(condition, executableRef));
		}
		
	}

	private void setCondition() throws ConfigurationException, CreationException {
		Element sourceElement = getXML().getChild("source");
		if(sourceElement!=null){
			isConditionSource = true;
			Element modifiedSourceElement = (Element) sourceElement.clone();
			modifiedSourceElement.removeContent();
			conditionSource = SourceFactory.getInstance().newSource(ViewSource.getViewDescriptor(), this, getContext(), modifiedSourceElement);
			Element inputElement = sourceElement.getChild("input");
			sourceInput = inputElement.getAttributeValue("nameref");
			Element rowElement = sourceElement.getChild("row");
			if(rowElement!=null)
				sourceRow = Integer.parseInt(rowElement.getValue());
		}else{
			isConditionSource = false;
			Element executableRef = getXML().getChild("load");
			if(executableRef==null)
				executableRef = getXML().getChild("job");
			conditionExecutable = (IExecutable)getContext().getComponent(getExecutableLocator(executableRef));
		}
		
	}
	
	private Locator getExecutableLocator(Element executableRef){
		ITypes.Managers manager = ITypes.Managers.valueOf(executableRef.getName()+"s");
		return getLocator().getRootLocator().add(manager.toString()).add(executableRef.getAttributeValue("nameref"));
		
	}
	
	public ArrayList<SwitchClause> getSwitches() {
		return switches;
	}

	public ISource getConditionSource() {
		return conditionSource;
	}
	
	public IExecutable getConditionExecutable() {
		return conditionExecutable;
	}

	/**
	 * @return the conditionSource
	 */
	public boolean isConditionSource() {
		return isConditionSource;
	}

	/**
	 * @return the sourceInput
	 */
	public String getSourceInput() {
		return sourceInput;
	}

	/**
	 * @return the sourceRow
	 */
	public int getSourceRow() {
		return sourceRow;
	}

}
