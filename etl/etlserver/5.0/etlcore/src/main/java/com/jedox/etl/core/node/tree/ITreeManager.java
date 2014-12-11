package com.jedox.etl.core.node.tree;

import java.util.List;

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public interface ITreeManager extends IDimension {

	public ITreeElement getElement(String name);
	public ITreeElement[] getElementsIgnoreCase(boolean withAttributes);
	public ITreeElement provideElement(String name, ElementType type) throws PaloJException;
	public ITreeElement getChild(IElement element, String childName) throws PaloJException;
	public ITreeElement getParent(IElement element, String parentName) throws PaloJException;
	public List<ITreeElement> getAncestors(IElement element, boolean includeElement) throws PaloJException;
	public List<ITreeElement> getDescendants(IElement element, boolean includeElement) throws PaloJException;
	public Attribute addAttribute(String name, ElementType type) throws PaloJException;
	public void addAttributes(IAttribute[] attributes, boolean override);
	public Attribute[] getAttributes() throws PaloException, PaloJException; 
	public void addAttributeValue(String attribute, String element, Object value) throws PaloJException; 
	public void commitAttributeValues() throws PaloJException;
	public void addElements(IElement[] elements, boolean withConsolidations);
	public void retainElements(String[] elementNames) throws PaloJException;
	public void addSubtree(IElement subTreeRoot, IConsolidation[] consolidations) throws PaloJException;
	public Object getAttributeValue(String attributeName, String elementName, boolean verify) throws PaloJException;
	public void removeConsolidation(IElement element) throws PaloJException;
	public void removeConsolidations(IConsolidation[] consolidations) throws PaloException, PaloJException;
	public void clearConsolidations() ;
	public ITreeElement renameElement(String oldName, String newName) throws PaloJException;
	public void addConsolidation(IElement parent, IElement child, double weight); 
	public void addConsolidations(IConsolidation[] consolidations);
	public ITreeElement[] getRootElements(boolean withAttributes) throws PaloException, PaloJException;
	public ITreeElement addBaseElement(String name, ElementType type) throws PaloJException, PaloException;
	public ITreeElement[] getElements(boolean withAttributes) throws PaloException, PaloJException;
	public IConsolidation[] getConsolidations();
	public void commitConsolidations() throws PaloJException;
	public void clear();
	public int getLevelsCount();
	public List<ITreeElement> getElementsByAttribute(String attribute, String value);
	public void setAutoCommit(boolean autoCommit);
	public boolean isAutoCommit();

}
