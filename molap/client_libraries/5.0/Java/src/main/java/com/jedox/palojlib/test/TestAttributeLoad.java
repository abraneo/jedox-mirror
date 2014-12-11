package com.jedox.palojlib.test;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.util.Properties;

import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionManager;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
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
public class TestAttributeLoad {

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

		try {
			IConnection con = ConnectionManager.getInstance().getConnection(config);
			System.out.println("before open ");
			con.open();

			System.out.println("after open ");


			IDatabase demo = con.getDatabaseByName("Biker2");
			IDimension d1 = demo.getDimensionByName("Balance_Sheet");
			IAttribute[] att = d1.getAttributes();
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

			//IAttribute att = d1.getAttributeByName("Price Per Unit");
			//d1.addAttributeValues(att, new Element[]{(Element) d1.getElements(false)[0],(Element) d1.getElements(false)[1],(Element) d1.getElements(false)[2]}, new Object[]{"33","","1"});

			con.close();

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}

}
