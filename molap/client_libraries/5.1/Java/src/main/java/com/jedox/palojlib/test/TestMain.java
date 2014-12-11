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
 *   You may obtain a copy of the License at
*
 *   If you are developing and distributing open source applications under the
 *   GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 *   ISVs, and VARs who distribute Palo with their products, and do not license
 *   and distribute their source code under the GPL, Jedox provides a flexible
 *   OEM Commercial License.
 *
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */

package com.jedox.palojlib.test;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Properties;

import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionManager;
import com.jedox.palojlib.interfaces.IConsolidation;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IRule;
import com.jedox.palojlib.interfaces.ICube.CubeType;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.interfaces.ICube.SplashMode;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.*;

public class TestMain {

	/**
	 * @param args
	 */
	@SuppressWarnings("unused")
	public static void main(String[] args) {

		IConnectionManager manager = ConnectionManager.getInstance();
		IConnectionConfiguration config = new ConnectionConfiguration();
		config.setHost("localhost");
		config.setPort("7777");
		config.setUsername("admin");
		config.setPassword("admin");
		config.setTimeout(10000);
		config.setSslPreferred(true);

		// sets the keystore path
		try{
			 Properties props = new Properties();
			 File propFile = new File("ssl.properties");

				try {
					props.load(propFile.toURI().toURL().openStream());
				} catch (MalformedURLException e) {
					//log.debug("Try to open SSL connecion " + e.getMessage());
				} catch (IOException e) {
					//log.debug("Try to open SSL connecion " + e.getMessage());
				}

			 for (Object prop: props.keySet()) {
			     System.getProperties().setProperty((String)prop,props.getProperty((String)prop));
			 }
		}catch(Exception ioe){
			//log.debug("SSL properties file is not found under the expected folder," + ioe.getMessage());
		}

		try {
			IConnection con = ConnectionManager.getInstance().getConnection(config);
			System.out.println("before open ");
			con.open();

			System.out.println("after open ");

			//System.exit(0);
			IDatabase db = con.getDatabaseByName("demo");
			IDimension xx = db.addDimension("dd");
			IElement[] elements = db.getDimensionByName("Years").getElements(false);
			for(IElement e: elements){
				db.getDimensionByName("Years").getElementByName(e.getName(), true);
			}
			IDatabase[] dbs = con.getDatabases();
			con.close();
			dbs[0].getDimensions();
			for(int i=0;i<dbs.length;i++){
				System.out.println("db  name "+ dbs[i].getName());
			}

		/*	try {
				con.createDatabase("db_test");
			} catch (Exception e) {
				System.err.println(e.getMessage());
			}*/

			IDatabase demo = con.getDatabaseByName("Demo_ETL");
			IDimension dimm2 = demo.getDimensionByName("Regions");
			demo.removeDimension(dimm2);
			IDimension dimm = demo.getDimensionByName("products");
			dimm.getRootElements(true);
			IAttribute[] atttt = dimm.getAttributes();

			//Cube ccc = demo.createCube("test", new Dimension[]{dimm});
			IElement ee = dimm.getElementByName("Monitors", true);
			//HashMap<String, Element[]> map= ee.getSubTree();
			//HashMap<String, HashMap<String, Object>> map2= ee.getSubTreeAttributes();

			//Rule[] ruless = demo.getCubeByName("Sales").getRules();

			//Attribute att = dimm.getAttributeByName("Price Per Unit");
			//System.out.println("wait");
			//att = dimm.getAttributeByName("Price Per Unit");
			//dimm.deleteElements(new String[]{"2013kais"});
			IElement[] roots= dimm.getRootElements(true);


			IElement[] ess= dimm.getElements(true);
			IAttribute[] attss = dimm.getAttributes();
			for(IElement en: ess){

				for(int i=0;i<attss.length;i++){
					String attributeName = attss[i].getName();
					System.out.println("the name is :" + en.getName() + " with " +  attributeName + "=" + en.getAttributeValue(attributeName) );
				}
			}

			ICube sales = demo.getCubeByName("Orders2");
			CubeType type = sales.getType();
			//long number = sales.getNumberOfFilledCells();
			IDimension[] SalesDimensions = sales.getDimensions();
			Element[][] area2 = new Element[SalesDimensions.length][];
			/*for(int i=0;i<2;i++){
				area2[i] = new Element[]{SalesDimensions[i].getElements(false)[0]};
			}*/
			for(int i=0;i<2;i++){
				area2[i] =null;
			}
			for(int i=2;i<3;i++){
				area2[i] = null;
			}
			/*for(int i=3;i<SalesDimensions.length;i++){
				area2[i] = new Element[]{SalesDimensions[i].getElements(false)[0]};
			}*/
			for(int i=3;i<SalesDimensions.length;i++){
				area2[i] = null;
			}
			/*for(int i=3;i<SalesDimensions.length;i++){
				area2[i] = new Element[]{SalesDimensions[i].getElements(false)[0]};
			}*/

			//Cell[] cells = demo.getCubeByName("Sales").extractCells(area2,ExportType.BOTH, 1000, false, false, true);
			ICellsExporter exporter = demo.getCubeByName("Orders2").getCellsExporter(area2,new CellExportContext(CellsExportType.BOTH, 10000, false, true, true));

			ArrayList<ICell> cc = new ArrayList<ICell>();
			while(exporter.hasNext()){
				ICell c = exporter.next();
				cc.add(c);
				//String[] names = c.getPathNames().clone();
				//for(int i=0;i<length;i++){
				//	System.out.print(names[i]+ ",");
				//}
				//System.out.println(c.getValue());
			}

			System.out.println("Finished");

			IDimension d1 = demo.getDimensionByName("Products");
			IDimension d2 = demo.getDimensionByName("Regions");
			IDimension d3 = demo.getDimensionByName("Months");
			IDimension d4 = demo.getDimensionByName("Years");
			IDimension d5 = demo.getDimensionByName("Datatypes");
			IDimension d6 = demo.getDimensionByName("Measures");

			IElement d1e= d1.getElementByName("Desktop L", false);
			IElement d2e= d2.getElementByName("Germany", false);
			IElement d3e1= d3.getElementByName("Jan", false);
			IElement d3e2= d3.getElementByName("Feb", false);
			IElement d3e3= d3.getElementByName("Mar", false);
			IElement d3e4= d3.getElementByName("Apr", false);
			IElement d4e= d4.getElementByName("2009", false);
			IElement d5e= d5.getElementByName("Actual", false);
			IElement d6e= d6.getElementByName("Units", false);

			IElement[][] paths ={{d1e,d2e,d3e1,d4e,d5e,d6e},
					{d1e,d2e,d3e2,d4e,d5e,d6e},
					{d1e,d2e,d3e3,d4e,d5e,d6e},
					{d1e,d2e,d3e4,d4e,d5e,d6e}};

			IElement[][] area = {{d1e},{d2e},{d3e1,d3e2,d3e3,d3e4},{d4e},{d5e},{d6e}};

			Object[] values = {1,2,3,4};
			IRule[] rules = demo.getCubeByName("Sales").getRules();
			//d1.addAttributes(new String[]{"thirdName"},new ElementType[]{ElementType.ELEMENT_NUMERIC});
			IAttribute att = d1.getAttributeByName("thirdName");
			d1.addAttributeValues(att, new IElement[]{d1.getElements(false)[0],d1.getElements(false)[1],d1.getElements(false)[2]}, new Object[]{"33","44","1"});
			d1.removeAttributeValues(att, new IElement[]{d1.getElements(false)[1]});
			demo.getCubeByName("Sales").clearCells(area);
			demo.getCubeByName("Sales").loadCells(paths, values, new CellLoadContext(SplashMode.SPLASH_DEFAULT,3, false,true),null);

			IDimension dim = demo.getDimensionByName("Products");


			IElement[] es= dim.getElements(false);

			ElementType[] types = { ElementType.ELEMENT_NUMERIC, ElementType.ELEMENT_STRING};
			//dim.addElements(new String[]{"newElement1","newElement2"}, types);

			//dim.addAttributes(new String[]{"Attribute1","Attribute2"}, types);



			IElement parent1= dim.getElementByName("All", false);
			IElement parent2= dim.getElementByName("2005", false);
			IElement child1= dim.getElementByName("2009", false);
			IElement child2= dim.getElementByName("2010", false);
			IElement child3= dim.getElementByName("2004", false);
			IConsolidation cons1 = dim.newConsolidation(parent1, child1, 0.2);
			IConsolidation cons2 = dim.newConsolidation(parent1, child2, 0.3);
			IConsolidation cons3 = dim.newConsolidation(parent2, child3, 0.4);
			//Consolidation [] cons = {cons1,cons2,cons3};
			//dim.updateConsolidations(cons);


			IElement e2e = dim.getElementByName("Subnote SL",true);
			IElement[] parents = e2e.getParents();
			System.out.println("The name is :" + e2e.getName());
			for(int jj=0;jj<parents.length;jj++){
				System.out.println("has parent :" + parents[jj].getName());
			}

			System.out.println("Starting .........................");

			IAttribute[] atts = dim.getAttributes();
			es= dim.getElements(true);
			dim.addAttributeConsolidation(atts[0], atts[1]);
			dim.removeAttributeConsolidations(atts[0]);
			for(IElement en: es){

				for(int i=0;i<atts.length;i++){
					String attributeName = atts[i].getName();
					System.out.println("the name is :" + en.getName() + " with " +  attributeName + "=" + en.getAttributeValue(attributeName) );
				}
			}
			con.close();

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}

}
