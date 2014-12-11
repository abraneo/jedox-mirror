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
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.config;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Attribute;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.Text;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.ITypes.Managers;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.context.IContext;
import com.jedox.etl.core.scriptapi.APIExtender;
import com.jedox.etl.core.scriptapi.APIExtender.ScriptDependenciesDescriptor;
import com.jedox.etl.core.scriptapi.APIExtender.ScriptDependenciesResult;

import org.jdom.xpath.XPath;

/**
 * @author Kais Haddadin. Mail: kais.haddadin@jedox.com
 *
 */
public class UpdateReferencesConfigUtil {
	
	private final static String nodeValue = "_#_#_NODE_VALUE";
	private final String[] xpaths = new String[]{
			"connection/@nameref[.='" + nodeValue+ "']",
			"source/@nameref[.='" + nodeValue+ "']",
			"execution/@nameref[.='" + nodeValue+ "' and ../@type='load']",
			"sources/source/@nameref[.='" + nodeValue+ "']",
			"functions/function/parameters/source[.='" + nodeValue+ "']",
			"functions/function/parameters/connection[.='" + nodeValue+ "']",
			"loops/loop/@nameref[.='" + nodeValue+ "']",
			"joins/join/*/@nameref[.='" + nodeValue+ "']",
			"functions/function/parameters/script/text()",
			"jobscript/text()",
			"execution/@nameref[.='" + nodeValue+ "' and ../@type='job']",
			"variable/@name[.='" + nodeValue+ "']",
			"functions/function/inputs/input/@nameref[.='" + nodeValue+ "']",
			"target/coordinates/coordinate/input/@nameref[.='" + nodeValue+ "']",
			/*"target/measures/measure/input/@nameref[.='" + nodeValue+ "']",
			"cube/dimensions/dimension/@input[.='" + nodeValue+ "']",
			"cube/drillthrough/annex/@input[.='" + nodeValue+ "']",
			"table/column/@nameref[.='" + nodeValue+ "']",
			"coordinates/coordinate/input/@nameref[.='" + nodeValue+ "']",*/
			//"jobscript[contains(.,'" + nodeValue+ "')]" 
			"xslt/connection/@nameref[.='" + nodeValue+ "']"
		};
	//private XPath[] xpathExpressions = null;
	private Map<String,Integer[]> extractsXPath = new HashMap<String, Integer[]>();
	private Map<String,Integer[]> transformsXPath = new HashMap<String, Integer[]>();
	private Map<String,Integer[]> loadsXPath = new HashMap<String, Integer[]>();
	private Map<String,Integer[]> jobsXPath = new HashMap<String, Integer[]>();
	private static UpdateReferencesConfigUtil util;
	private static final Log log = LogFactory.getLog(UpdateReferencesConfigUtil.class);
	
	private UpdateReferencesConfigUtil() throws JDOMException{
		
		/*xpathExpressions = new XPath[xpaths.length];
		for(int i=0;i<xpaths.length;i++){
			xpathExpressions[i] = XPath.newInstance(xpaths[i]);
		}*/
		
		//A extract need to be changed. here are the needed xpaths.
		extractsXPath.put("connections", new Integer[]{0,14});
		
		//A transform need to be changed. here are the needed xpaths.
		transformsXPath.put("connections", new Integer[]{5,8});
		transformsXPath.put("extracts", new Integer[]{3,4,6,7,8});
		transformsXPath.put("transforms", new Integer[]{3,4,6,7,8});
		transformsXPath.put("functions", new Integer[]{12,13});
		
		//A load need to be changed. here are the needed xpaths.
		loadsXPath.put("connections", new Integer[]{0,14});
		loadsXPath.put("extracts", new Integer[]{1});
		loadsXPath.put("transforms", new Integer[]{1});
		loadsXPath.put("variables", new Integer[]{11});
		//loadsXPath.put("functions", new Integer[]{15,16,17,18});
		
		//A job need to be changed. here are the needed xpaths.
		jobsXPath.put("loads",new Integer[]{2,9});
		jobsXPath.put("jobs",new Integer[]{9,10});
		jobsXPath.put("variables",new Integer[]{11});
		
	}
	
	public static UpdateReferencesConfigUtil getInstance(){
		if(util==null){
			try {
				util = new UpdateReferencesConfigUtil();
			} catch (JDOMException e) {
				log.error("Error in the given xpaths in this class, this should never happen!");
			}
		}
		return util;
	}
	
	public void renameReference(Element e, Locator referenceLoc,Locator dependantLoc, String newName){
			Managers type = ITypes.Managers.valueOf(e.getName()+"s");
			try {
			switch (type) {
			case extracts: updateReferences(e,referenceLoc,dependantLoc, newName,extractsXPath);break;
			case transforms:updateReferences(e,referenceLoc,dependantLoc, newName,transformsXPath);break;
			case loads: updateReferences(e,referenceLoc,dependantLoc, newName,loadsXPath);break;
			case jobs: updateReferences(e,referenceLoc,dependantLoc, newName,jobsXPath);break;
			case functions: break;
			default:break;
			}	
			
			} catch (Exception e1) {
				log.error("Error while evaluating the xpath expression " + e1.getMessage());
			}
	}

	/**
	 * @param e
	 * @param newName
	 * @param index
	 * @throws JDOMException
	 */
	private void rename(Element e, Locator referenceLoc, Locator dependantLoc, String newName, Integer index)
			throws JDOMException {
		List<?> nodes = XPath.selectNodes(e,xpaths[index].replace(nodeValue, referenceLoc.getName()));
		for(Object n:nodes){
			if(n!=null){
				if(n instanceof org.jdom.Attribute){
					Attribute att = (Attribute) n;
					att.setValue(newName);
					
				}else if(n instanceof Element){
					Element element = (Element) n;
					element.setText(newName);
				} else if (n instanceof Text) {
					Text script = (Text)n;
					//map to function in transform if transform
					//TODO this is sensitive to errors on structure change. think about including functions directly in depencencies / dependants
					if (dependantLoc.getManager().equalsIgnoreCase(ITypes.Managers.transforms.toString())) {
						dependantLoc = dependantLoc.clone().add(ITypes.Managers.functions.toString()).add(script.getParentElement().getParentElement().getParentElement().getAttributeValue("name"));
					}
					String updatedScript = updateScript(referenceLoc,dependantLoc, newName,script.getText());
					script.setText(updatedScript);
				}
			}
		}
	}
	
	private void updateReferences(Element e, Locator referenceLoc, Locator dependantLoc, String newName, Map<String,Integer[]> xpathMap) throws JDOMException {
		Integer[] xpathsIndices = xpathMap.get(referenceLoc.getManager());
		if(xpathsIndices!=null){
			for(Integer index:xpathsIndices){
				rename(e, referenceLoc , dependantLoc, newName, index);
			}
		}
	}
	
	
	private String buildParameterString(String[] parameters) {
		StringBuffer result = new StringBuffer();
		if (parameters != null) {
			for (int i=0; i<parameters.length-1; i++) {
				result.append(parameters[i]+",");
			}
			result.append(parameters[parameters.length-1]);
		}
		return result.toString();
	}
	
	private String updateScript(Locator referenceLocator, Locator dependantLocator, String newName, String script) {
		try {
			IComponent component = ConfigManager.getInstance().getComponent(dependantLocator, IContext.defaultName);
			ScriptDependenciesResult result = APIExtender.getInstance().getGuessedDependencies(component, Arrays.asList(new String[]{script}));
			for (ScriptDependenciesDescriptor d : result.descriptors) {
				if (d.component.getLocator().toString().equalsIgnoreCase(referenceLocator.toString())) {
					String origString = d.methodString+buildParameterString(d.parameters)+")";
					d.parameters[d.scan.position] = "\""+newName+"\"";
					String replaceString = d.methodString+buildParameterString(d.parameters)+")";
					script = script.replace(origString, replaceString);
				}
			}
		}
		catch (Exception e) {
			log.error("Error renaming dependency in script: "+e.getMessage());
		}
		return script;
	}
}
