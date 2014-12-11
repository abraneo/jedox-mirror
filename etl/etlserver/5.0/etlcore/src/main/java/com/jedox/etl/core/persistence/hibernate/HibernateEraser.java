package com.jedox.etl.core.persistence.hibernate;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.hibernate.Query;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.util.SQLUtil;

public class HibernateEraser extends HibernatePersistor {
	
	private class DeleteAggregator {
		private List<String> buffer = new ArrayList<String>();
		private List<Object> values = new ArrayList<Object>();
		
		public void add(Row data) throws RuntimeException {
			List<String> names = new ArrayList<String>();
			for (String k : getLogicalKeyMap().keySet()) {
				names.add(k);
				Object value = data.getColumn(k).getValue();
				values.add(value);
			}
			buffer.add(SQLUtil.getWhereCondition(SQLUtil.quoteNames(names, getDefinition().getConnection().getIdentifierQuote()), SQLUtil.getConstants("?", names.size()), SQLUtil.getConstants("=", names.size()), "AND"));
		}
		
		private String getDeleteCondition() {
			return SQLUtil.getJunction(buffer, "OR");
		}
		
		public Query getDeleteQuery() throws RuntimeException {
			Query query = getSession().createSQLQuery("delete from "+getTablename()+" where "+getDeleteCondition());
			for (int i=0; i<buffer.size(); i++) {
				int offset = i*getLogicalKeyMap().size();
				for (String k : getLogicalKeyMap().keySet()) {
					Column c = new Column(k);
					c.setValue(values.get(offset));
					Object value =  convert(c,getDataDefinition().getColumn(c.getName()));
					query.setParameter(offset,value);
					offset++;
				}
			}
			buffer.clear();
			values.clear();
			return query;
		}
		
		public int getSize() {
			return buffer.size();
		}
	}
	
	private DeleteAggregator deleteAggregator;
	private int deleteAggregationSize = 20;
	private Map<String,String> logicalKeyMap;

	public HibernateEraser(PersistorDefinition definition) throws RuntimeException {
		super(definition);
		deleteAggregator = new DeleteAggregator();
	}
	
	private Map<String,String> getLogicalKeyMap() {
		if (logicalKeyMap == null) {
			logicalKeyMap = new HashMap<String,String>();
			//add explicitly defined keys
			for (IColumn c : getDefinition().getKeys().getColumns()) 
				logicalKeyMap.put(c.getName(),getColumnMap().get(c.getName()));
			//add assigned primary key if keyMap is still empty
			if (logicalKeyMap.isEmpty() && getDefinition().getPrimaryKeyGeneration().equals(MappingHelper.strategyAssigned)) {
				for (IColumn c : getPrimaryKeys().getColumns()) {
					logicalKeyMap.put(c.getName(),getColumnMap().get(c.getName()));
				}
			}
			//add all data columns if keyMap is still empty
			if (logicalKeyMap.isEmpty()) {
				for (IColumn c : getDefinition().getColumnDefinition().getColumns()) {
					logicalKeyMap.put(c.getName(),getColumnMap().get(c.getName()));
				}
			}
		}
		return logicalKeyMap;
	}
	
	private void bulkDelete() throws RuntimeException {
		if (deleteAggregator != null && deleteAggregator.getSize() > 0) {
			try {
				//int deletedEntities = getConnection().open().createStatement().executeUpdate(sqlDelete);
				Query query = deleteAggregator.getDeleteQuery();
				//Query query = getSession().createQuery( hqlDelete );
				int deletedEntities = query.executeUpdate();
				getLog().debug("deleted "+deletedEntities+" lines.");
			}
			catch (Exception e) {
				throw new RuntimeException(e);
			}
		}
	}
	
	public void write() throws RuntimeException {
		if (deleteAggregator == null) { //use single line deletion
			delete(getDefinition().getColumnDefinition());
		}
		else {
			deleteAggregator.add(getDefinition().getColumnDefinition());
			if (deleteAggregator.getSize() >= deleteAggregationSize)
				bulkDelete();
		}
	}
	
	public void commit() throws RuntimeException {
		//delete remaining buffer
		bulkDelete();
		super.commit();
	}
	
	

}
