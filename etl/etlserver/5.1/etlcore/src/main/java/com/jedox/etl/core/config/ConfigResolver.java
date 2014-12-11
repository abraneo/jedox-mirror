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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.config;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.jdom.Element;
import org.jdom.Attribute;
import org.jdom.Text;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.context.IContext;

public class ConfigResolver {

	protected static Pattern variable = Pattern.compile("\\$\\{[^{}<>]{1,}\\}");
	
	private List<String> variables = new ArrayList<String>();

	public ConfigResolver() {
	}

	protected String resolve(String text, Properties properties) throws CreationException {
		if (text != null) {
			Matcher m = variable.matcher(text);
			while (m.find()) {
				String group = m.group();
				String key = group.substring(2,group.length()-1);
				String value = properties.getProperty(key, group);
				if (value.equalsIgnoreCase(group)) {
					String message = "Variable "+group+" not found in context. Please specify value";
					throw new CreationException(message);
				}
				variables.add(key);
				text = text.replace((CharSequence)group, (CharSequence)value);
			}
		}
		return text;
	}

	@SuppressWarnings("unchecked")
	protected void resolveElement(Element element, Properties properties) throws CreationException {
		
		if(element.getName().equals("comment"))
			return;
		
		element.setName(resolve(element.getName(),properties));
		List<?> content = element.getContent();
		for (Object c : content) {
			if (c instanceof Text) {
				Text t = (Text) c;
				t.setText(resolve(t.getTextTrim(),properties));
			}
		}
		List<Attribute> attributes = element.getAttributes();
		for (Attribute a : attributes) {
			a.setName(resolve(a.getName(),properties));
			a.setValue(resolve(a.getValue(),properties));
		}
	}

	@SuppressWarnings("unchecked")
	protected void walkTree(Element element, Properties properties) throws CreationException {
		resolveElement(element, properties);
		List<Element> l = element.getChildren();
		for (Element e : l) {
			walkTree(e, properties);
		}
	}

	public Element resolve(Element config, IContext context) throws CreationException {
		Properties properties = new Properties();
		if (context != null)
			properties.putAll(context.getVariables());
		Element element = (Element) config.clone();
		walkTree(element,properties);
		return element;
	}
	
	public List<String> getVariables() {
		return variables;
	}
}
