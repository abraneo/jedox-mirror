package com.jedox.etl.components.connection;


import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.persistence.Datastore;
import com.jedox.etl.core.persistence.PersistorDefinition;

public class Filestore extends Datastore {
	private long timestamp;
	
	public Filestore(PersistorDefinition definition) throws RuntimeException {
		super(definition);
	}
	
	/**
	 * sets an expiry timestamp for a given configuration defined by the hash value
	 * @param hash the hash value defining the configuration
	 * @param timestamp the timestamp
	 */
	public void setTimestamp(long timestamp) {
		this.timestamp = timestamp;
	}

	/**
	 * gets the timestamp for a given configuration defined by the hash
	 * @param hash the hash defining the configuration
	 * @return the timestamp value
	 */
	public long getTimestamp() {
		return timestamp;
	}
	
	public void setDefinition(PersistorDefinition definition) throws RuntimeException {
		super.setDefinition(definition);
	}
}