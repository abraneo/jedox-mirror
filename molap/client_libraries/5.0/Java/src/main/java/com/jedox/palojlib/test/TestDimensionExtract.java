package com.jedox.palojlib.test;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.util.HashMap;
import java.util.Properties;

import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionManager;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.*;

/**
 *
 */

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
 *	 Exclusive worldwide exploitation right (commercial copyright) has Jedox AG, Freiburg.
 *
 *   @author Kais Haddadin, Jedox AG, Freiburg, Germany
 */
public class TestDimensionExtract {

	/**
	 * @param args
	 */
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


			IConnection con;
			try {

			con = ConnectionManager.getInstance().getConnection(config);

			con.open();
			
			/*IElement[] users = con.getDatabaseByName("System").getDimensionByName("#_USER_").getElements(true);
			for(int i=0;i<users.length;i++){
				users[i].getAttributeValue("email");
			}*/

			IDatabase biker = con.getDatabaseByName("Demo");

			IDimension dims = con.getDatabaseByName("Biker").getDimensionByName("Measures");
			dims.setCacheTrustExpiry(20);
			IElement units = dims.getElementByName("Units", true);
			IElement sales2 = dims.getElementByName("Sales", false);
			dims.addBaseElement("kais", ElementType.ELEMENT_NUMERIC);
			IElement sales = dims.getElementByName("Sales", false);
			IElement cost = dims.getElementByName("Cost of Sales", true);
			HashMap<String, IElement[]> bla = (HashMap<String, IElement[]>) dims.getChildrenMap();
		
			
			
			IDimension dim = biker.getDimensionByName("Products");
			
			IDatabase biker1 = con.getDatabaseByName("Element_ETL");
			IDimension dim1 = biker1.getDimensionByName("numbers");
			IElement[] es = dim1.getElements(false);
			System.out.println(es.length);
			dim1.getElements(false);
			System.exit(0);
			
			IAttribute[] at = dim.getAttributes();

			IElement[] elements = dim.getElements(true);
			IElement child = dim.getElementByName("TOTAL ASSETS", true);
			IElement parent = dim.getElementByName("Balance Sheet", true);
			child.getWeight(parent);
			for(IElement e: elements){
				//System.out.print(e.getName() + ":");
					System.out.println(e.getName());
					System.out.print(e.getAttributeValue(at[0].getName()).toString());
					System.out.print(e.getAttributeValue(at[1].getName()).toString());
			}

			con.close();

			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

	}

}
