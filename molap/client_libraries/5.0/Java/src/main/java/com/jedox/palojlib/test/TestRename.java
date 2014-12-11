package com.jedox.palojlib.test;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Properties;
import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionManager;
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
public class TestRename {

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

			//System.exit(0);
			IDatabase[] dbs = con.getDatabases();
			for(int i=0;i<dbs.length;i++){
				System.out.println("db  name "+ dbs[i].getName());
			}


			IDatabase demo = con.getDatabaseByName("Biker_ETL");
			demo.rename("Demo10");
			System.out.println(demo.getName());
			
			dbs = con.getDatabases();
			for(int i=0;i<dbs.length;i++){
				System.out.println("db  name "+ dbs[i].getName());
			}
			
			demo.rename("Biker_ETL");
			
			
			IDimension dimm2 = demo.getDimensionByName("Years");
			IDimension[] dims = demo.getDimensions();
			for(int i=0;i<dims.length;i++){
				System.out.println("dimension name "+ dims[i].getName());
			}
			dimm2.rename("dim_new");
			
			dims = demo.getDimensions();
			
			for(int i=0;i<dims.length;i++){
				System.out.println("dimension name "+ dims[i].getName());
			}
			
			dimm2.rename("Years");
			
			IElement e = dimm2.getElementByName("2010", true);
			e.rename("newname");
			
			IElement e2 = dimm2.getElementByName("2010", true);
			if(e2!=null){
				System.err.print("Somethings is wrong");
			}
			e.rename("2010");
			
			IAttribute att = dimm2.getAttributeByName("Name");
			att.rename("xx");
			
			IAttribute[] atts = dimm2.getAttributes();
			for(int i=0;i<atts.length;i++){
				System.out.println("attributes name "+ atts[i].getName());
			}
			att.rename("Name");
			
			con.close();

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}

}
