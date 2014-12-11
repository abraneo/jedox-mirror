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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.load;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.dom4j.Element;
//import org.palo.api.Dimension;
//import org.palo.api.Database;
//import org.palo.api.Connection;
//import org.palo.api.subsets.Subset2;
//import org.palo.api.subsets.SubsetHandler;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.InputSource;
import com.jedox.etl.components.config.load.SubsetConfigurator;
import com.jedox.etl.core.component.CreationException;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.load.Load;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;

/**
 * load Class for OLAP Cube Rules.
 * @author Kais Haddadin
 *
 */


public abstract class SubsetLegacyLoad extends Load implements ILoad {
/*
	private static final Log log = LogFactory.getLog(SubsetLegacyLoad.class);
	private String dimensionName;
	private ArrayList<DimensionSubset> subsets;

	public class DimensionSubset {
		public String name; // name of the subset
		public String definition; // xml definition of the subset
	}


	*//**
	 * constructor that set the configurator
	 *//*

	public SubsetLegacyLoad() {
		setConfigurator(new SubsetConfigurator());
	}

	*//**
	 * get the configurator
	 *//*
	public SubsetConfigurator getConfigurator() {
		return (SubsetConfigurator)super.getConfigurator();
	}

	*//**
	 * get the connection
	 *//*
	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		throw new RuntimeException("OLAP connection is needed for source "+getName()+".");
	}


	*//**
	 * gets the name of the database hosting this dimension
	 * @return
	 *//*
	public String getDatabaseName() throws RuntimeException {
		return getConnection().getDatabase();
	}

	*//**
	 * get the subset from OLAP dimension if it exists
	 * Note: these rules are of type org.palo.api.Subset2
	 * @param Subsets
	 * @param id
	 * @return the needed subset if existed
	 *//*
	private Subset2 getSubset(Subset2[] subsets, String name) {
		for (Subset2 s: subsets) {
			if (s.getName().equals(name))
				return s;
		}
		return null;
	}

	*//**
	 * load the subsets to a certain dimension with a certain load mode
	 * @param dim OLAP dimension
	 * @param mode the load mode (only update and delete are allowed)
	 * @throws RuntimeException
	 *//*
	private void exportSubsets(Dimension dim, Modes mode) throws RuntimeException {

		SubsetHandler sh = dim.getSubsetHandler();

		// search the rules in the load, if they exist already in the cube then delete them
		if (mode.equals(Modes.DELETE)){
			log.info("Starting deleting rules: only rules' ids are here relevant.");
			for (DimensionSubset subset : subsets) {
				if(subset.name == null || subset.name.equals("")){
					log.warn("deleting rules requires the rule name.");
					continue;
				}


				Subset2 s = getSubset(sh.getSubsets(),subset.name);
				if (s != null)
					sh.remove(s);
				else
					log.warn("Subset with name " + subset.name + " does not exit and therefor can not be deleted.");
			}
		}
		else if(mode.equals(Modes.INSERT) || mode.equals(Modes.ADD) ){
			log.info("Starting inserting rules: rules' ids are only relevant if they exist already in the cube.");
			// search the rules in the load, if they exist already in the cube then delete them
			for (DimensionSubset subset : subsets) {

				//remove before add to avoid duplicate subset
				Subset2 s = getSubset(sh.getSubsets(),subset.name);
				if (s != null){
					sh.remove(s);
				}
				Subset2 newSubset = sh.addSubset(subset.name, Subset2.TYPE_GLOBAL);
				subset.definition = updateDefinition(newSubset.getId(), dim.getId(), subset.definition);
				newSubset.setDefinition(subset.definition);
				newSubset.save();
			}
		}
		else{
			log.info("Starting updating subsets.");

			log.info("deleting existing rules in dimension " + dim.getName());

			for(Subset2 s:sh.getSubsets()){
				sh.remove(s);
			}

			//add the new one
			log.info("Inserting the new subsets in dimension " +dim.getName());
			for (DimensionSubset subset : subsets) {

				Subset2 newSubset = sh.addSubset(subset.name, Subset2.TYPE_GLOBAL);
				subset.definition = updateDefinition(newSubset.getId(), dim.getId(), subset.definition);
				newSubset.setDefinition(subset.definition);
				newSubset.save();
			}

		}
	}

	*//**
	 * execute the load
	 *//*
	public void execute() {
		if (isExecutable()) {
			log.info("Starting load of Subsets into dimension: "+ dimensionName);
			try {
				Connection connection =  (org.palo.api.Connection)getConnection().open();
				Database d = connection.getDatabaseByName(getDatabaseName());

				Dimension c = d.getDimensionByName(dimensionName);
				if(c != null){
				exportSubsets(c,getMode());

//				if (doCommit)
//					c.commitLog();
				}
				else{
					throw new Exception("Dimension does not exist.");
				}
			}
			catch (Exception e) {
				log.error("Cannot load subsets into dimension "+ dimensionName+": "+e.getMessage());
				log.debug("",e);
			}
			log.info("Finished load of subsets into dimension "+ dimensionName+".");
		}
	}

	*//**
	 * read the rules from the processor and return them as an array list of rules
	 * @return array list of rules
	 * @throws RuntimeException
	 *//*
	public ArrayList<DimensionSubset> getSubsets() throws RuntimeException {

		ArrayList<DimensionSubset> subsets = new ArrayList<DimensionSubset>();
		IProcessor subsetsProcessesor = getProcessesor();
		Row r = subsetsProcessesor.current();

		while (subsetsProcessesor.next() != null) {

			    DimensionSubset subset = new DimensionSubset();
			    subset.name = r.getColumn(0).getValueAsString();
			    subset.definition = r.getColumn(1).getValueAsString();
			    subsets.add(subset);

		}
		return subsets;
	}

	*//**
	 * update the definition to contain the correct ids
	 * @param subsetId the id of the subset
	 * @param dimensionId id of the dimension
	 * @param oldDefinition old definition of the subset
	 * @return updated definition
	 * @throws RuntimeException
	 *//*
	private String updateDefinition(String subsetId,String dimensionId, String oldDefinition) throws RuntimeException{

		try{
			DocumentBuilderFactory factory =DocumentBuilderFactory.newInstance();
			DocumentBuilder db = factory.newDocumentBuilder();
			InputSource inStream = new InputSource();
			inStream.setCharacterStream(new StringReader(oldDefinition));
			Document doc = db.parse(inStream);
			Node n =  (doc.getElementsByTagName("subset")).item(0);
			n.getAttributes().getNamedItem("id").setNodeValue(subsetId);
			n.getAttributes().getNamedItem("sourceDimensionId").setNodeValue(dimensionId);
			Transformer transformer = TransformerFactory.newInstance().newTransformer();
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");
			StreamResult result = new StreamResult(new StringWriter());
			DOMSource source = new DOMSource(doc);
			transformer.transform(source, result);
			String xmlString = result.getWriter().toString();

			return xmlString;
		}catch(Exception e){
			throw new RuntimeException(e.getMessage());
		}
	}

	*//**
	 * get the processor that contains the subsets, it should contain exactly 2 columns
	 * @return the processor that contains the rules
	 *//*
	private IProcessor getProcessesor()
	{
		TableSource subsetsSource = null;
		try {
			subsetsSource = (TableSource)(getConfigurator().getSource());
		} catch (CreationException e) {
			log.error("Unable to read source table : " + e.getMessage());
			log.debug(e);
			return null;
		}


		try {
			return subsetsSource.getProcessor();
		} catch (RuntimeException e) {
			log.error("Unable to read read source table" + e.getMessage());
			log.debug(e);
			return null;
		}
	}


	*//**
	 * get the dimension name and the subsets from the source
	 *//*
	public void init() throws InitializationException {
		try {
			super.init();
			dimensionName = getConfigurator().getDimensionName();
			subsets = getSubsets();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}
*/}
