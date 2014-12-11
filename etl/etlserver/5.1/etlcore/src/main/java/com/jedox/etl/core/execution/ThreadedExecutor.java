package com.jedox.etl.core.execution;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.config.Settings;

public class ThreadedExecutor extends ScheduledThreadPoolExecutor {

	public enum SyncModes {
		DIRECT, PARALLEL, SINGLE
	}
	
	private static final Log log = LogFactory.getLog(ThreadedExecutor.class);
	private final List<Runnable> running = Collections.synchronizedList(new LinkedList<Runnable>());
	private SyncModes syncMode;
	private boolean isPaused;
	private ReentrantLock pauseLock = new ReentrantLock();
	private Condition unpaused = pauseLock.newCondition();
	private ExecutionController controller;

	private final static int getPoolSizeFromConfig(String name, int defaultValue) {
		try {
			return Integer.parseInt(Settings.getInstance().getContext(Settings.ExecutionsCtx).getProperty(name, String.valueOf(defaultValue)));
		}
		catch (Exception e) {
			log.warn("Error getting value for execution config parameter "+name+": "+e.getMessage());
			return defaultValue;
		}
	}
	
	private final static int getCorePoolSize(SyncModes syncMode) {
		switch(syncMode) {
		case SINGLE: return 1;
		case PARALLEL: return getPoolSizeFromConfig("maxParallelThreads",7);
		case DIRECT: return getPoolSizeFromConfig("maxDirectThreads",11);
		default: return 1;
		}
	}
	
	public ThreadedExecutor(SyncModes syncMode, ExecutionController controller) {
		super(getCorePoolSize(syncMode));
		this.syncMode = syncMode;
		this.controller = controller;
	}

	public SyncModes getSyncMode() {
		return syncMode;
	}

	protected void beforeExecute(Thread t, Runnable r) {
		super.beforeExecute(t, r);
		pauseLock.lock();
		try {
			while (isPaused) unpaused.await();
			switch (syncMode) {
			case DIRECT: {
				running.add(r);
				break;
			}
			default: {
				controller.waitFor(this,r);
				break;
			}
			}
		} catch (InterruptedException ie) {
			t.interrupt();
		} finally {
			pauseLock.unlock();
			//controller.resume(this.getSyncMode());
		}
	}

	public void pause() {
		pauseLock.lock();
		try {
			isPaused = true;
		} finally {
			pauseLock.unlock();
		}
	}

	public void resume() {
		pauseLock.lock();
		try {
			isPaused = false;
			unpaused.signalAll();
		} finally {
			pauseLock.unlock();
		}
	}

	protected synchronized void afterExecute(Runnable r, Throwable t) {
		running.remove(r);
		super.afterExecute(r, t);
	}

	public synchronized List<Runnable> getRunning() {
		return running;
	}

}
