package com.jedox.etl.core.scriptapi;

import org.jdom.Element;

import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.Manager;
import com.jedox.etl.core.component.RuntimeException;

public class MixedManager extends Manager {

	@Override
	public IComponent add(Element config) throws CreationException, RuntimeException {
		throw new CreationException("Not implemented");
	}
	
	public String getName() {
		return ITypes.Any;
	}
	
}