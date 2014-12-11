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

package com.jedox.palojlib.main;

import java.util.Random;

import org.apache.log4j.Logger;
import com.jedox.palojlib.managers.LoggerManager;
import com.jedox.palojlib.exceptions.PaloException;
import com.jedox.palojlib.exceptions.PaloJException;
import com.jedox.palojlib.http.HttpHandler;
import com.jedox.palojlib.interfaces.IConnection;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IConnectionManager;

public class ConnectionManager implements IConnectionManager{

	private static Logger log = LoggerManager.getInstance().getLogger(HttpHandler.class.getSimpleName());
	private static ConnectionManager manager;
	private static Random generator = new Random();
	
	private ConnectionManager(){}
	
	public static ConnectionManager getInstance(){
		if(manager!= null)
			return manager;
		
		manager = new ConnectionManager();
		return manager;
	}

	public synchronized IConnection getConnection(IConnectionConfiguration config) throws PaloException, PaloJException{

		
		((ConnectionConfiguration)config).contextId = ((ConnectionConfiguration)config).contextId + generator.nextInt();
		IConnection con = new Connection(config);
		log.debug("New Connection is created with id: " + ((ConnectionConfiguration)config).getContextId());
		return con;

	}

}
