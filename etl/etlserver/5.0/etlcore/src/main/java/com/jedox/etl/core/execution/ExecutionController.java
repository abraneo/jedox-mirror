package com.jedox.etl.core.execution;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.util.NamingUtil;

public class ExecutionController {
	
	private ThreadedExecutor single; 
	private ThreadedExecutor parallel; 
	private ThreadedExecutor direct;
	
	private Map<Long,Future<?>> lookup = new HashMap<Long,Future<?>>();
	private static final Log log = LogFactory.getLog(ExecutionController.class);
	
	public ExecutionController() {
		single = new ThreadedExecutor(SyncModes.SINGLE, this);
		parallel = new ThreadedExecutor(SyncModes.PARALLEL, this);
		direct = new ThreadedExecutor(SyncModes.DIRECT, this);
	}
	
	private void waitForFuture(Future<?> f) {
		try {
			f.get();
		} catch (InterruptedException e1) {
			waitForFuture(f);
		} catch (ExecutionException e1) {
			log.error("Error waiting Execution to finish: "+e1.getMessage());
		}
	}
	
	public synchronized void waitFor(ThreadedExecutor caller, Runnable runnable) {
		single.pause();
		parallel.pause();
		List<Runnable> runningSingle = single.getRunning();
		while (runningSingle.size() > 0) {
			try { //we are not synchronized on the list. so list may be empty now, which causes an exception, which we ignore here
				Runnable r = runningSingle.get(0);
				waitForFuture((Future<?>)r);
			}
			catch(Exception e) {}
		}
		if (caller.getSyncMode().equals(SyncModes.SINGLE)) {
			List<Runnable> runningGroup = parallel.getRunning();
			while (runningGroup.size() > 0) {
				try { //we are not synchronized on the list. so list may be empty now, which causes an exception, which we ignore here
					Runnable r = runningGroup.get(0);
					waitForFuture((Future<?>)r);
				}
				catch (Exception e) {}
			}
		}
		caller.getRunning().add(runnable);
		parallel.resume();
		single.resume();
	}

	
	public void add(Execution execution) throws com.jedox.etl.core.execution.ExecutionException {
		String syncModeValue = null;
		String configSyncModeString = Settings.getInstance().getContext(Settings.ExecutionsCtx).getProperty(NamingUtil.internal("syncMode"), SyncModes.SINGLE.toString());
		switch (execution.getExecutionType()) {
		case EXECUTION: syncModeValue = execution.getExecutable().getContext().getParameter().getProperty(NamingUtil.internal("syncMode"), configSyncModeString); break;
		default: syncModeValue = SyncModes.DIRECT.toString();
		}
		try {
			//ensure that parameter is present in context and gets displayed in log. 
			execution.getExecutable().getContext().getParameter().setProperty(NamingUtil.internal("syncMode"), syncModeValue);
		}
		catch (Exception e) {}
		SyncModes syncMode = SyncModes.valueOf(syncModeValue.toUpperCase());
		switch (syncMode) {
		case DIRECT: lookup.put(execution.getKey(), direct.submit(execution)); break;
		case PARALLEL: lookup.put(execution.getKey(), parallel.submit(execution)); break;
		case SINGLE: lookup.put(execution.getKey(), single.submit(execution)); break;
		}
	}
	
	public Future<?> getFuture(Long id) {
		return lookup.get(id);
	}
	
	public void remove(Execution execution) {
		direct.remove(execution);
		parallel.remove(execution);
		single.remove(execution);
		direct.purge();
		parallel.purge();
		single.purge();
	}
	
	public synchronized void shutdown() {
		direct.shutdown();
		parallel.shutdown();
		single.shutdown();
	}
	
}
