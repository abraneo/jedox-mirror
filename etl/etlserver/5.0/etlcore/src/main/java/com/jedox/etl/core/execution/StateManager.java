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
package com.jedox.etl.core.execution;

import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.StringTokenizer;

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import org.hibernate.Session;
import org.hibernate.Criteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Restrictions;
import org.hibernate.criterion.Disjunction;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.persistence.hibernate.HibernateUtil;
import com.jedox.etl.core.component.RuntimeException;

/**
 * Persistence manager class for {@link ExecutionState} objects.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

public class StateManager {

	private static StateManager instance = null;
	private static final Log log = LogFactory.getLog(StateManager.class);
	private long counter = 0;

	StateManager() throws RuntimeException {
		//do some cleanup of left over states
		if (HibernateUtil.isActive()) {
			List<ExecutionState> leftovers = getResults(null,null,null,null,null,"0 5");
			if (leftovers.size() > 0) {
				EntityManager m = new HibernateUtil().getEntityManager();
				EntityTransaction tx = m.getTransaction();
				tx.begin();
				for (ExecutionState l : leftovers) {
					if (Codes.statusQueued.equals(l.getStatus()))
						l.setStatus(Codes.statusAborted);
					else
						l.setStatus(Codes.statusFailed);
					m.merge(l);
				}
				tx.commit();
			}
		}
	}

	/**
	 * gets a singleton manager instance
	 * @return the StateManager instance.
	 */
	public static synchronized final StateManager getInstance() throws RuntimeException {
		if (instance == null) {
			instance = new StateManager();
		}
		return instance;
	}

	/**
	 * gets all {@link Message} objects of an {@link Execution} representing all log entries produced by components involved in the executions.
	 * @param executionId the unique id of the execution
	 * @return a list of messages.
	 */
	@SuppressWarnings("unchecked")
	public List<Message> getMessages(Long resultId) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			List<Message> messages = (List<Message>) new HibernateUtil().query("from Message m where resultId = "+resultId+" order by timestamp");
			return messages;
		}
		return new ArrayList<Message>();
	}

	/**
	 * persists an {@link ExecutionState}. On the initial persistence the unique id of the state (and thus of the execution) is created.
	 * @param state the ExecutionState to persist
	 */
	public void save(ExecutionState state) {
		if (HibernateUtil.isActive() && state.isArchive()) {
			try {
				HibernateUtil util = new HibernateUtil();
				util.saveOrUpdate(state);
				//only save messages when state indicates that there are messages.
				switch (state.getStatus()) {
				case statusQueued: break;
				case statusRunning: break;
				case statusInvalid: break;
				case statusAborted: break;
				default: {
					synchronized(state.getMessages()) {
						Iterator<Message> i = state.getMessages().iterator(); // Must be in synchronized block
						while (i.hasNext()) {
							try {
								Message m = i.next();
								if (m.getId() == null)
									util.save(m);
							}
							catch (RuntimeException e) {
								log.warn("Log message of execution "+state.getName()+" has not been persisted: "+e.getMessage());
							}
						}
					}
				}
				}
			}
			catch (RuntimeException e) {
				log.warn("State with id "+state.getId()+" of execution "+state.getName()+ "has not been persisted: "+e.getMessage());
			}
		}
		else {//give internal ad hoc id to execution
			if (state.getId() == null)
				state.setId(counter++);
		}
	}

	/**
	 * removes an {@link ExecutionState} from the persistence.
	 * @param state the state to remove
	 */
	public void remove(ExecutionState state) throws RuntimeException {
		if (HibernateUtil.isActive() && state.isArchive()) {
			HibernateUtil util = new HibernateUtil();
			util.remove(util.get(ExecutionState.class, state.getId()));
		}
	}

	private void addRestriction(Criteria c, String name, String value) {
		if (value != null && !value.isEmpty()) {
			if (value.contains("*")) {
				value = value.replace("*", "%");
				c.add(Restrictions.like(name, value));
			}
			else {
				c.add(Restrictions.eq(name, value));
			}
		}
	}

	private void addStatus(Criteria c, String value) {
		if (value != null && !value.isEmpty()) {
			ResultCodes codes = new ResultCodes();
			StringTokenizer t = new StringTokenizer(value," ,");
			Disjunction d = Restrictions.disjunction();
			while (t.hasMoreTokens()) {
				d.add(Restrictions.eq("status", codes.getCode(t.nextToken())));
			}
			c.add(d);
		}
	}

	/**
	 * gets a list of {@link ExecutionState} objects complying to the given filter criteria
	 * @param project only executions, where {@link IExecutable Executable} belongs to this project.
	 * @param type only executions with this type of {@link IExecutable Executable} executed by the execution (e.g "job", "load", etc.)
	 * @param name only executions with this name of the {@link IExecutable Executable} executed by the execution
	 * @param start only executions with {@link ExecutionState#getStartDate()} after this date
	 * @param stop only executions with {@link ExecutionState#getStopDate()} before this date
	 * @param status on only executions with one of these stati. {@link ResultCodes Stati} are given in their numeric representation separated by space or comma.
	 * @return the list of ExecutionStates
	 */
	@SuppressWarnings("unchecked")
	public List<ExecutionState> getResults(String project, String type, String name, Date start, Date stop, String status) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			Session s = new HibernateUtil().getSession();
			Criteria c = s.createCriteria(ExecutionState.class);
			addRestriction(c,"project",project);
			addRestriction(c,"type",type);
			addRestriction(c,"name",name);
			if (start != null) c.add(Restrictions.ge("startDate",start));
			if (stop != null) c.add(Restrictions.le("stopDate",stop));
			addStatus(c,status);
			c.addOrder(Order.asc("startDate"));
			List<ExecutionState> results = c.list();
			return results;
		}
		else {
			log.warn("Persistence of Results is not active.");
			return new ArrayList<ExecutionState>();
		}
	}

	/**
	 * gets a single {@link ExecutionState} by its id.
	 * @param id the unique id of the state to get
	 * @return the execution state from the persistence or null if no such state exists.
	 */
	public ExecutionState getResult(Long id) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			return (ExecutionState) new HibernateUtil().get(ExecutionState.class, id);
		}
		return null;
	}

}
