package com.jedox.etl.components.transform;


import java.util.HashSet;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.components.config.transform.SQLConfigurator;
import com.jedox.etl.core.component.InitializationException;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.persistence.generic.PersistenceManager;
import com.jedox.etl.core.persistence.mem.MemPersistor;
import com.jedox.etl.core.persistence.IPersistence;
import com.jedox.etl.core.source.ISource;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.SourceManager;
import com.jedox.etl.core.source.TableSource;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;

public class SQLTransform extends TableSource implements ITransform {
	
	private boolean persistent = false;
	private SourceManager sourceManager;
	private static final Log log = LogFactory.getLog(SQLTransform.class);

	
	public SQLTransform() {
		setConfigurator(new SQLConfigurator());
	}

	public SQLConfigurator getConfigurator() {
		return (SQLConfigurator)super.getConfigurator();
	}

	@Override
	protected IProcessor getSourceProcessor(int size) throws RuntimeException {
		CacheTypes myCacheType = (persistent ? CacheTypes.disk : CacheTypes.memory);
		if (isOptimizePersistence()) {
			Set<CacheTypes> sourceTypeSet = new HashSet<CacheTypes>();
			for (ISource s : sourceManager.getAll()) {
				if (s instanceof IView) {
					sourceTypeSet.add(((IView)s).getBaseSource().getCacheType());
				} else {
					sourceTypeSet.add(s.getCacheType());
				}
			}
			if (!sourceTypeSet.contains(CacheTypes.none) && sourceTypeSet.size() == 1) {
				CacheTypes uniqueType = sourceTypeSet.iterator().next();
				if (!uniqueType.equals(myCacheType)) {
					log.info("Overriding persistence setting with "+uniqueType.toString()+" persistence because it is already available.");
					myCacheType = uniqueType;
				}
			}
		}
		for (ISource s : sourceManager.getAll()) {
			s.setCaching(myCacheType);
			IProcessor p = initProcessor(s.getProcessor(0),Facets.INPUT);
			setQuerySource(getQuerySource().replace(escapeName(s.getName()), s.getPersistentView())); 
			//if (!persistent) this.cache(p);
		}
		IPersistence persistence = myCacheType.equals(CacheTypes.disk) ? PersistenceManager.getPersistence(getInternalConnection()) : MemPersistor.getInstance();
		//System.err.println(getQuerySource());
		return initProcessor(persistence.getQueryResult(getLocator(), getQuerySource(), size),Facets.OUTPUT);
	}
	
	public void init() throws InitializationException {
		super.init();
		try {
			sourceManager = new SourceManager();
			sourceManager.addAll(getConfigurator().getSources());
			addManager(sourceManager);
			persistent = getConfigurator().isPersistent();
		}
		catch (Exception e) {
			throw new InitializationException(e.getMessage());
		}
	}

}
