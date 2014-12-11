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
*   @author Andreas Fr�hlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.persistence.hibernate;

import javax.persistence.*;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

import org.hibernate.ejb.HibernateEntityManager;
import org.hibernate.Session;
import org.hibernate.Transaction;
import org.hibernate.criterion.Example;
import org.hibernate.criterion.Order;
import org.hibernate.Criteria;
import org.hibernate.ejb.Ejb3Configuration;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.connection.IRelationalConnection;


public class HibernateUtil {
	
	private static Log log = LogFactory.getLog(HibernateUtil.class);
	
	private static final String moduleName = "etlcore";
	private static final EntityManagerFactory factory = createFactory();
	private EntityManager manager;
	
	private static synchronized final Map<String,String> getConfiguration(IRelationalConnection connection) throws RuntimeException {
		Map<String,String> map = new HashMap<String,String>();
		map.put("hibernate.connection.driver_class", connection.getDriver());
		map.put("hibernate.connection.url", connection.getConnectionUrl());
		map.put("hibernate.connection.username", connection.getUser());
		map.put("hibernate.connection.password", connection.getPassword());
		return map;
	}
	
	private static synchronized final EntityManagerFactory createFactory() {
		if (isActive()) {
			try {
				//Cschw. ensure that file connection is present before passing to connection pool
				IRelationalConnection persistence = ConfigManager.getInstance().getPersistenceConnection();
				java.sql.Connection dbConnection = persistence.open();
				dbConnection.close();
				Map<String,String> properties = getConfiguration(ConfigManager.getInstance().getPersistenceConnection());
				new Ejb3Configuration().configure(moduleName,properties);
				return Persistence.createEntityManagerFactory(moduleName,properties);
			}
			catch (Exception e) {
				log.error(e.getMessage());
				e.printStackTrace();
			}
		}
		return null;
	}
	
	public static boolean isActive() {
		return Settings.getInstance().getContext(Settings.PersistenceCtx).getProperty("mode","client").equalsIgnoreCase("server");
	}
	
	public HibernateUtil() throws RuntimeException {
		if (factory != null) {
			manager = factory.createEntityManager();
		}
		else 
			throw new RuntimeException("Cannot create hibernate persistence instance. Factory building failed.");
	}
	
	/**
	 * @return The entity manager used to persist objects for this connection handle.
	 */
	public EntityManager getEntityManager() {
		return manager;
	}
	
	/**
	 * @return The Hibernate session of if the underlying entity manager.
	 */
	public Session getSession() {
		return ((HibernateEntityManager)manager).getSession();
	}
	
	/**
	 * Test whether the template entry exists.  The the supplied template implements IPersistable
	 * and contains a valid ID, it will be fetched with that ID and any other template
	 * values will be ignored.  Otherwise any entry matching the template will constitute
	 * the existence of the template.
	 * @param template
	 * @return Returns true if a matching entry has been persisted, otherwise returns false.
	 */
	public boolean exists(IPersistable template) {
		if (template.getId() == null) {
			Session session = getSession();
			Example example = Example.create(template);
			Criteria criteria = session.createCriteria(template.getClass());
			criteria.add(example);
			return !criteria.list().isEmpty();			
		} else {
			return get(template.getClass(), template.getId()) != null;
		}
	}
	
	
	/**
	 * Query the entity manager from the supplied string. 
	 * @param query
	 * @return
	 */
	public List<?> query(String query) {
		Session s = getSession();
		return s.createQuery(query).list();
	}
	
	/**
	 * Query the objects matching the templates.  All given attributes must match.  Any of the templates can constitute a match.
	 * @param templates An array of templates that should be matches.  All templates must be of the same class.
	 * @return
	 */
	public List<?> queryByExample(Object[] templates) {
		List<?> result = new ArrayList<Object>();
		if (templates != null && templates.length > 0) {
			Class<?> clazz = templates[0].getClass();
			Session session = getSession();
			Criteria criteria = session.createCriteria(clazz);
			for (Object template: templates) {
				Example example = Example.create(template);
				criteria.add(example);
			}
			criteria.addOrder(Order.desc("id"));
			result = criteria.list();
		}
		return result;
	}
	
	/**
	 * Retrieve a persistent object via it unique ID.
	 * @param clazz Class of the persisted object.
	 * @param id Unique ID.
	 * @return The persisted object or null if it was not found.
	 */
	public IPersistable get(Class<?> clazz, Long id) {
		EntityManager m = getEntityManager();
		IPersistable result = (IPersistable) m.find(clazz, id);
		return result;
	}
	
	/**
	 * Merges an object in the underlying persistence layer.
	 * @param entity
	 * @return The persisted object which now can be identified by ID.
	 */
	public IPersistable merge(IPersistable entity) throws RuntimeException {
		EntityManager m = getEntityManager();
		EntityTransaction tx = m.getTransaction(); 
		try {
			tx.begin(); 
			entity = m.merge(entity);
			tx.commit();
		}
		catch (Exception e) {
			if (tx.isActive())
				tx.rollback();
			log.error(e.getMessage());
			throw new RuntimeException(e);
		}
		return entity;
	} 
	
	/**
	 * Store / update an object in the underlying persistence layer.
	 * @param entity
	 * @return The persisted object which now can be identified by ID.
	 */
	public IPersistable saveOrUpdate(IPersistable entity) throws RuntimeException {
		Session s = getSession();
		Transaction tx = s.beginTransaction();
		try {
			tx.begin(); 
			s.saveOrUpdate(entity);
			tx.commit();
		}
		catch (Exception e) {
			if (tx.isActive())
				tx.rollback();
			log.error(e.getMessage());
			e.printStackTrace();
			throw new RuntimeException(e);
		}
		return entity;
	}
	
	
	/**
	 * Store an object in the underlying persistence layer.
	 * @param entity
	 * @return The persisted object which now can be identified by ID.
	 */
	public IPersistable save(IPersistable entity) throws RuntimeException {
		EntityManager m = getEntityManager();
		EntityTransaction tx = m.getTransaction(); 
		try {
			tx.begin(); 
			m.persist(entity); 
			tx.commit();
		}
		catch (Exception e) {
			if (tx.isActive())
				tx.rollback();
			log.error(e.getMessage());
			e.printStackTrace();
			throw new RuntimeException(e);
		}
		return entity;
	}
	
	public void flush() throws RuntimeException {
		EntityManager m = getEntityManager();
		EntityTransaction tx = m.getTransaction(); 
		try {
			tx.begin();
			m.flush();
			tx.commit();
		}
		catch (Exception e) {
			if (tx.isActive())
				tx.rollback();
			log.error(e.getMessage());
			throw new RuntimeException(e);
		}
	} 


	/**
	 * Remove an object in the underlying persistence layer.
	 * @param entity
	 * @return true if the removal was successful, otherwise return false.
	 */
	public boolean remove(IPersistable entity) throws RuntimeException {
		boolean returnValue = false;
		if (entity != null) {
			EntityManager m = getEntityManager();
			EntityTransaction tx = m.getTransaction(); 
			try {
				tx.begin(); 
				m.remove(entity);
				tx.commit();
				returnValue = true;
			}
			catch (Exception e) {
				e.printStackTrace();
				log.error(e.getMessage());
				if (tx.isActive())
					tx.rollback();
				throw new RuntimeException(e);
			}
		}
		return returnValue;
	}
	
	/**
	 * Closes the connection to the database and frees any resources associated with it.
	 */
	public static synchronized void shutdown() {
		factory.close();
	}
	
}
