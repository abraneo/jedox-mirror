package com.jedox.etl.core.olap4j;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.olap4j.OlapException;
import org.olap4j.metadata.Member;
import org.olap4j.metadata.NamedList;
import org.olap4j.metadata.Property;

public class ElementWrapper implements IElement {

	private Member member;
	private DimensionWrapper dimension;
	private IElement[] parents;
    private IElement[] children;
	private static final Log log = LogFactory.getLog(ElementWrapper.class);
	
	public ElementWrapper(DimensionWrapper dimension, Member member) {
		this.member = member;
		this.dimension = dimension;
	}
	
	@Override
	public String getName() {
		return member.getName();
	}

	@Override
	public ElementType getType() {
		try {
			if (member.getChildMemberCount() > 0) {
				return ElementType.ELEMENT_CONSOLIDATED;
			}
			else {
				/*
				AttributeWrapper a = dimension.getElementCache().getAttribute("VALUE");
				if (a != null) {
					switch (a.getProperty().getDatatype()) {
					case ACCP: return ElementType.ELEMENT_STRING;
					case BOOLEAN: return ElementType.ELEMENT_STRING;
					case CHAR: return ElementType.ELEMENT_STRING;
					case CUKY: return ElementType.ELEMENT_STRING;
					case STRING: return ElementType.ELEMENT_STRING;
					case VARIANT: return ElementType.ELEMENT_STRING;
					case DATS: return ElementType.ELEMENT_STRING;
					case FLTP: return ElementType.ELEMENT_STRING;
					case LCHR: return ElementType.ELEMENT_STRING;
					case SSTR: return ElementType.ELEMENT_STRING;
					case STRG: return ElementType.ELEMENT_STRING;
					case TIMS: return ElementType.ELEMENT_STRING;
					case VARC: return ElementType.ELEMENT_STRING;
					case UNIT: return ElementType.ELEMENT_STRING;
					default: return ElementType.ELEMENT_NUMERIC;
					}
				}
				*/
				return ElementType.ELEMENT_NUMERIC;
			}
		}
		catch (OlapException e) {
			log.error("Cannot get type of member "+member.getName()+": "+e.getMessage());
			return ElementType.ELEMENT_NUMERIC;
		}
	}
	
	private Property getProperty(String name) {
		for (Property p : member.getProperties()) {
			if (p.getName().equalsIgnoreCase(name)) return p;
		}
		return null;
	}

	@Override
	public Object getAttributeValue(String attributeName) {
		AttributeWrapper a = dimension.getElementCache().getAttribute(attributeName);
		//Property a = getProperty(attributeName);
		if (a != null) {
			try {
				return member.getPropertyValue(a.getProperty());
			}
			catch (OlapException e) {
				log.error("Cannot get attribute "+attributeName+" of element "+member.getName()+": "+e.getMessage());
			}
		}
		return null;
	}
	
	@Override
	public IElement[] getChildren() {
        if (children == null) {
			try {
				NamedList<? extends Member> mChildren = member.getChildMembers();
				IElement[] result = new IElement[mChildren.size()];
				for (int i=0; i<mChildren.size(); i++) {
					result[i] = dimension.getElementByName(mChildren.get(i).getName(), false);
				}
				children = result;
			}
			catch (OlapException e) {
				log.error("Cannot get children of member "+member.getName()+": "+e.getMessage());
				children = new IElement[0];
			}
        }
        return children;
	}

	@Override
	public IElement[] getParents() {
        if (parents == null) {
			List<Member> members = dimension.getElementCache().getMembers(member.getName());
			List<IElement> result = new ArrayList<IElement>();;
			for (int i=0; i<members.size(); i++) {
				if (members.get(i).getParentMember() != null) {
					result.add(dimension.getElementByName(members.get(i).getParentMember().getName(), true));
				}
			}
            parents = result.toArray(new IElement[result.size()]);
       }
       return parents;
	}
	
	/*

	@Override
	public IElement[] getChildren() {
		try {
			NamedList<? extends Member> children = member.getChildMembers();
			IElement[] result = new IElement[children.size()];
			for (int i=0; i<children.size(); i++) {
				result[i] = new ElementWrapper(dimension,children.get(i));
			}
			return result;
		}
		catch (OlapException e) {
			log.error("Cannot get children of member "+member.getName()+": "+e.getMessage());
			return null;
		}
	}

	@Override
	public IElement[] getParents() {
		List<Member> members = dimension.getElementCache().getMembers(member.getName());
		List<IElement> result = new ArrayList<IElement>();;
		for (int i=0; i<members.size(); i++) {
			if (members.get(i).getParentMember() != null) {
				result.add(new ElementWrapper(dimension,members.get(i).getParentMember()));
			}
		}
		return result.toArray(new IElement[result.size()]);
	}
	*/

	@Override
	public int getChildCount() {
		try {
			return member.getChildMemberCount();
		} catch (OlapException e) {
			log.error("Cannot get child count of member "+member.getName()+": "+e.getMessage());
			return 0;
		}
	}

	@Override
	public int getParentCount() {
		int parents = 0;
		List<Member> members = dimension.getElementCache().getMembers(member.getName());
		for (int i=0; i<members.size(); i++) {
			if (members.get(i).getParentMember() != null) {
				parents++;
			}
		}
		return parents;
	}

	@Override
	public double getWeight(IElement parent) {
		// TODO Find out if we can get the weight somehow.
		return 1;
	}

	@Override
	public HashMap<String, IElement[]> getSubTree() {
		HashMap<String,IElement[]> map = new HashMap<String,IElement[]>();
		IElement[] children = getChildren();
		map.put(getName(), children);
		for (IElement e : children) {
			if (!map.containsKey(e.getName()))
				map.putAll(e.getSubTree());
		}
		return map;
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getSubTreeAttributes() {
		HashMap<String,HashMap<String, Object>> map = new HashMap<String,HashMap<String, Object>>();
		IElement[] children = getChildren();
		HashMap<String,Object> values = new HashMap<String,Object>(); 
		for (IAttribute a : dimension.getAttributes()) {
			values.put(a.getName(), this.getAttributeValue(a.getName()));
		}
		map.put(getName(), values);
		for (IElement e : children) {
			if (!map.containsKey(e.getName()))
				map.putAll(e.getSubTreeAttributes());
		}
		return map;
	}

	@Override
	public void rename(String newname) throws PaloException {
		throw new PaloException("Renaming elements not supported by OLAP4j provider.");
	}
	
	public Member getMember() {
		return member;
	}

}
