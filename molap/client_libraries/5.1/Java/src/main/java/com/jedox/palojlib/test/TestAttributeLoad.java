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
import com.jedox.palojlib.main.*;

public class TestAttributeLoad {

	public static void main(String[] args) {

		IConnectionConfiguration config = new ConnectionConfiguration();
		TestSettings.getInstance().setConfigFromFile(config);
		TestSettings.getInstance().setSSL();
		//TestSettings.getInstance().setDebugLevel();

		try {

			IConnection con = ConnectionManager.getInstance().getConnection(config);
			con.open();
			IDatabase biker = con.getDatabaseByName("Biker");
			IDimension productsDim = biker.getDimensionByName("Products");
			IAttribute att = productsDim.getAttributeByName("deutsch");
			IElement e1 = productsDim.getElementByName("Off-Road-100 Blue 38", true);
			System.out.println("Before: " + e1.getAttributeValue(att.getName()));
			productsDim.addAttributeValues(att, new IElement[]{e1}, new Object[]{"newAttValue"});
			System.out.println("After: " + e1.getAttributeValue(att.getName()));
			con.close(true);

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


	}

}
