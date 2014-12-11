package com.jedox.etl.components.config.transform;

import java.util.List;

import org.jdom.Element;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.transform.ColumnConfigurator;
import com.jedox.etl.core.config.transform.ITransformConfigurator;
import com.jedox.etl.core.node.AttributeNode;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeTransformConfigurator extends TreeViewConfigurator implements ITransformConfigurator {
	
	private Row row;

	@Override
	public Row getRow() throws RuntimeException {
		return row;
	}

	@Override
	public List<IComponent> getSources() throws ConfigurationException {
		return getConfigUtil().getSources();
	}

	@Override
	public List<IComponent> getFunctions() throws ConfigurationException {
		return getConfigUtil().getFunctions();
	}
	
	public Views getFormat() throws ConfigurationException {
		Element sources = getXML().getChild("sources");
		if (sources != null) {
			Element source = sources.getChild("source");
			if (source != null) return Views.valueOf(source.getAttributeValue("format", "ea").toUpperCase());
		}
		return Views.EA;
	}
	
	private Element getTarget() {
		return getXML().getChild("target");
	}
	
	private Row setupRow() {
		Element target = getTarget();
		if (target != null) {
			Row row = new Row();
			ColumnConfigurator conf = new ColumnConfigurator(getName());
			List<?> columns = getChildren(getTarget(),"attribute");
			for (int j=0; j<columns.size(); j++) {
				Element column = (Element) columns.get(j);
				String inputName = conf.getInputName(column);
				String name = conf.getColumnName(column,inputName);
				String value = conf.getInputValue(column);
				if (inputName.equals(NamingUtil.skipColumn()))			
					continue;		
				ElementType elementType = ElementType.ELEMENT_STRING;		
				if (column.getAttributeValue("type","string").equalsIgnoreCase("numeric")) {
					elementType = ElementType.ELEMENT_NUMERIC;
				}
				Attribute a = new Attribute(name,elementType);
				AttributeNode aNode = ColumnNodeFactory.getInstance().createAttributeNode(a, new Column(inputName));
				if (value != null) aNode.setValue(value);
				row.addColumn(aNode);
			}
			return row;
		}
		return null;
	}
	
	public void configure() throws ConfigurationException {
		super.configure();
		row = setupRow();
	}

}
