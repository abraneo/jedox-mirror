package com.jedox.etl.core.persistence.hibernate;

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.sql.ResultSet;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.type.Type;
import org.hibernate.type.TypeFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Text;

import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.util.NamingUtil;

public class MappingHelper {
	
	protected final static String strategyAssigned = "assigned";
	protected final static String strategyNative = "native";
	protected final static String strategyIncrement = "increment";
	protected final static String strategyHilo = "hilo";
	protected final static String strategySequence = "sequence";
	protected final static String strategyIdentity = "identity";
	protected final static String strategyGuid = "guid";
	protected final static String targetExisting ="existing";
	protected final static String targetNew ="new";
	protected final static int defaultStringLength = 254;
	protected final static String idName = "id";
	
	private PersistorDefinition definition;
	private Map<String,String> columnMap = new HashMap<String, String>();
	private Map<Integer,Integer> precisionMap = new HashMap<Integer, Integer>();
	private static final Log log = LogFactory.getLog(MappingHelper.class);
	
	public MappingHelper(PersistorDefinition definition) {
		this.definition = definition;
		try {
			//fix for postresql!!
			definition.getConnection().commit();
			ResultSet info = definition.getConnection().open().getMetaData().getTypeInfo();
			while (info.next()) {
				int codeNumber = info.getInt("DATA_TYPE");
				int precision = info.getInt("PRECISION");
				precisionMap.put(codeNumber,precision);
			}
		}
		catch (Exception e) {
			
		}
	}

	private String getPropertyName(int i) {
		return "p"+i;
	}
	
	public Map<String,String> getColumnMap() {
		return columnMap;
	}
	
	protected Row createIdMapping(Document document, Element parent, Row data, Row primaryKeys) {
		Row keys = new Row();
		keys.addColumns(primaryKeys);
		Element id = null;
		if (keys.size() == 1) {
			IColumn keyColumn = keys.getColumn(0);
			id = document.createElement("id");
			Type type = TypeFactory.heuristicType(keyColumn.getValueType());
			id.setAttribute("type", type.getName());
			id.setAttribute("column", "`"+keyColumn.getName()+"`");
			Element generator = document.createElement("generator");
			if (definition.getColumnDefinition().containsColumn(keyColumn.getName())) { //externally provided primary key, which is filled by provided data
				if (definition.getPrimaryKeyGeneration() != null && !strategyAssigned.equals(definition.getPrimaryKeyGeneration()))
					log.warn("Overruling primary key generation strategy on column "+keyColumn.getName()+" by assigning given input data.");
				definition.setPrimaryKeyGeneration(strategyAssigned);
				generator.setAttribute("class", strategyAssigned);
			} else if (NamingUtil.internalHibernateKeyName().equals(keyColumn.getName())) {//internally created default primary key, use hilo by default
				if (definition.getPrimaryKeyGeneration() == null)
					definition.setPrimaryKeyGeneration(strategyHilo);
				generator.setAttribute("class", definition.getPrimaryKeyGeneration());
			} else { //externally provided auto-generated primary key. use by default native functionality which may be slower
				if (definition.getPrimaryKeyGeneration() == null)
					definition.setPrimaryKeyGeneration(strategyNative);
				generator.setAttribute("class", definition.getPrimaryKeyGeneration());
			}
			//set hilo lookup table schema if needed.
			if (strategyHilo.equals(definition.getPrimaryKeyGeneration()) && !definition.getLocator().getPersistentSchema().isEmpty()) {
				Element schemaParam = document.createElement("param");
				schemaParam.setAttribute("name", "schema");
				schemaParam.appendChild(document.createTextNode("`"+definition.getLocator().getPersistentSchema()+"`"));
				generator.appendChild(schemaParam);
			}
			id.setAttribute("name", idName);
			id.appendChild(generator);
			getColumnMap().put(keyColumn.getName(), idName);
			log.debug("Using primary key generation strategy "+definition.getPrimaryKeyGeneration()+" on column "+keyColumn.getName());
		}
		else {
			//if (keys.size() == 0) //if there is no key make all keys primary keys! use composite key element
			//	keys.addColumns(manager.getColumnsOfType(ColumnTypes.key));
			if (keys.size() == 0) //if there are still no keys make all columns primary keys!
				keys.addColumns(data);
			id = document.createElement("composite-id");
			for (IColumn c: keys.getColumns()) {
				Element ref = document.createElement("key-property");
				String columnName = getPropertyName(data.indexOf(c));
				getColumnMap().put(c.getName(), columnName);
				ref.setAttribute("name", columnName);
				Type type = TypeFactory.heuristicType(c.getValueType());
				ref.setAttribute("type", type.getName());
				ref.setAttribute("column", "`"+c.getName()+"`");
				id.appendChild(ref);
			}
			definition.setPrimaryKeyGeneration(strategyAssigned);
		}
		parent.appendChild(id);
		return keys;
	}
	
	protected int getMaximumPrecision(Type type, int defaultPrecision) {
		int precision = defaultPrecision;
		for (int i: type.sqlTypes(null)) {
			precision = precisionMap.get(i);
		}
		return precision;
	}

	protected List<Element> createDataMapping(Document document, Row data, Row keys) {
		List<Element> elements = new ArrayList<Element>();
		for (int i=0; i<data.size(); i++) {
			IColumn c = data.getColumn(i);
			if (!keys.containsColumn(c.getName())) {
				Element e = document.createElement("property");
				String columnName = getPropertyName(i);
				getColumnMap().put(c.getName(), columnName);
				e.setAttribute("name", columnName);
				//e.setAttribute("name", c.getName());
				//check if this works!!
				Type type = TypeFactory.heuristicType(c.getValueType());
				e.setAttribute("type", type.getName());
				e.setAttribute("column", "`"+c.getName()+"`");
				//set default string length if we create this target
				if (targetNew.equals(data.getName())) {
					if (c.getValueType().equals(String.class.getCanonicalName())) 
						e.setAttribute("length", String.valueOf(getMaximumPrecision(type,defaultStringLength)));
				}
				elements.add(e);
			}
		}
		return elements;
	}

	public Document createMappingDocument(Row data, Row primaryKeys) throws ParserConfigurationException, RuntimeException {
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		Document document = factory.newDocumentBuilder().newDocument();
		Element root = document.createElement("hibernate-mapping");
		document.appendChild(root);
		Element clazz = document.createElement("class");
		clazz.setAttribute("entity-name", definition.getLocator().getName());
		if (!definition.getLocator().getPersistentSchema().isEmpty()) {
			clazz.setAttribute("schema", "`"+definition.getLocator().getPersistentSchema()+"`");
		}
		clazz.setAttribute("table", "`"+definition.getLocator().getName()+"`");
		root.appendChild(clazz);
		Row keys = createIdMapping(document,clazz,data,primaryKeys);
		for (Element e : createDataMapping(document,data, keys)) {
			clazz.appendChild(e);
		}
		log.debug(sourceToXMLString(new DOMSource(document)));
		return document;
	}
	
	private String sourceToXMLString(Source result) {
		String xmlResult = null;
		try {
			TransformerFactory factory = TransformerFactory.newInstance();
			Transformer transformer = factory.newTransformer();
			transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
			transformer.setOutputProperty(OutputKeys.METHOD, "xml");
			OutputStream out = new ByteArrayOutputStream();
			StreamResult streamResult = new StreamResult();
			streamResult.setOutputStream(out);
			transformer.transform(result, streamResult);
			xmlResult = streamResult.getOutputStream().toString();
		} catch (TransformerException e) {
			e.printStackTrace();
		}
		return xmlResult;
	}

}
