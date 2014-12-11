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

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Properties;
import java.util.Set;
import java.util.regex.Matcher;







//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.util.XMLUtil;

/**
 * @author Kais Haddadin. Mail: kais.haddadin@jedox.com
 *
 */
public abstract class VariablesDependencyHandler extends ConfigResolver {
	
	protected ArrayList<Locator> referenceLocators;	
	protected Element lastComponent = null;
	protected Element project = null;
	protected String session = null;
	protected ConfigManager cm;
	protected HashMap<String,String> newNames = new HashMap<String, String>();
	private Set<String> elementToBeChanged = new HashSet<String>();
	
	public VariablesDependencyHandler(ArrayList<Locator> referenceLocators) {
		super();
		this.referenceLocators=referenceLocators;
		for(Locator referenceLocator:referenceLocators){
			newNames.put(referenceLocator.getName(), "");
		}
	}
	
	
	protected String resolve(String text, Properties properties) throws CreationException {
		if (text != null) {
			Matcher m = variable.matcher(text);
			while (m.find()) {
				String group = m.group();
				String key = group.substring(2,group.length()-1);
				for(Locator oldName:referenceLocators){
					if (key.equals(oldName.getName())){
						text = text.replace(group, "${"+newNames.get(oldName.getName())+"}");
						elementToBeChanged.add(oldName.getName());
					}
				}
			}
		}
		return text;
	}
	

	protected void resolveElement(Element element, Properties properties) throws CreationException {
		if(!element.getName().equals("project") && ITypes.Components.contains(element.getName())&& ITypes.MainManagers.contains(element.getParentElement().getName()))
				lastComponent = element;
		
		super.resolveElement(element, properties);

			if(lastComponent!=null){
				for(String reference:elementToBeChanged){
					try {				
						 elementFound(Locator.parse(reference));
					} catch (ConfigurationException e) {
					}
				}
				elementToBeChanged.clear();
			}
				//lastComponent = null;
		}
		
	protected abstract void elementFound(Locator reference) throws ConfigurationException;
	
	protected void handleVariables(ConfigManager cm) throws CreationException, ConfigurationException {
		if(this.project==null){
			this.project = cm.findElement(referenceLocators.get(0).getRootLocator());
			this.session = referenceLocators.get(0).getSessioncontext();
		}
		
		Locator[] jobLocs = cm.getLocators(referenceLocators.get(0).getRootLocator().add(ITypes.Jobs));
		
		/* handle static variables in Jobs*/
		for (Locator depLoc : jobLocs) {
			for(Locator refLoc:referenceLocators){
				lastComponent = cm.findElement(depLoc);
				Element componentOld = (Element) lastComponent.clone();
				UpdateReferencesConfigUtil.getInstance().renameReference(lastComponent, refLoc, depLoc, newNames.get(refLoc.getName()));
				try {
					if(!XMLUtil.compare(lastComponent, componentOld)){
						elementFound(refLoc);
					}
				} catch (IOException e) {
					throw new ConfigurationException("Error while comparing the new/old versions: " + e.getMessage());
				}	
			}
		}
		lastComponent = null;

		/* handle variable references in all components*/
		walkTree(this.project.getChild("connections"), new Properties());
		walkTree(this.project.getChild("extracts"), new Properties());
		walkTree(this.project.getChild("transforms"), new Properties());
		walkTree(this.project.getChild("loads"), new Properties());
		walkTree(this.project.getChild("jobs"), new Properties());
	}
	
	
}
