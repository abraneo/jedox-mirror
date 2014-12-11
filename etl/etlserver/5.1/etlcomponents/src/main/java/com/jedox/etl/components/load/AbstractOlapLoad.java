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
*   Developed by proclos OG, Wien on behalf of Jedox AG. Intellectual property
*   rights has proclos OG, Wien. Exclusive worldwide exploitation right
*   (commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Gerhard Weis, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.components.load;

import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.connection.IConnection;
import com.jedox.etl.core.connection.IOLAPConnection;
import com.jedox.etl.core.load.Load;
import com.jedox.palojlib.interfaces.ICube;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IRule;

public abstract class AbstractOlapLoad extends Load {

	private static final Log log = LogFactory.getLog(AbstractOlapLoad.class);

	public IOLAPConnection getConnection() throws RuntimeException {
		IConnection connection = super.getConnection();
		if ((connection != null) && (connection instanceof IOLAPConnection))
			return (IOLAPConnection) connection;
		throw new RuntimeException("OLAP connection is needed for source "+getName()+".");
	}


	protected String getDatabaseName() throws RuntimeException {
		return getConnection().getDatabase();
	}

	protected IDatabase getDatabase() throws RuntimeException {
		return getConnection().getDatabase(false,true);
	}
	
	protected IDatabase getDatabase(boolean throwExceptionIfNotExists) throws RuntimeException {
		return getConnection().getDatabase(false,throwExceptionIfNotExists);
	}
	
	protected void reactivateRules(ICube c, ArrayList<IRule> deactivatedRules) throws RuntimeException {
		if(deactivatedRules.size()!=0){
			c.activateRules(deactivatedRules.toArray(new IRule[0]));
			log.info("Cube " + c.getName()+ " rules activated again after loading: "+deactivatedRules.size());			
		}
	}
	
	protected ArrayList<IRule> deactivateRules(ICube c) throws RuntimeException {
		ArrayList<IRule> deactivatedRules = new ArrayList<IRule>();
			IRule[] rules = c.getRules();
				for(IRule r:rules){
					if(r.isActive()) {
							deactivatedRules.add(r);
							//if (isOldRulesAPI)
							//	c.updateRule(r.getIdentifier(), r.getDefinition(), false, r.getExternalIdentifier(),r.getComment());
							log.debug("Cube " + c.getName()+ " rule " + r.getIdentifier() + " will be deactivated before load.");
						}
					}
				if(deactivatedRules.size()!=0){
					c.deactivateRules(deactivatedRules.toArray(new IRule[0]));
					log.info("Cube " + c.getName()+ " rules deactivated before loading: "+deactivatedRules.size());
				}
			
		return deactivatedRules;
	}


	
	public void init() throws InitializationException {
		try {
			super.init();
		}
		catch (Exception e) {
			throw new InitializationException(e);
		}
	}

}
