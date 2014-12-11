package com.jedox.etl.core.persistence.hibernate;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.hibernate.Query;
import org.hibernate.StatelessSession;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.MessageHandler;
import com.jedox.etl.core.node.Column;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.RelationalNode;
import com.jedox.etl.core.node.RelationalNode.UpdateModes;
import com.jedox.etl.core.node.Row;
import com.jedox.etl.core.persistence.PersistorDefinition;
import com.jedox.etl.core.persistence.PersistorDefinition.AggregateModes;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.SQLUtil;
import com.jedox.etl.core.util.TypeConversionUtil;

public class HibernateAggregator extends HibernatePersistor {
	
	private Log aggLog = new MessageHandler(getLog());
		
	
	public HibernateAggregator(PersistorDefinition definition) throws RuntimeException {
		super(definition);
		if (AggregateModes.bulk.equals(definition.getAggregateMode()) && !isBulkAggregable(definition)) 
			throw new RuntimeException("Cannot aggregate data in mode 'bulk' because table does not have a signle numeric primary key specified. Maybe you want to use modes 'row'/'false' instead, change the key or create a new table?");
	}
	
	private boolean isBulkAggregable(PersistorDefinition definition) {
		if (MappingHelper.strategyGuid.equals(definition.getPrimaryKeyGeneration()))
			return false;
		if (MappingHelper.strategyAssigned.equals(definition.getPrimaryKeyGeneration()) && getPrimaryKeys().size() == 1) {
			IColumn c = getDataDefinition().getColumn(getPrimaryKeys().getColumn(0).getName());
			try {
				if (Number.class.isAssignableFrom(c.getValueType())) {
					getLog().warn("You are using bulk aggregation on a primary key with provided (numeric) values. Please note, that primary key values will change during aggregation!");
					return true;
				}
				else
					return false;
			}
			catch (Exception e) {
				return false;
			}
		}
		return true;
	}	

	
	private String getSQLAggregateSelect(RelationalNode c, String quote) {
		String name = SQLUtil.quoteName(c.getName(), quote);
		String aggName =  SQLUtil.aliasName(c.getRole().toString()+"("+name+")",name);
		switch(c.getRole()) {
		case sum: return aggName;
		case min: return aggName;
		case max: return aggName;
		case count: return aggName;
		case avg: return aggName;
		default : return "";
		}
	}

	private Boolean isInternalKey() {
		return (!MappingHelper.strategyIdentity.equals(getDefinition().getPrimaryKeyGeneration()) 
				&& !MappingHelper.strategySequence.equals(getDefinition().getPrimaryKeyGeneration()) 
				&& !MappingHelper.strategyNative.equals(getDefinition().getPrimaryKeyGeneration()));
	}
	
	private String getSQLAggregateQuery(Long offset) throws RuntimeException  {
		Row keys = getDefinition().getKeys();
		Row toSet = new Row();
		toSet.addColumns(getDefinition().getToSet());
		Row toKeep = getDefinition().getToKeep();
		String pkName = getPrimaryKeys().getColumn(0).getName();
		if (keys.containsColumn(pkName) || toSet.containsColumn(pkName) || toKeep.containsColumn(pkName)) {
			keys.removeColumn(pkName);
			toSet.removeColumn(pkName);
			toKeep.removeColumn(pkName);
			getLog().warn("Usage of primary key '"+pkName+"' in bulk aggregation column definition is not allowed. Ignoring it.");
		}
		
		String quote = getConnection().getIdentifierQuote();
		String baseSource = "base";
		String maxSource = "jput";
		String minSource = "jkeep";
		String minId = "min("+SQLUtil.quoteName(getPrimaryKeys().getColumn(0).getName(),quote)+")";
		String maxId = "max("+SQLUtil.quoteName(getPrimaryKeys().getColumn(0).getName(), quote)+")";
		String minRef = SQLUtil.quoteName(NamingUtil.internal("minId"),quote);
		String maxRef = SQLUtil.quoteName(NamingUtil.internal("maxId"),quote);
		String unquotedId = getPrimaryKeys().getColumn(0).getName();
		String id = SQLUtil.quoteName(unquotedId, quote);
		List<String> innerSelects = new LinkedList<String>();
		List<String> outerSelects = new ArrayList<String>();

		Row data = getDefinition().getColumnDefinition();
		if (isInternalKey()) { //explicitly calculate key to be set explicitly later
			innerSelects.add(SQLUtil.aliasName("("+offset+"+"+minId+")",id));
			outerSelects.add(SQLUtil.prefixName(id,baseSource));
		}
		for (RelationalNode c : data.getColumns(RelationalNode.class)) {
			if (keys.indexOf(c) >= 0) {
				innerSelects.add(SQLUtil.quoteName(c.getName(),quote));
				outerSelects.add(SQLUtil.prefixName(c.getName(),baseSource,quote));
			}
			else if (toSet.indexOf(c) >= 0 && !c.getRole().equals(UpdateModes.last)) {
				innerSelects.add(getSQLAggregateSelect(c,quote));
				outerSelects.add(SQLUtil.prefixName(c.getName(),baseSource,quote));
			}
			else if (toKeep.indexOf(c) >= 0) {
				outerSelects.add(SQLUtil.prefixName(c.getName(),minSource,quote));
			}
			else if (toSet.indexOf(c) >= 0 && c.getRole().equals(UpdateModes.last)) {
				outerSelects.add(SQLUtil.prefixName(c.getName(),maxSource,quote));;
			}
		}

		StringBuffer outerFromPart = new StringBuffer();
		//join minSource?
		if (toKeep.size() > 0) {
			innerSelects.add(SQLUtil.aliasName(minId,minRef));
			outerFromPart.append(" inner join "+getTablename()+" "+minSource+" on " +SQLUtil.prefixName(minRef,baseSource)+" = "+SQLUtil.prefixName(id, minSource));
		}
		//join maxSource?
		boolean hasLast = false;
		for (RelationalNode c : toSet.getColumns(RelationalNode.class)) { if (c.getRole().equals(UpdateModes.last)) hasLast = true;}
		if (hasLast) {
			innerSelects.add(SQLUtil.aliasName(maxId,maxRef));
			outerFromPart.append(" inner join "+getTablename()+" "+maxSource+" on " +SQLUtil.prefixName(maxRef,baseSource)+" = "+SQLUtil.prefixName(id, maxSource));
		}

		String aggInnerSelect = SQLUtil.buildQuery(
				getTablename(),
				SQLUtil.enumNames(innerSelects),
				"",
				SQLUtil.enumNames(SQLUtil.quoteNames(keys.getColumnNames(),quote)),
				"");

		outerFromPart.insert(0, "("+aggInnerSelect+") "+baseSource);

		String aggOuterSelect = SQLUtil.buildQuery(
				outerFromPart.toString(),
				SQLUtil.enumNames(outerSelects),
				"", "", "");

		return aggOuterSelect;
	}

	private void bulkAggregate() throws RuntimeException {
		try {
			getLog().info("Aggregating the data ..");
			flush();
			List<String> ids = new ArrayList<String>();
			ids.add("max("+MappingHelper.idName+")");
			ids.add("min("+MappingHelper.idName+")");
			String getIdsString = SQLUtil.buildQuery(getName(),SQLUtil.enumNames(ids), "", "", "");
			Long maxId = new Long(0);
			Long minId = new Long(0);
			Object[] idsReturned = (Object[])getSession().createQuery(getIdsString).uniqueResult();
			if (idsReturned != null && idsReturned[0] instanceof Number && idsReturned[1] instanceof Number) {
				maxId = ((Number) idsReturned[0]).longValue();
				minId = ((Number) idsReturned[1]).longValue();
				
				String aggregate = getSQLAggregateQuery(maxId-minId+1);
				getLog().debug("SQL insert:"+ aggregate);

				String quote = getConnection().getIdentifierQuote();
				Row data = new Row();
				data.addColumns(getDefinition().getColumnDefinition());
				String unquotedId = getPrimaryKeys().getColumn(0).getName();
				String id = SQLUtil.quoteName(unquotedId, quote);
				List<String> columns = new ArrayList<String>();
				if (isInternalKey()) {
					columns.add(id);
					//remove potential assigned key from data, since we already have dealt with key.
					data.removeColumn(unquotedId);
				}
				columns.addAll(SQLUtil.quoteNames(data.getColumnNames(),quote));
				//we have to use connection directly, since we violate hibernate hilo key contraints.
				String insertString = "insert into "+getTablename() +"(" + SQLUtil.enumNames(columns)+ ") "+aggregate;
				getLog().debug(insertString);
				//commit all changes before update.
				//getConnection().commit();
				int inserted = getConnection().open().createStatement().executeUpdate(insertString);

				getLog().debug("after aggregating, inserted "+inserted+" lines.");

				String deleteString = "delete from "+getTablename()+" where "+SQLUtil.quoteName(getPrimaryKeys().getColumn(0).getName(), quote)+" <= "+maxId;
				int deleted = getConnection().open().createStatement().executeUpdate(deleteString);
				getLog().debug("deleted "+deleted+" lines.");
				getConnection().commit();
			}
			else
				getLog().error("Primary key "+getDefinition().getPrimaryKey()+" is not numeric. Bulk aggregation cannot be performed.");
		}
		catch (Exception e) {
			getLog().error(e.getMessage());
			throw new RuntimeException(e);
		}
	}
	
	private Double getNumericValue(String name, Object value) {
		if (value instanceof Number) {
			return ((Number)value).doubleValue();
		}
		if (value == null) {
			return Double.valueOf(0);
		}
		try {
		  return TypeConversionUtil.convertToNumeric(value.toString());
		}
		catch (NumberFormatException e) {
			aggLog.warn("Value '"+value+"' of column '"+name+"' is not numeric. Setting it to 0.");
			return Double.valueOf(0);
		}
	}
	
	private void rowAggregate(Row row) throws RuntimeException {
		try {
			StatelessSession session = getSession();
			Row keys = getDefinition().getKeys();
			String hqlSelect = "from "+getName()+SQLUtil.getPrepearedWhereCondition(getHibernateNames(keys));
			Query query = getSession().createQuery( hqlSelect);
			for (int i=0; i<keys.size(); i++) {
				IColumn c = keys.getColumn(i);
				Object value =  convert(c,getDataDefinition().getColumn(c.getName()));
				query.setParameter(i,value);
			}
			@SuppressWarnings("unchecked")
			List<Map<String,Object>> result = query.list();
			if (result.isEmpty()) {
				//check for count
				for (RelationalNode c : getDefinition().getToSet().getColumns(RelationalNode.class)) {
					if (c.getRole().equals(UpdateModes.count))
						row.getColumn(c.getName()).setValue(Integer.valueOf(1));
				}
				save(row);
				//we have to flush on each save, since we do read / write
				flush();
				openTransaction();
			}
			else {
				for (Map<String,Object> m : result) {
					for (RelationalNode c : getDefinition().getToSet().getColumns(RelationalNode.class)) {
						IColumn definitionColumn = getDataDefinition().getColumn(c.getName());
						if(definitionColumn != null) { // column is present in database
							String pname = getColumnMap().get(c.getName());
							switch (c.getRole()) {
							case last: m.put(pname, convert(c,definitionColumn)); break;
							case sum: {
								Column num = new Column();
								num.setValue(getNumericValue(c.getName(),c.getValue())+getNumericValue(c.getName(),m.get(pname)));
								m.put(pname, convert(num,definitionColumn));
								break;
							}
							case min: {
								Column num = new Column();
								num.setValue(Math.min(getNumericValue(c.getName(),c.getValue()),getNumericValue(c.getName(),m.get(pname))));
								m.put(pname, convert(num,definitionColumn));
								break;
							}
							case max: {
								Column num = new Column();
								num.setValue(Math.max(getNumericValue(c.getName(),c.getValue()),getNumericValue(c.getName(),m.get(pname))));
								m.put(pname, convert(num,definitionColumn));
								break;
							}
							case count: {
								Column num = new Column();
								num.setValue(getNumericValue(c.getName(),m.get(pname))+1);
								m.put(pname, convert(num,definitionColumn));
								break;
							}
							case avg: {
								Column num = new Column();
								num.setValue((getNumericValue(c.getName(),c.getValue())+getNumericValue(c.getName(),m.get(pname)))/2);
								m.put(pname, convert(num,definitionColumn));
								break;
							}
							default: throw new RuntimeException("Operation not supported.");
							}
						}
					}
					//update
					session.update(getName(), m);
					//we have to flush on each update, since we do read / write
					flush();
					openTransaction();				
				}
			}
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
	
	public void write() throws RuntimeException {
		switch (getDefinition().getAggregateMode()) { 
		case row: rowAggregate(getDefinition().getColumnDefinition()); break;
		default: super.write();
		}
	}
	
	public void commit() throws RuntimeException {
		switch (getDefinition().getAggregateMode()) { 
		case bulk: bulkAggregate();
		default: break;
		}
		super.commit();
	}

}
