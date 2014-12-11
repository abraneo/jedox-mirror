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
*   @author Christian Schwarzinger, proclos OG, Wien, Austria
*   @author Andreas Froehlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/

package com.jedox.etl.components.transform;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.rosuda.JRI.RMainLoopCallbacks;
import org.rosuda.JRI.Rengine;

import java.io.StringWriter;

/**
 * REngineManager
 * Handles the access to the Rconsole and the REngine
 * Only one instance of R is allowed to run in one JVM, hence you can run
 * new Rengine() only once in the JVM. This is a design limitation of R
 * which does not allow multiple threads.
 */

public class REngineManager {

	
	static public class RConsole implements RMainLoopCallbacks {
		
		private boolean outputEnabled = false;
		private StringWriter writer = new StringWriter();
		private boolean hasError = false;
		
		public RConsole() {
		}
		
		public void setOutputEnabled(boolean outputEnabled) {
			this.outputEnabled = outputEnabled;
		}

		@Override
		public void rBusy(Rengine re, int which) {
			log.debug("rBusy("+which+")");
		}

		@Override
		public String rChooseFile(Rengine arg0, int arg1) {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void rFlushConsole(Rengine arg0) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void rLoadHistory(Rengine arg0, String arg1) {
			// TODO Auto-generated method stub			
		}

		@Override
		public String rReadConsole(Rengine re, String prompt, int addToHistory) {
		    return null; 
		}

		@Override
		public void rSaveHistory(Rengine arg0, String arg1) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void rShowMessage(Rengine re, String message) {
			log.info("rShowMessage \""+message+"\"");
			
		}

		@Override
		public void rWriteConsole(Rengine re, String text, int oType) {
			if (oType == 1) {
				hasError = true;
				writer.append(text);
			} else {
				if (outputEnabled && !hasError) writer.append(text);
			}
		}	
		
		public StringWriter getWriter() {
			return writer;
		}
		
		public boolean hasError() {
			return hasError;
		}
		
		public void reset() {
			hasError=false;
			// clear all console messages
			writer = new StringWriter(); 
		}
		
		
	}
	
	private static REngineManager instance = null;
	private Rengine re;
	private RConsole console;
	private boolean isLocked=false;
	private static final Log log = LogFactory.getLog(REngineManager.class);
	
	private REngineManager() {
	}

	/**
	 * gets the singleton instance of this ConfigManager
	 * @return the productive instance
	 */
	public synchronized static final REngineManager getInstance() throws RuntimeException {
		if (instance == null) {
			instance = new REngineManager();
			instance.console = new RConsole();
			String[] args = new String[1];
			args[0] = "--no-save";
			instance.re = new Rengine(args, false, instance.console);
	        log.info("Rengine created, waiting for R");
			// the engine creates R is a new thread, so we should wait until it's ready
	        if (!instance.re.waitForR()) {
	            throw new RuntimeException("Cannot load R");
	        }
		}		
		if (instance.isLocked)
			// Prevent parallel threads using the Rengine			
			throw new RuntimeException("REngine is already used by another Execution.");
		
        instance.isLocked=true;
		return instance;
	}
	
	public synchronized static final void releaseInstance() {
		// Release the lock on the Rengine
		instance.isLocked=false;
	}
	
	public Rengine getRengine() {
		return re;
	}
	
	public RConsole getConsole()  {
		return console;
	}
		
	/* ToDo: Check if necessary and if yes find a good place of calling it */
/*	
	public void shutDown() {
		re=null;
		console=null;
	}
*/		
	


}
