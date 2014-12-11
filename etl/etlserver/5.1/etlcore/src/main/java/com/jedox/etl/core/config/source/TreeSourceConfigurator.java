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
package com.jedox.etl.core.config.source;

import java.util.List;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.Attribute.AttributeModes;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
/**
 * Basic Configurator class for Sources based on trees and thus providing multiple rendering possibilities as tables.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class TreeSourceConfigurator extends TableSourceConfigurator {

	private static final Log log = LogFactory.getLog(TreeSourceConfigurator.class);
	private ITreeManager manager;
	
	private double parseWeight(String weightString) {
		double weight = 1.0;
		try {
			weight = Double.parseDouble(weightString.trim());
		}
		catch (Exception e) {
			log.error("Failed to set weigth of node. Falling back to default 1.");
		}
		return weight;
	}
	
	protected ElementType getElementType(String name) {
		if ("string".equalsIgnoreCase(name) || "text".equalsIgnoreCase(name) || "alias".equalsIgnoreCase(name))
			return ElementType.ELEMENT_STRING;
		return ElementType.ELEMENT_NUMERIC;
	}
	
	private void processNodeAttributes(ITreeManager manager, IElement node, List<?> attributes) throws ConfigurationException {
		for (int j=0; j<attributes.size(); j++) {
			Element attribute = (Element) attributes.get(j);
			String name = attribute.getAttributeValue("name",attribute.getName());
			String type = attribute.getAttributeValue("type","string");
			Attribute a = manager.addAttribute(name, getElementType(type));
			a.setMode(type.equalsIgnoreCase("alias") ? AttributeModes.ALIAS : AttributeModes.ATTRIBUTE);
			manager.addAttributeValue(a.getName(), node.getName(), attribute.getTextTrim());
		}
	}
	
	private void processNodes(List<?> children, ITreeManager manager, IElement parent) throws ConfigurationException {
		for (int i=0; i<children.size(); i++) {
			Element child = (Element) children.get(i);
			String name = child.getAttributeValue("name");
			String weightString = child.getAttributeValue("weight","1");
			IElement c = manager.addBaseElement(name, getElementType(child.getAttributeValue("type")));
			manager.addConsolidation(parent,c,parseWeight(weightString));
			processNodeAttributes(manager,c, child.getChildren("attribute"));
			processNodeAttributes(manager,c, child.getChildren("rule"));
			processNodes(child.getChildren("constant"), manager, c);
		}
		manager.commitConsolidations();
		manager.commitAttributeValues();
	}
	
	protected void setNodes() throws ConfigurationException {
			processNodes(getChildren(getXML(),"constant"),manager,null);
	}

	/**
	 * gets the Tree Node Manager, which manages the internal tree representation
	 * @return the Tree Node Manager
	 */
	public ITreeManager getNodeManager() {
		return manager;
	}
	
	/**
	 * return the default format the tree should be rendered in
	 */
	public Views getFormat() throws ConfigurationException {
		return Views.FH;
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		manager = new TreeManagerNG(getName());
		boolean autocommit = manager.isAutoCommit();
		manager.setAutoCommit(false);
		setNodes();
		manager.setAutoCommit(autocommit);
	}

}
