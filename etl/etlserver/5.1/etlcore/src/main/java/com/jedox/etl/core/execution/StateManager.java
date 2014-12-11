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
package com.jedox.etl.core.execution;

import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.ArrayList;
import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;
import org.hibernate.Session;
import org.hibernate.Criteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.hibernate.criterion.Disjunction;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Level;

import com.jedox.etl.core.execution.Execution.ExecutionTypes;
import com.jedox.etl.core.execution.ResultCodes.Codes;
import com.jedox.etl.core.persistence.hibernate.HibernateUtil;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;

/**
 * Persistence manager class for {@link ExecutionState} objects.
 * @author Christian Schwarzinger. Mail: christian.schwarzinger@proclos.com
 *
 */

public class StateManager {

	private static StateManager instance = null;
	private static final Log log = LogFactory.getLog(StateManager.class);
	private long counter = 0;
	
	public static enum SortDirection {
		asc, desc
	}	

	StateManager() throws RuntimeException {
		//do some cleanup
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				EntityManager m = util.getEntityManager();
				//do removal of old executions
				Date standardRemovalDate = calculateRemovalDate(Settings.getInstance().getContext(Settings.ExecutionsCtx).getProperty("removeOldStandard"));
				Date otherRemovalDate = calculateRemovalDate(Settings.getInstance().getContext(Settings.ExecutionsCtx).getProperty("removeOldOthers"));
				if (standardRemovalDate != null) { //do removal of outdated STANDARD executions
					EntityTransaction tx = m.getTransaction();
					try {
						tx.begin();
						m.createNativeQuery("create temp table standardtemp as select executionstate.RESULT_ID from executionstate where executiontype = :etype and startdate < :date").setParameter("etype", ExecutionTypes.STANDARD.ordinal()).setParameter("date", standardRemovalDate).executeUpdate();
						int messages = m.createNativeQuery("delete from message where resultId in (select RESULT_ID from standardtemp)").executeUpdate();
						int details = m.createNativeQuery("delete from executiondetail where resultId in (select RESULT_ID from standardtemp)").executeUpdate();
						int states = m.createNativeQuery("delete from executionstate where RESULT_ID in (select RESULT_ID from standardtemp)").executeUpdate();
						m.createNativeQuery("drop table standardtemp");
						log.info("Removed "+states+" outdated standard execution states holding "+messages+" messages and "+details+" details.");
						tx.commit();
					}
					catch (Exception e) {
						log.warn("Deletion of outdated standard execution states failed: "+e.getMessage());
						e.printStackTrace();
						tx.rollback();
					}
				}
				if (otherRemovalDate != null) {//do removal of ALL Other executions
					EntityTransaction tx = m.getTransaction();
					try {
						tx.begin();
						m.createNativeQuery("create temp table othertemp as select executionstate.RESULT_ID from executionstate where executiontype != :etype and startdate < :date").setParameter("etype", ExecutionTypes.STANDARD.ordinal()).setParameter("date", otherRemovalDate).executeUpdate();
						int messages = m.createNativeQuery("delete from message where resultId in (select RESULT_ID from othertemp)").executeUpdate();
						int details = m.createNativeQuery("delete from executiondetail where resultId in (select RESULT_ID from othertemp)").executeUpdate();
						int states = m.createNativeQuery("delete from executionstate where RESULT_ID in (select RESULT_ID from othertemp)").executeUpdate();
						m.createNativeQuery("drop table othertemp");
						log.info("Removed "+states+" outdated non-standard execution states holding "+messages+" messages and "+details+" details.");
						tx.commit();
					}
					catch (Exception e) {
						log.warn("Deletion of outdated non-standard execution states failed: "+e.getMessage());
						e.printStackTrace();
						tx.rollback();
					}
				}
				//do some cleanup of left over states
				List<ExecutionState> leftovers = getResults(null,null,null,null,null,new String[]{"0","5"},null,0,0,null);
				if (leftovers.size() > 0) {
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
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}	
		}
	}
	
	private Date calculateRemovalDate(String setting) {
		try {
			int value = Integer.parseInt(setting);
			if (value == -1) return null;
			Calendar calendar = Calendar.getInstance();
			calendar.add(Calendar.HOUR_OF_DAY, -value);
			return calendar.getTime();
		}
		catch (Exception e) {
			log.warn("Calculation of obsoleted state removal date failed. Setting is "+setting);
			return null;
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
	
	public ExecutionState provideState(ExecutionTypes type, Locator locator) throws RuntimeException {
		ExecutionState state = new ExecutionState(locator);
		state.setExecutionType(type);
		state.setArchive(save(state));
		state.init();
		return state;
	}

	/**
	 * gets all {@link Message} objects of an {@link Execution} representing all log entries produced by components involved in the executions.
	 * @param executionId the unique id of the execution
	 * @return a list of messages.
	 */
	@SuppressWarnings("unchecked")
	public List<Message> getMessages(Long resultId) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				List<Message> messages = (List<Message>) util.query("from Message m where resultId = "+resultId+" order by timestamp");
				return messages;
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}
		}
		return new ArrayList<Message>();
	}
	
	
	public void saveMessages(Collection<Message> messages) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				util.saveCollection(messages);
			}
			catch (RuntimeException e) {
				log.warn("Messages have not been persisted: "+e.getMessage());
			}
			finally {
				util.close();
			}
		}
	}
	
	public void persistDetails(Collection<ExecutionDetail> details, Boolean persistDetails) throws RuntimeException {
		if (HibernateUtil.isActive() && persistDetails) {
			HibernateUtil util = new HibernateUtil();
			try {
				util.saveCollection(details);
			}
			catch (RuntimeException e) {
				log.warn("Details have not been persisted: "+e.getMessage());
			}
			finally {
				util.close();
			}
		}
	}

	/**
	 * persists an {@link ExecutionState}. On the initial persistence the unique id of the state (and thus of the execution) is created.
	 * @param state the ExecutionState to persist
	 */
	public boolean save(ExecutionState state) throws RuntimeException {
		if (HibernateUtil.isActive() && state.isArchive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				util.saveOrUpdate(state);
				return true;
			}
			catch (RuntimeException e) {
				log.warn("State with id "+state.getId()+" of execution "+state.getName()+ "has not been persisted: "+e.getMessage());
				return false;
			}
			finally {
				util.close();
			}
		}
		else {//give internal ad hoc id to execution
			if (state.getId() == null) {
				state.setId(counter++);
			}
			return false;
		}
	}

	/**
	 * removes an {@link ExecutionState} from the persistence.
	 * @param state the state to remove
	 */
	public void remove(ExecutionState state) throws RuntimeException {
		if (HibernateUtil.isActive() && state.isArchive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				util.remove(util.get(ExecutionState.class, state.getId()));
				//also remove all messages
				EntityManager m = util.getEntityManager();
				EntityTransaction tx = m.getTransaction();	
				tx.begin();
				util.getEntityManager().createNativeQuery("delete from message where resultId = :id").setParameter("id", state.getId()).executeUpdate();
				util.getEntityManager().createNativeQuery("delete from executiondetail where resultId = :id").setParameter("id", state.getId()).executeUpdate();
				tx.commit();
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}	
		}
	}

	private void addRestriction(Criteria c, String name, String value) {
		if (value != null && !value.isEmpty()) {
			if (value.contains("*")) {
				value = value.replace("*", "%");
				c.add(Restrictions.ilike(name, value));
			}
			else {
				c.add(Restrictions.eq(name, value));
			}
		}
	}

	private void addOrRestriction(String columnName,Criteria c, String[] values) {
		if (values != null && values.length!=0) {
			ResultCodes codes = new ResultCodes();
		
			Disjunction d = Restrictions.disjunction();
			for(String value:values){
				if(columnName.equals("status"))
					d.add(Restrictions.eq(columnName, codes.getCode(value)));
				else if(columnName.equals("executionType"))
					d.add(Restrictions.eq(columnName, ExecutionTypes.valueOf(value)));
				else
					d.add(Restrictions.eq(columnName, value));
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
	public List<ExecutionState> getResults(String project, String[] types, String name, Date start, Date stop, String[] statuses, String[] executionTypes, int startIndex, int pagesize, SortDirection sortDirection) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				Session s = util.getSession();
				Criteria c = s.createCriteria(ExecutionState.class);
				setCriteriaParameters(project, types, name, start, stop, statuses,
						executionTypes, startIndex, pagesize, c);
				if (sortDirection!=null) {
					switch (sortDirection) {
					case asc : c.addOrder(Order.asc("startDate")); break;
					case desc : c.addOrder(Order.desc("startDate")); break;
					}
				}	
				List<ExecutionState> results = c.list();
				return results;
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}	
		}
		else {
			log.warn("Persistence of Results is not active.");
			return new ArrayList<ExecutionState>();
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
	 * @return the count list of ExecutionStates
	 */
	public Long getResultsCount(String project, String[] types, String name, Date start, Date stop, String[] statuses, String[] executionTypes, int startIndex, int pagesize) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				Session s = util.getSession();
				Criteria c = s.createCriteria(ExecutionState.class);
				setCriteriaParameters(project, types, name, start, stop, statuses,
						executionTypes, startIndex, pagesize, c);
				c.setProjection(Projections.count("id"));
				Long result = (Long) c.uniqueResult();
				return result;
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}	
		}
		else {
			log.warn("Persistence of Results is not active.");
			return Long.valueOf("0");
		}
	}

	/**
	 * @param project
	 * @param types
	 * @param name
	 * @param start
	 * @param stop
	 * @param statuses
	 * @param executionTypes
	 * @param startIndex
	 * @param pagesize
	 * @param c
	 */
	private void setCriteriaParameters(String project, String[] types,
			String name, Date start, Date stop, String[] statuses,
			String[] executionTypes, int startIndex, int pagesize, Criteria c) {
		addRestriction(c,"project",project);
		addOrRestriction("type",c,types);
		addRestriction(c,"name",name);
		addOrRestriction("executionType",c,executionTypes);
		if (start != null) c.add(Restrictions.ge("startDate",start));
		if (stop != null) c.add(Restrictions.le("stopDate",stop));
		addOrRestriction("status",c,statuses);
		if (startIndex != 0) c.setFirstResult(startIndex);
		if (pagesize != 0) c.setMaxResults(pagesize);
	}

	/**
	 * gets a single {@link ExecutionState} by its id.
	 * @param id the unique id of the state to get
	 * @return the execution state from the persistence or null if no such state exists.
	 */
	public ExecutionState getResult(Long id) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				return (ExecutionState) util.get(ExecutionState.class, id);
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}		
		}
		return null;
	}
	
	@SuppressWarnings("unchecked")
	public String getMessagesText(ExecutionState state, String type, Long timestamp, int start, int pagesize) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			if (state.getStatus().equals(Codes.statusRunning)) state.getMessageWriter().flush(); //flush messages to disc
			HibernateUtil util = new HibernateUtil();
			try {
				Session s = util.getSession();
				Criteria c = s.createCriteria(Message.class);
				if (type == null) type = Level.DEBUG.toString();
				if (timestamp == null) timestamp = new Long(0);
				c.add(Restrictions.eq("resultId", state.getId()));
				c.add(Restrictions.ge("intType", Level.toLevel(type).toInt()));
				c.add(Restrictions.ge("timestamp", timestamp));
				if (start != 0) c.setFirstResult(start);
				if (pagesize != 0) c.setMaxResults(pagesize);
				//commented the order since it does not work properly with pagesize
				//c.addOrder(Order.asc("timestamp"));
				List<Message> results = c.list();
				StringBuffer buffer = new StringBuffer();
				for (Message m : results) {
					buffer.append(m.getMessage());
				}
				// If no log messages available check for first error message from configuration phase
				if (buffer.length() == 0 && state.getFirstErrorMessage() != null)
					buffer.append("Configuration Error : "+state.getFirstErrorMessage());
				return buffer.toString();
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}
		}
		else {
			return "Persistence of messages is not active.";
		}
	}
	
	public Long getMessagesCount(ExecutionState state, String type, Long timestamp) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			if (state.getStatus().equals(Codes.statusRunning)) state.getMessageWriter().flush(); //flush messages to disc
			HibernateUtil util = new HibernateUtil();
			try {
				Session s = util.getSession();
				Criteria c = s.createCriteria(Message.class);
				if (type == null) type = Level.DEBUG.toString();
				if (timestamp == null) timestamp = new Long(0);
				c.add(Restrictions.eq("resultId", state.getId()));
				c.add(Restrictions.ge("intType", Level.toLevel(type).toInt()));
				c.add(Restrictions.ge("timestamp", timestamp));
				c.setProjection(Projections.count("id"));
				Long result = (Long) c.uniqueResult();
				return result;
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}
		}
		else {
			return new Long(0);
		}
	}
	
	@SuppressWarnings("unchecked")
	public List<ExecutionDetail> getExecutionDetails(long stateId) throws RuntimeException {
		if (HibernateUtil.isActive()) {
			HibernateUtil util = new HibernateUtil();
			try {
				Session s = util.getSession();
				Criteria c = s.createCriteria(ExecutionDetail.class);
				c.add(Restrictions.eq("resultId", stateId));
				c.addOrder(Order.asc("scope"));
				return c.list();
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
			finally {
				util.close();
			}
		} else {
			return new ArrayList<ExecutionDetail>();
		}
	}

}
