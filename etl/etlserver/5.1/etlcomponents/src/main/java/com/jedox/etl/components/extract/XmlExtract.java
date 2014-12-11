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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.extract;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jaxen.JaxenException;
import org.jaxen.Navigator;
import org.jaxen.function.StringFunction;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import com.jedox.etl.components.config.extract.XmlExtractConfigurator;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IXmlConnection;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.extract.IExtract;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.source.processor.Processor;
import com.jedox.etl.core.util.PersistenceUtil;



public class XmlExtract extends TableSource implements IExtract {

	private static final Log log = LogFactory.getLog(XmlExtract.class);
	private String level;
	private ArrayList<String> fields;
	private Document xmlDoc;
	private ArrayList<String> namespaces;

	private class XmlProcessor extends Processor {

		private Row row;
		private int count;
		private ArrayList<String> header;
		private ArrayList<ArrayList<String>> data;

		private Iterator<ArrayList<String>> dataIterator;

		public XmlProcessor(int size) throws RuntimeException {
			this.setLastRow(size);
		}

		/**
		 * read ALL the data and store it in the processor. All the data is stored once because when doing
		 * the first step (searching the level nodes), we retrieve all the elements that we need to read.
		 *
		 */
		@SuppressWarnings("unchecked")
		protected void init() throws RuntimeException{
			try{
				xmlDoc = getConnection().open();
				//String ss = xmlDoc.getDocumentElement().getElementsByTagName("rows").item(0).getChildNodes().item(1).getChildNodes().item(1).getTextContent();
                //log.info(XMLUtil.w3cDocumentToString(xmlDoc));
				//the data in the xmlfile
				data = new ArrayList<ArrayList<String>>();

				//get the nodes in the XML file that should be searched
				//XPathFactory xPathFactory =  XPathFactory.newInstance();
				//XPath xpath = xPathFactory.newXPath();

				org.jaxen.dom.DOMXPath expression = new org.jaxen.dom.DOMXPath(level);
				setNamespaces(expression);
				Navigator navigator = expression.getNavigator();
				List<?> levelElements = expression.selectNodes(xmlDoc);

				//NodeList levelElements = (NodeList) xpath.evaluate(level, xmlDoc,XPathConstants.NODESET);

				//set the header
				ArrayList<String> row = new ArrayList<String>();
				for(String field:fields)
					row.add(field);
				header = (ArrayList<String>) row.clone();

				// prepare the XPath queries
				org.jaxen.dom.DOMXPath [] fieldExpressions = new org.jaxen.dom.DOMXPath[fields.size()];
				for(int i=0;i<fieldExpressions.length;i++){
					org.jaxen.dom.DOMXPath fieldExp = new org.jaxen.dom.DOMXPath(fields.get(i));
					setNamespaces(fieldExp);
					fieldExpressions[i] = fieldExp;
				}

				//set the data
				for(int i=0;i<levelElements.size();i++){
					Node levelNode = (Node)levelElements.get(i);
					row.clear();
					for(org.jaxen.dom.DOMXPath fieldExp:fieldExpressions){
						Node n = (Node) fieldExp.selectSingleNode(levelNode);
						StringFunction.evaluate(n,navigator);
						row.add(StringFunction.evaluate(n,navigator));
						//row.add((String) xpath.evaluate(field, levelNode,XPathConstants.STRING));
					}
					data.add((ArrayList<String>)row.clone());

					if(getLastRow()!= 0 && i>=getLastRow())
						break;
				}
			}
			catch(Exception e){
				throw new RuntimeException(e.getMessage());
			}
			dataIterator = data.iterator();
			xmlDoc = null;
			row = PersistenceUtil.getColumnDefinition(getAliasMap(),getColumns(header));
			row.setAliases(getAliasMap());
		}

		private void setNamespaces(org.jaxen.dom.DOMXPath expression)throws JaxenException {
			for(int i=0;i<namespaces.size();){
				expression.addNamespace(namespaces.get(i), namespaces.get(i+1));
				i = i+2;
			}
		}

		protected boolean fillRow(Row row) throws Exception {
			if (data != null && dataIterator.hasNext()) {
				count++;
				ArrayList<String> dataRow = (ArrayList<String>)dataIterator.next();
				if (dataRow != null) {
					try {
						for (int j=0; j<dataRow.size(); j++) {
							row.getColumn(j).setValue(dataRow.get(j));
						}
					}
					catch (Exception e) {
						log.error("Error in export cell: "+count+" "+e.getMessage());
					}
				}
				return true;
			}
			else { //finished ... do some cleanup
				dataIterator = null;
				data = null;
				count = 0;
				row = null;
				getConnection().close();
				return false;
			}
		}

		protected Row getRow() {
			return row;
		}

	}

	public XmlExtract() {
		setConfigurator(new XmlExtractConfigurator());
	}

	public XmlExtractConfigurator getConfigurator() {
		return (XmlExtractConfigurator) super.getConfigurator();
	}


	private String[] getColumns(ArrayList<String> header) {

		String [] names = new String[header.size()];
		for (int i=0; i<names.length; i++) {
			names[i] = header.get(i);
		}
		return names;
	}

	public IXmlConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IXmlConnection))
			return (IXmlConnection) connection;
		throw new RuntimeException("XML File connection is needed for extract "+getName()+".");
	}


	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		IProcessor processor = initProcessor(new XmlProcessor(size),Facets.OUTPUT);
		return processor;
	}
	
	public Row getOutputDescription() throws RuntimeException {			
		Row row = PersistenceUtil.getColumnDefinition(fields);
		row.setAliases(getAliasMap());
		return row;
	}

	public void init() throws InitializationException {
		super.init();
		try {
			level = getConfigurator().getLevel();
			fields = getConfigurator().getFields();
			namespaces = getConfigurator().getNamespaces();
		} catch (ConfigurationException e) {
			throw new InitializationException("Can not initialise extract "+getName()+":" + e.getMessage());
		}
	}

}
