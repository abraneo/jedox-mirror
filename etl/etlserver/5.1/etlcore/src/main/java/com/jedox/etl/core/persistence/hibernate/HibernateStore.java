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
* 	Portions of the code developed by proclos OG, Wien on behalf of Jedox AG.
* 	Intellectual property rights for these portions has proclos OG Wien, 
* 	or otherwise Jedox AG, Freiburg. Exclusive worldwide exploitation right 
* 	(commercial copyright) has Jedox AG, Freiburg.
*
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.persistence.hibernate;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.load.ILoad.Modes;
import com.jedox.etl.core.persistence.IStore;
import com.jedox.etl.core.persistence.PersistorDefinition;


public class HibernateStore implements IStore {
	
	private IStore delegateBackend;
	private PersistorDefinition definition;

	public HibernateStore(PersistorDefinition definition) throws RuntimeException {
		//check mode specific initializations
		this.definition = definition;
		switch (definition.getMode()) {
		case FILL: delegateBackend = new HibernatePersistor(definition); break;
		case TEMPORARY: delegateBackend = new HibernatePersistor(definition); break;
		case DELETE: delegateBackend = new HibernateEraser(definition); break;
		default: {
			if (definition.doAggregate()) {
				delegateBackend = new HibernateAggregator(definition);
			}
			else
				delegateBackend = new HibernatePersistor(definition);		
		}
		}
	}
	
	public void write() throws RuntimeException {
		delegateBackend.write();
	}

	public void commit() throws RuntimeException {
		delegateBackend.commit();
		//auto close on commit when not in mode TEMPORARY, which is closed explicitly by application, causing the table to be instantly dropped.
		if (!Modes.TEMPORARY.equals(definition.getMode())) {
			delegateBackend.close();
		}
	}

	public long getCurrentRowCount() throws RuntimeException {
		return delegateBackend.getCurrentRowCount();
	}

	public void close() throws RuntimeException {
		delegateBackend.close();
	}

}
