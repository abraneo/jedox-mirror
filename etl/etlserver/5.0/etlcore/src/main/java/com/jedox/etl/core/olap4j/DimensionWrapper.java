package com.jedox.etl.core.olap4j;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.olap4j.OlapException;
import org.olap4j.metadata.Dimension;
import org.olap4j.metadata.Level;
import org.olap4j.metadata.NamedList;
import org.olap4j.metadata.Schema;
import org.olap4j.metadata.Member;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.Consolidation;
import com.jedox.palojlib.main.DimensionInfo;

public class DimensionWrapper implements IDimension {
	
	private class Olap4jDimensionInfo extends DimensionInfo {
		public Olap4jDimensionInfo(String maxLevel, String maxDepth) {
			super("","",maxLevel,"0",maxDepth,"0");
		}
	}
	
	private Dimension dimension;
	private Schema schema;
	private Olap4jDimensionInfo info;
	private ElementCache elements;
	private static final Log log = LogFactory.getLog(DimensionWrapper.class);
	
	public DimensionWrapper(Dimension dimension, Schema schema) throws PaloException {
		this.dimension = dimension;
		this.schema = schema;
		NamedList<Level> levels = dimension.getDefaultHierarchy().getLevels();
		int maxDepth = 0;
		int maxLevel = levels.size();
		for (Level l : levels) {
			maxDepth = Math.max(maxDepth, l.getDepth());
		}
		info = new Olap4jDimensionInfo(String.valueOf(maxLevel),String.valueOf(maxDepth));
		elements = new ElementCache(this);
	}

	@Override
	public String getName() {
		return dimension.getName();
	}

	@Override
	public int getId() {
		try {
			return schema.getSharedDimensions().indexOf(dimension);
		}
		catch (OlapException e) {
			return 0;
		}
	}

	@Override
	public DimensionType getType() {
		return DimensionType.DIMENSION_NORMAL;
	}

	@Override
	public DimensionInfo getDimensionInfo() throws PaloException {
		return info;
	}

	@Override
	public IElement[] getElements(boolean withAttributes) throws PaloException {
		return getElementCache().getElements();
	}

	@Override
	public IElement[] getRootElements(boolean withAttributes) throws PaloException {
		try {
			NamedList<Member> members = dimension.getDefaultHierarchy().getRootMembers();
			List<IElement> elements = new ArrayList<IElement>();
			for (Member m : members) {
				elements.add(getElementCache().getElement(m.getName()));
			}
			return elements.toArray(new IElement[elements.size()]);
		}
		catch (OlapException e) {
			throw new PaloException("Cannot get root elements from dimension "+dimension.getName()+": "+e.getMessage());
		}
	}

	@Override
	public ElementWrapper getElementByName(String name, boolean withAttributes) throws PaloException {
		return getElementCache().getElement(name);
	}

	@Override
	public AttributeWrapper getAttributeByName(String name) throws PaloException {
		return getElementCache().getAttribute(name);
	}

	@Override
	public void addElements(String[] names, ElementType[] types) {
		throw new PaloException("Adding elements is not supported by OLAP4j provider.");
	}

	@Override
	public IElement addBaseElement(String name, ElementType type) {
		throw new PaloException("Adding elements is not supported by OLAP4j provider.");
	}

	@Override
	public void addAttributes(String[] names, ElementType[] types) {
		throw new PaloException("Adding attributes is not supported by OLAP4j provider.");
	}

	@Override
	public void removeElements(IElement[] elements) {
		throw new PaloException("Removing elements is not supported by OLAP4j provider.");
	}

	@Override
	public void removeAttributes(IAttribute[] attributes) {
		throw new PaloException("Removing attributes is not supported by OLAP4j provider.");
	}

	@Override
	public void removeAttributeValues(IAttribute attribute, IElement[] elements) {
		for (IElement e : elements) {
			for (Member m : getElementCache().getMembers(e.getName())) {
				try {
					m.setProperty(getElementCache().getAttribute(attribute.getName()).getProperty(), null);
				} catch (OlapException ex) {
					log.error("Cannot remove attribute value: "+ex.getMessage());
				}
				catch (UnsupportedOperationException ex1) {
					log.error("Removing attribute values is not supported by OLAP4j provider.");
				}
			}
		}
	}

	@Override
	public void addAttributeValues(IAttribute attribute, IElement[] elements, Object[] values) {
		int i = 0;
		for (IElement e : elements) {
			for (Member m : getElementCache().getMembers(e.getName())) {
				try {
					m.setProperty(getElementCache().getAttribute(attribute.getName()).getProperty(), values[i]);
				} catch (OlapException ex) {
					log.error("Cannot add attribute value: "+ex.getMessage());
				}
				catch (UnsupportedOperationException ex1) {
					log.error("Adding attribute values is not supported by OLAP4j provider.");
				}
			}
			i++;
		}

	}

	@Override
	public void removeConsolidations(IElement[] elements) {
		throw new PaloException("Removing consolidations is not supported by OLAP4j provider.");
	}


	@Override
	public void removeAttributeConsolidations(IAttribute attribute) {
		throw new PaloException("Removing attribute consolidations is not supported by OLAP4j provider.");

	}

	@Override
	public void addAttributeConsolidation(IAttribute attribute, IAttribute child) {
		throw new PaloException("Adding attribute consolidations is not supported by OLAP4j provider.");
	}

	@Override
	public Consolidation newConsolidation(IElement parent, IElement child,
			double weight) {
		throw new PaloException("Adding consolidations is not supported by OLAP4j provider.");
	}

	@Override
	public IAttribute[] getAttributes() {
		return getElementCache().getAttributes(true);
	}

	@Override
	public void rename(String newname) throws PaloException {
		throw new PaloException("Renaming dimension is not supported by OLAP4j provider.");
	}

	@Override
	public void updateConsolidations(IConsolidation[] consolidations) {
		throw new PaloException("Updateing consolidation is not supported by OLAP4j provider.");	
	}
	
	public ElementCache getElementCache() {
		return elements;
	}
	
	public Dimension getDimension() {
		return dimension;
	}

	@Override
	public HashMap<String, IElement[]> getChildrenMap() throws PaloException {
		throw new PaloException("Not supported by OLAP4j provider.");
	}

	@Override
	public HashMap<String, HashMap<String, Object>> getAttributesMap()throws PaloException {
		throw new PaloException("Not supported by OLAP4j provider.");
	}

	@Override
	public HashMap<String, HashMap<String, Double>> getWeightsMap() throws PaloException {
		throw new PaloException("Not supported by OLAP4j provider");
	}

	@Override
	public void setCacheTrustExpiry(int arg0) {
		// throw new PaloException("Not supported by OLAP4j provider");
		
	}

	@Override
	public void updateElementsType(IElement[] arg0, ElementType arg1)
			throws PaloException, PaloJException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public IElement[] getBasesElements(boolean arg0) throws PaloException,
			PaloJException {
		// TODO Auto-generated method stub
		return null;
	}


}
