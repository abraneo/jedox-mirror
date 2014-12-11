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

import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.ICube.SplashMode;
import com.jedox.palojlib.main.*;

public class TestCubeLoad {

	public static void main(String[] args) {

		IConnectionConfiguration config = new ConnectionConfiguration();
		TestSettings.getInstance().setConfigFromFile(config);
		TestSettings.getInstance().setSSL();
		TestSettings.getInstance().setDebugLevel();

		try {
			IConnection con = ConnectionManager.getInstance().getConnection(config);
			con.open();

			IDatabase demo = con.getDatabaseByName("Demo");
			ICube cube = demo.getCubeByName("NewCube");
			if(cube!=null)
				demo.removeCube(cube);
			IDimension[] dims = new IDimension[]{demo.getDimensionByName("Months"),demo.getDimensionByName("Years")};
			cube = demo.addCube("NewCube", dims);
			int lockId = cube.lockComplete();
			trytoWrite(cube);
			cube.commitLock(lockId);
			trytoWrite(cube);

			
			con.close(true);

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}

	private static void trytoWrite(ICube cube) {
		IDimension d1 = cube.getDimensionByName("Months");
		IDimension d2 = cube.getDimensionByName("Years");

		IElement d1e1= d1.getElementByName("Jan", false);
		IElement d1e2= d1.getElementByName("Feb", false);
		IElement d1e3= d1.getElementByName("Mar", false);
		IElement d2e1= d2.getElementByName("2014", false);
		IElement d2e2= d2.getElementByName("2015", false);
		
		IElement[][] paths ={{d1e1,d2e1},{d1e2,d2e1},{d1e3,d2e1},{d1e1,d2e2},{d1e2,d2e2},{d1e3,d2e2}};
		Object[] values = {1,0,4,2,3,44};
		try {
			cube.loadCells(paths, values, new CellLoadContext(SplashMode.SPLASH_DEFAULT, 3, false, true),null);
		} catch (PaloException e) {
			e.printStackTrace();
		}
	
	}
	
	

}
