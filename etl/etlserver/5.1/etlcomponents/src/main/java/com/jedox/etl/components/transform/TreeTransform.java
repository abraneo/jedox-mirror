package com.jedox.etl.components.transform;

import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.TreeTransformConfigurator;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.function.FunctionManager;
import com.jedox.etl.core.node.AttributeNode;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.ColumnNodeFactory;
import com.jedox.etl.core.node.CoordinateNode;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.node.tree.Attribute;
import com.jedox.etl.core.node.tree.ITreeManager;
import com.jedox.etl.core.node.tree.TreeManagerNG;
import com.jedox.etl.core.source.IView.Views;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.ITreeProcessor;
import com.jedox.etl.core.source.processor.TreeBuildProcessor;
import com.jedox.etl.core.source.processor.TreeManagerProcessor;
import com.jedox.etl.core.source.processor.TreeViewProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.TransformInputProcessor;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;

public class TreeTransform extends TreeView {
	
	private Views format;
	private Row attributeRow;
	private Row targetRow;
	private static final String elementObjectName = NamingUtil.internal("element");
	//private static final Log log = LogFactory.getLog(TreeTransform.class);
	
	public TreeTransform() {
		setConfigurator(new TreeTransformConfigurator());
	}

	public TreeTransformConfigurator getConfigurator() {
		return (TreeTransformConfigurator)super.getConfigurator();
	}
	
	protected FunctionManager getFunctionManager() {
		return (FunctionManager)getManager(ITypes.Functions);
	}
	
	protected Row getFunctionOutputRow(IProcessor processor) throws RuntimeException {
		Row columns = new Row();
		for (IColumn c : processor.current().getColumns()) {
			CoordinateNode coordinate = ColumnNodeFactory.getInstance().createCoordinateNode(c.getName(), c);
			columns.addColumn(coordinate);
		}
		if (targetRow != null) { //we have a target remove all original attributes and only add target attributes
			for (IColumn c : super.getAttributes().getColumns()) {
				if (!elementObjectName.equals(c.getName())) columns.removeColumn(c.getName());
			}
			for (AttributeNode c : targetRow.getColumns(AttributeNode.class)) {
				if (columns.containsColumn(c.getName())) {
					throw new RuntimeException("Attribute "+c.getName()+" with input "+c.getInputName()+" is not allowed to appear in target. Name is reserved.");
				} else {
					columns.addColumn(c);
				}
			}
		} else {
			for (IColumn f : getFunctionManager().getAll()) {
				columns.addColumn(ColumnNodeFactory.getInstance().createCoordinateNode(f.getName(), new Column(f.getName())));
			}
		}
		return columns;
	}
	
	protected Row getAttributeRow(Row baseAttributes) {
		if (attributeRow == null) {
			attributeRow = new Row();
			for (AttributeNode a : baseAttributes.getColumns(AttributeNode.class)) {
				attributeRow.addColumn(a);
			}
			for (IColumn f : getFunctionManager().getAll()) {
				if (!attributeRow.containsColumn(f.getName())) {
					Attribute a = new Attribute(f.getName());
					attributeRow.addColumn(ColumnNodeFactory.getInstance().createAttributeNode(a, new Column(a.getName())));
				}
			}
		}
		return attributeRow;
	}
	
	public ITreeProcessor buildTree() throws RuntimeException {
		IDimension dimension = getDimensionObj();
		//add elementObjectName to treeView to have additional column later in TransformInputProcessor
		dimension.addAttributes(new String[]{elementObjectName}, new ElementType[]{ElementType.ELEMENT_STRING});
		//build a TreeViewProcessor delivering tabular format
		ITreeProcessor selectionProcessor = super.buildTree();
		IConsolidation[] selectedConsolidations = selectionProcessor.getManager().getConsolidations();
		ITreeProcessor treeViewProcessor = initTreeProcessor(new TreeViewProcessor(selectionProcessor,format),Facets.HIDDEN);
		//fill the originalElements attribute for selected nodes
		dimension = selectionProcessor.getManager();
		IElement[] originalElements = dimension.getElements(true);
		IAttribute elementObject = dimension.getAttributeByName(elementObjectName);
		Object[] elementObjectValues = new Object[originalElements.length];
		for (int i=0; i<originalElements.length; i++) {
			elementObjectValues[i] = originalElements[i];
		}
		dimension.addAttributeValues(elementObject, originalElements, elementObjectValues);
		//execute functions on tabular tree selection
		treeViewProcessor.current().getColumn(elementObjectName).setValueType(Object.class); //set object type for script functions to avoid conversion to string
		IProcessor functionProcessor = initProcessor(new TransformInputProcessor(treeViewProcessor,getFunctionManager(),getFunctionOutputRow(treeViewProcessor)),Facets.HIDDEN);
		//rebuild a tree with the modified selection
		TreeBuildProcessor treeBuilder = new TreeBuildProcessor(functionProcessor,format);
		Row attributes = new Row();
		attributes.addColumns(getAttributes());
		attributes.addColumn(ColumnNodeFactory.getInstance().createAttributeNode(new Attribute(elementObject.getName(),elementObject.getType()), new Column(elementObject.getName())));
		treeBuilder.setAttributes(attributes.getColumns(AttributeNode.class));		
		initProcessor(treeBuilder,Facets.HIDDEN).run();
		IConsolidation[] treeConsolidations = treeBuilder.getManager().getConsolidations();
		//rename modified elements and adjust types in original source before joining selection back
		ITreeManager fullSourceTree = new TreeManagerNG(getSource().getTreeManager());
		for (IElement element : treeBuilder.getManager().getElements(true)) {
			IElement origElement = (IElement)element.getAttributeValue(elementObjectName);
			if (origElement != null && !element.getName().equals(origElement.getName())) {
				fullSourceTree.renameElement(origElement.getName(), element.getName());
			}
			IElement fullTreeElement = fullSourceTree.getElementByName(element.getName(), false);
			if (fullTreeElement != null && fullTreeElement.getType() != element.getType()) {
				fullSourceTree.updateElementsType(new IElement[]{fullTreeElement}, element.getType());
			}
		}
		//add new attributes and rejoin selection elements
		treeBuilder.getManager().removeAttributes(new IAttribute[]{elementObject});
		fullSourceTree.addAttributes(treeBuilder.getManager().getAttributes(), false);
		fullSourceTree.addElements(treeBuilder.getManager().getElements(true), false);
		//update consolidations
		if (!Views.EA.equals(format)) {
			List<IConsolidation> treeConsolidationList = Arrays.asList(treeConsolidations);
			List<IConsolidation> selectedConsolidationList = Arrays.asList(selectedConsolidations);
			Set<IConsolidation> dropSet = new LinkedHashSet<IConsolidation>();
			dropSet.addAll(selectedConsolidationList);
			dropSet.removeAll(treeConsolidationList);
			Set<IConsolidation> fullSet = new LinkedHashSet<IConsolidation>();
			fullSet.addAll(Arrays.asList(fullSourceTree.getConsolidations()));
			fullSet.removeAll(dropSet);
			fullSet.addAll(treeConsolidationList);
			fullSourceTree.updateConsolidations(fullSet.toArray(new IConsolidation[fullSet.size()]));
		}
		return initTreeProcessor(new TreeManagerProcessor(fullSourceTree),Facets.INPUT);
	}
	
	public Row getAttributes() throws RuntimeException {
		return targetRow != null ? targetRow : getAttributeRow(super.getAttributes());
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			addManager(new FunctionManager());
			getFunctionManager().addAll(getConfigurator().getFunctions());
			format = getConfigurator().getFormat();
			targetRow = getConfigurator().getRow();
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
