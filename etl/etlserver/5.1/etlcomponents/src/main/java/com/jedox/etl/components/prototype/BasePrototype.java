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
*/
package com.jedox.etl.components.prototype;

import java.util.ArrayList;
import java.util.List;

import org.jdom.CDATA;
import org.jdom.Element;

import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.config.Settings;

public abstract class BasePrototype  {

	protected Element project;

	protected Element addChild(Element el, String tagname, String content) {
		Element child=new Element(tagname);
		if (content!=null) {
			child.addContent(content);
		}	
		el.addContent(child);
		return child;
	}

	protected Element addChild(Element el, String tagname) {
		return addChild(el, tagname, null);
	}
		
	protected Element addNameref(Element el, String scope, String nameref) {
		Element child=new Element(scope.substring(0,scope.length()-1));
		child.setAttribute("nameref", nameref);
		el.addContent(child);
		return child;
	}	

	protected Element addInput(Element el, boolean isNameref, String name) {
		Element child = new Element("input");
		child.setAttribute(isNameref ? "nameref" : "constant", name);
		el.addContent(child);
		return child;
	}
	
	protected Element rename(Element el, String newName) {
		el.setAttribute("name", newName);
		return el;
	}
		
	protected Element addComponent(String scope, String name, String type) {
		Element component = new Element(scope.substring(0,scope.length()-1));
		project.getChild(scope).addContent(component);
		component.setAttribute("name",name);
		component.setAttribute("type",type);
		return component;
	}
	
	protected String[] getAllComponentsForType(String scope) {
		List<String> names = new ArrayList<String>();
		@SuppressWarnings("unchecked")
		List<Element> components = project.getChild(scope).getChildren();
		for (Element component : components) {
			names.add(component.getAttributeValue("name"));
		}
		return names.toArray(new String[names.size()]);
	}
	
	protected Element getComponent(String scope, String name) {
		@SuppressWarnings("unchecked")
		List<Element> components = project.getChild(scope).getChildren();
		for (Element component : components) {
			if (component.getAttributeValue("name","").equals(name))
				return component;
		}
		return null;
	}	
	
	protected Element addFunction (String name, String transformName, String type) {
		Element transform=getComponent("transforms", transformName);
		Element functions = transform.getChild("functions");
		if (functions==null)
			functions=addChild(transform,"functions");
		Element function = addChild(functions,"function");
		function.setAttribute("type", type);
		function.setAttribute("name", name);
		addChild(function,"inputs");
		addChild(function,"parameters");
		return function;
	}
	
	public Element cloneComponent(Element comp, String newName){
		Element parent = comp.getParentElement();
		Element clone = (Element) comp.clone();
		rename(clone,newName);
		parent.addContent(clone);
		return clone;
	}
	
	public String getComponentName(Element comp){
		return comp.getAttributeValue("name");
	}	

	protected void initEmptyProject(String projectName, String description) {
		project = new Element("project");
		project.setAttribute("name", projectName);
		project.setAttribute("version", ""+Settings.versionetl);
		Element headers = new Element("headers");
		Element header = new Element("header");
		Element comment = new Element("comment");
		headers.addContent(header);
		header.addContent(comment);
		header.setAttribute("name", "comment");
		CDATA cdata = new CDATA(description);
		comment.addContent(cdata);
		project.addContent(headers);
		Element vars = new Element("variables");
		project.addContent(vars);
		
		project.addContent(new Element(ITypes.Connections));
		project.addContent(new Element(ITypes.Extracts));
		project.addContent(new Element(ITypes.Transforms));
		project.addContent(new Element(ITypes.Loads));
		project.addContent(new Element(ITypes.Jobs));		
	}
	
	
}
