package com.jedox.etl.core.node.tree;

import com.jedox.etl.core.node.INamedValue;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public interface ITreeElement extends INamedValue<ElementType>, IElement {
	
	public ITreeElement[] getChildren() throws PaloException, PaloJException;
	public ITreeElement[] getParents() throws PaloException, PaloJException;

}
