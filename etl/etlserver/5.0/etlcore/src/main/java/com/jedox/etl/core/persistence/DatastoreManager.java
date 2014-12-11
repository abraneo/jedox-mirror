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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.persistence;

import java.util.Hashtable;
import java.util.List;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.persistence.hibernate.HibernateUtil;

/**
 * Manager class for {@link Datastore Datastores}.
 * Enables to persist and retrieve Datastore definitions from disk.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */
public class DatastoreManager {

	private static DatastoreManager instance = null;
	private Hashtable<String,Datastore> lookup = new Hashtable<String,Datastore>();
	
	private String getKey(String key) {
		return key.toUpperCase();
	}
	
	DatastoreManager() throws RuntimeException {
		try {
		if (HibernateUtil.isActive()) {
			List<?> stores = new HibernateUtil().query("from Datastore"); 
			for (Object s : stores) { 
				Datastore store = (Datastore) s; 
				put(store.getLocator(), store);
			} 
		}
		}
		catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException(e);
		}
	}
	
	/**
	 * gets this singleton manager instance
	 * @return the manager instance
	 */
	public static final synchronized DatastoreManager getInstance() throws RuntimeException {
		if (instance == null) instance = new DatastoreManager();
		return instance;
	}
	
	private void save(Datastore store) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			new HibernateUtil().saveOrUpdate(store); 
		}
	}
	
	/**
	 * Adds a Datastore to this manager and persists it 
	 * @param store
	 */
	public void add(Datastore store) throws RuntimeException {
		save(store);
		put(store.getLocator(), store);
	}
	
	/**
	 * Gets an existing Datastore from this manager
	 * @param locator
	 * @return
	 */
	public Datastore get(String locator) {
		return lookup.get(getKey(locator));
	}
	

	private void put(String key, Datastore store) {
		lookup.put(getKey(key), store);
	}
	
	/**
	 * gets the keys / ids from all Datastores managed.
	 * @return
	 */
	public String[] getKeys() {
		return lookup.keySet().toArray(new String[lookup.keySet().size()]);
	}
	
	public synchronized Datastore provide(PersistorDefinition definition) throws RuntimeException {
		return provide(definition,false);
	}
		
	/**
	 * provides a Datastore. If the Datastore already exists, its persistent properties are updated with the given definition, else a new Datastore conforming to the given definition is created.
	 * @param definition the definition of the Datastore
	 * @return the Datastore requested.
	 * @throws RuntimeException 
	 */
	public synchronized Datastore provide(PersistorDefinition definition, boolean writeable) throws RuntimeException {
		Datastore d = get(definition.getId());
		//it does not exist. create it
		if (d == null) {
			d = new Datastore(definition);
		}
		//it exists but maybe has different definition settings. update it.
		else {
			d.checkChanges(definition);
			d.setDefinition(definition);
		}
		if (writeable && definition.getDirectSource() == null) d.setWriteable();
		switch (definition.getMode()) {
		case TEMPORARY: break;
		default: add(d);
		}
		return d;
	}

	public void remove(Datastore store) throws RuntimeException {
		if(lookup.get(getKey(store.getLocator()))!= null){
			lookup.remove(getKey(store.getLocator()));
			if (HibernateUtil.isActive()) {
				HibernateUtil util = new HibernateUtil();
				util.remove(util.get(Datastore.class, store.getId()));
			}
		}
	}
	
}
