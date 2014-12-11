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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.config.extract;

import java.util.ArrayList;
import java.util.List;
import org.jdom.Element;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.source.*;

public class XmlExtractConfigurator extends TableSourceConfigurator {

	private String level; // the path of the element in the xml file
	private ArrayList<String> fields; // the fields needed to be read
	private ArrayList<String> namespaces =  new ArrayList<String>(); // list of namespaces given as (name,value,name,value,...)


	/**
	 * set the level and the fields from the xml
	 * @throws ConfigurationException
	 */
	@SuppressWarnings("unchecked")
	protected void setQueryData() throws ConfigurationException {

		Element query = getXML().getChild("query");

		if(query != null){
			Element levelElement = query.getChild("level");
			if(levelElement != null)
				level = levelElement.getText();
			else throw new ConfigurationException("Element level should be specified");

			Element fieldsElement = query.getChild("fields");

			if(fieldsElement != null) {
				List<Element> fieldsElements = (List<Element>)fieldsElement.getChildren("field");
				if(fieldsElements.size()<1){
					throw new ConfigurationException("At least one field should specified");
				}else{
					fields = new ArrayList<String>();
					for(int i=0;i<fieldsElements.size();i++){
						Element field = (Element)fieldsElements.get(i);
						fields.add(field.getText());
					}
				}
			}else{
				throw new ConfigurationException("At least one field should specified");
			}
		}
	}

	public ArrayList<String> getNamespaces() {
		return namespaces;
	}

	/**
	 * get needed fields
	 * @return get the needed fields
	 * @throws ConfigurationException
	 */
	public ArrayList<String> getFields() throws ConfigurationException {
		return fields;
	}

	/**
	 * get XML file level, which contain the needed data
	 * @return rules definitions
	 * @throws ConfigurationException
	 */
	public String getLevel() throws ConfigurationException {
		return level;
	}

	/**
	 * configure the source by configuring the super class, and setting the level and the fields
	 */
	public void configure() throws ConfigurationException {
		super.configure();
		setNamespaces();
		setQueryData();
	}

	private void setNamespaces() {
		Element namespacesList = getXML().getChild("namespaces");
		if(namespacesList!= null){
			for(Object obj:namespacesList.getChildren()){
				Element namespace = (Element) obj;
				namespaces.add(namespace.getAttributeValue("name"));
				namespaces.add(namespace.getValue());
			}
		}

	}

}
