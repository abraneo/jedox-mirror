package com.jedox.etl.core.persistence;

import com.jedox.etl.core.component.RuntimeException;

public interface IStore {

	/**
	 * writes the data held by the current row to the Datastore using the settings defined by {@link #setDefinition(PersistorDefinition)}
	 * @throws RuntimeException
	 */
	public void write() throws RuntimeException;

	/**
	 * commits the changes written in the Store
	 * @throws RuntimeException
	 */
	public void commit() throws RuntimeException;

	/**
	 * get the current number of rows in the datastore
	 * @throws RuntimeException
	 * @return number of rows in the current table
	 */
	public long getCurrentRowCount() throws RuntimeException;
	
	public void close() throws RuntimeException;


}
