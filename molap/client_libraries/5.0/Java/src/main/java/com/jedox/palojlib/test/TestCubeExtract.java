package com.jedox.palojlib.test;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Properties;

import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionManager;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
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
public class TestCubeExtract {

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
			
			con.save();

			IDatabase demo = con.getDatabaseByName("Biker");
			ICube sales = demo.getCubeByName("Orders");
			long iddddddddd = sales.getCBToken();
			iddddddddd = sales.getCCToken();
			IDimension[] SalesDimensions = sales.getDimensions();
			/*
			IElement[] path = new IElement[SalesDimensions.length];
			path[0]=SalesDimensions[0].getElementByName("Desktop L",false);
			path[1]=SalesDimensions[1].getElementByName("Germany",false);
			path[2]=SalesDimensions[2].getElementByName("Jan",false);
			path[3]=SalesDimensions[3].getElementByName("2004",false);
			path[4]=SalesDimensions[4].getElementByName("Actual",false);
			path[5]=SalesDimensions[5].getElementByName("Units",false);
			ICell cell = sales.getCell(path);
			System.out.println("The value is " + cell.getValue());*/
			
			
			
			IElement[][] area2 = new IElement[SalesDimensions.length][];


			//area2[0] = new IElement[]{SalesDimensions[0].getElements(false)[0]};
			for(int i=0;i<SalesDimensions.length;i++){
				area2[i] = new IElement[]{SalesDimensions[i].getElements(false)[0],SalesDimensions[i].getElements(false)[1],SalesDimensions[i].getElements(false)[2]};;
			}

			ICellsExporter exporter = sales.getCellsExporter(area2,new CellExportContext(CellsExportType.BOTH, 5, false, false, false));

			ArrayList<ICell> cc = new ArrayList<ICell>();
			while(exporter.hasNext()){
				ICell c = exporter.next();
				cc.add(c);
				String[] names = c.getPathNames();
				//String[] names =c.getPathIds();
				int length = names.length;
				//for(int i=0;i<length;i++){
				//	System.out.print(names[i]+ ",");
				//}
				//System.out.println(c.getValue());
			}
			con.close();
			System.out.println("Finished with number of cells " + cc.size());
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}


	}

}
