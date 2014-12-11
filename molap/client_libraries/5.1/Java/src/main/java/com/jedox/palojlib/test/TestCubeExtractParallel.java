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

import java.util.ArrayList;
import com.jedox.palojlib.interfaces.ICell;
import com.jedox.palojlib.interfaces.ICellsExporter;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICube.CellsExportType;
import com.jedox.palojlib.main.*;

public class TestCubeExtractParallel {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		//TestSettings.getInstance().setDebugLevel();

		Runnable run1 = new Runnable() {

		
			public void run() {
				
				IConnectionConfiguration config = new ConnectionConfiguration();
				TestSettings.getInstance().setConfigFromFile(config);
				TestSettings.getInstance().setSSL();

				try {
					IConnection con = ConnectionManager.getInstance().getConnection(config);
					con.open();
					IDatabase demo = con.getDatabaseByName("Demo");
					ICube sales = demo.getCubeByName("Sales");
					IDimension[] dims = sales.getDimensions();
					IElement[][] area = new IElement[dims.length][];

					area[0] = new IElement[] { dims[0].getElements(false)[0] };
					for (int i = 1; i < dims.length-1; i++) {
						area[i] = new IElement[] {
								dims[i].getElements(false)[0],
								dims[i].getElements(false)[1]};
					}
					area[dims.length-1] = null;

					ICellsExporter exporter = sales.getCellsExporter(area,
							new CellExportContext(CellsExportType.BOTH, 5, false,
									true, true));

					ArrayList<ICell> cc = new ArrayList<ICell>();
					while (exporter.hasNext()) {
						ICell c = exporter.next();
						cc.add(c);
						String[] names = c.getPathNames();
						int length = names.length;
						for (int i = 0; i < length; i++) {
							System.out.print(names[i] + ",");
						}
						System.out.println(c.getValue());
					}
					con.close(true);
					System.out.println("Finished with number of cells " + cc.size());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		Runnable run2 = new Runnable() {

			public void run() {
				IConnectionConfiguration config = new ConnectionConfiguration();
				TestSettings.getInstance().setConfigFromFile(config);
				TestSettings.getInstance().setSSL();

				try {
					IConnection con = ConnectionManager.getInstance().getConnection(config);
					con.open();
					IDatabase demo = con.getDatabaseByName("Demo");
					ICube sales = demo.getCubeByName("Sales");
					IDimension[] dims = sales.getDimensions();
					IElement[][] area = new IElement[dims.length][];

					area[0] = new IElement[] { dims[0].getElements(false)[0] };
					for (int i = 1; i < dims.length-1; i++) {
						area[i] = new IElement[] {
								dims[i].getElements(false)[0],
								dims[i].getElements(false)[1]};
					}
					area[dims.length-1] = null;

					ICellsExporter exporter = sales.getCellsExporter(area,
							new CellExportContext(CellsExportType.BOTH, 5, false,
									true, true));

					ArrayList<ICell> cc = new ArrayList<ICell>();
					while (exporter.hasNext()) {
						ICell c = exporter.next();
						cc.add(c);
						String[] names = c.getPathNames();
						int length = names.length;
						for (int i = 0; i < length; i++) {
							System.out.print(names[i] + ",");
						}
						System.out.println(c.getValue());
					}
					con.close(true);
					System.out.println("Finished with number of cells " + cc.size());
				} catch (Exception e) {
					e.printStackTrace();
				}

			}
		};

		new Thread(run1).start();
		new Thread(run2).start();


	}

}
