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

import com.jedox.palojlib.interfaces.IAttribute;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.interfaces.IElement.ElementType;
import com.jedox.palojlib.main.*;

public class TestRename {

	public static void main(String[] args) {

		IConnectionConfiguration config = new ConnectionConfiguration();
		TestSettings.getInstance().setConfigFromFile(config);
		TestSettings.getInstance().setSSL();
		//TestSettings.getInstance().setDebugLevel();

		try {
			IConnection con = ConnectionManager.getInstance().getConnection(config);
			con.open();
			
			IDatabase db = con.getDatabaseByName("Demo");
			db.rename("DemoNewName");
			
			IDatabase[] dbs = con.getDatabases();
			for(int i=0;i<dbs.length;i++){
				System.out.println("db  name "+ dbs[i].getName());
			}
			
			db.rename("Demo");
			dbs = con.getDatabases();
			for(int i=0;i<dbs.length;i++){
				System.out.println("db  name "+ dbs[i].getName());
			}
			
			
			IDimension dim = db.getDimensionByName("Years");
			IDimension[] dims = db.getDimensions();
			for(int i=0;i<dims.length;i++){
				System.out.println("dimension name "+ dims[i].getName());
			}
			dim.rename("YearsNewName");
			
			dims = db.getDimensions();
			
			for(int i=0;i<dims.length;i++){
				System.out.println("dimension name "+ dims[i].getName());
			}
			
			dim.rename("Years");
			
			IElement e = dim.getElementByName("2015", true);
			e.rename("2015New");
			
			dim.resetCache();
			IElement e2 = dim.getElementByName("2015New", true);
			
			if(e2==null){
				System.err.print("Somethings is wrong");
			}
			e.rename("2015");
			
			dim.addAttributes(new String[]{"att1"}, new IElement.ElementType[]{ElementType.ELEMENT_STRING});
			IAttribute att = dim.getAttributeByName("att1");
			if(att==null){
				System.err.print("Somethings is wrong");
			}
			att.rename("att1New");
			
			IAttribute[] atts = dim.getAttributes();
			for(int i=0;i<atts.length;i++){
				System.out.println("attributes name "+ atts[i].getName());
			}
			att.rename("att1");
			
			con.close(false);

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}

}
