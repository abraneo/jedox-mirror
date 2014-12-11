package com.jedox.etl.core.olap4j;
import java.util.ArrayList;
import java.util.Map;
import java.util.List;

import org.olap4j.OlapException;
import org.olap4j.metadata.Level;
import org.olap4j.metadata.Member;
import org.olap4j.metadata.NamedList;
import org.olap4j.metadata.Property;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IElement;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Set;

public class ElementCache {
	
	private Map<String,List<Member>> dimMemberLookup = new LinkedHashMap<String,List<Member>>();
    private Map<String,ElementWrapper> dimElementLookup = new LinkedHashMap<String,ElementWrapper>();
    private Map<String,ElementWrapper> dimRootElementLookup = new LinkedHashMap<String,ElementWrapper>();
	private Map<String,AttributeWrapper> attributes = new LinkedHashMap<String,AttributeWrapper>();
	
	
	public ElementCache(DimensionWrapper dimension) throws PaloJException {
		NamedList<Level> levels = dimension.getDimension().getDefaultHierarchy().getLevels();
		List<Member> members = new ArrayList<Member>(); 
		try {
                        Set<Member> rootMembers = new HashSet<Member>();
			for (Level l : levels) {
                                List<Member> mList = l.getMembers();
				members.addAll(mList);
				for (Property p : l.getProperties()) {
					if (!attributes.containsKey(p.getName())) {
						attributes.put(p.getName(), new AttributeWrapper(p));
					}
				}
                                if (l.getDepth() == 0) {
                                    rootMembers.addAll(mList);
                                }
			}
			for (Member m : members) {
				List<Member> membersPerElement = dimMemberLookup.get(m.getName());
				if (membersPerElement == null) {
					membersPerElement = new ArrayList<Member>();
					dimMemberLookup.put(m.getName(), membersPerElement);
                                        if (!dimElementLookup.containsKey(m.getName())) {
                                            ElementWrapper element = new ElementWrapper(dimension,m);
                                            dimElementLookup.put(m.getName(), element);
                                            if (rootMembers.contains(m)) {
                                                dimRootElementLookup.put(m.getName(), element);
                                            }
                                        }
				}
				membersPerElement.add(m);
			}
		}
		catch (OlapException e) {
			throw new PaloJException("Cannot cache elements from dimension "+dimension.getName()+": "+e.getMessage());
		}
	}


	
	public ElementWrapper getElement(String name) {
		return dimElementLookup.get(name);
	}
	
	public IElement[] getElements() {
		return dimElementLookup.values().toArray(new IElement[dimElementLookup.values().size()]);
	}

    public IElement[] getRootElements() {
        return dimRootElementLookup.values().toArray(new IElement[dimRootElementLookup.values().size()]);
    }
	
	public List<Member> getMembers(String name) {
		return dimMemberLookup.get(name);
	}
	
	public void clear() {
		dimMemberLookup.clear();
        dimElementLookup.clear();
        dimRootElementLookup.clear();
        attributes.clear();
	}
	
	public AttributeWrapper getAttribute(String name) {
		return attributes.get(name);
	}
	
	private boolean isUserAttribute(String name) {
		for (Property p : Property.StandardMemberProperty.values()) {
			if (p.getName().equals(name)) return false;
		}
		return true;
	}
	
	public IAttribute[] getAttributes(boolean userDefinedOnly) {
		List<IAttribute> result = new ArrayList<IAttribute>();
		for (IAttribute attribute : attributes.values()) {
			if (!userDefinedOnly || isUserAttribute(attribute.getName())) {
				result.add(attribute);
			}
		}
		return result.toArray(new IAttribute[result.size()]);
	}
	
	
}
