package com.jedox.etl.core.config;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jdom.Element;

import com.jedox.etl.core.component.Locator;

public class MockConfigPersistence implements IConfigPersistence {
	
	private static final Log log = LogFactory.getLog(MockConfigPersistence.class);

	@Override
	public void load() throws Exception {
		log.error("Loading not supported in "+this.getClass().getName());
	}

	@Override
	public void save(Locator loc, Element project) throws Exception {
		log.error("Saving not supported in "+this.getClass().getName());
	}

	@Override
	public void migrate(IConfigPersistence oldPersistence) throws Exception {
		log.error("Migrating not supported in "+this.getClass().getName());
	}

	@Override
	public boolean needMigration() {
		return false;
	}

}
