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
*   @author Andreas Fröhlich, Jedox AG, Freiburg, Germany
*   @author Kais Haddadin, Jedox AG, Freiburg, Germany
*/
package com.jedox.etl.core.context;

import com.jedox.etl.core.component.IComponent;
import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.execution.ExecutionDetail;
import com.jedox.etl.core.load.ILoad;
import com.jedox.etl.core.source.IView;
import com.jedox.etl.core.source.processor.IProcessor;
import com.jedox.etl.core.source.processor.IProcessor.Facets;
import com.jedox.etl.core.transform.ITransform;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;

public class ProcessorRegistry {
	
	private LinkedHashMap<String,List<IProcessor>> registry = new LinkedHashMap<String,List<IProcessor>>();
	private Map<String, IComponent> componentMap = new HashMap<String,IComponent>();
	//private static final Log log = LogFactory.getLog(ProcessorRegistry.class);
	
	/*
	private void checkIncluded(IProcessor processor) {
		List<IProcessor> chain = processor.getProcessorChain();
		if (chain.size() > 1) {
		    IProcessor rootProcessor = chain.get(0);
		    if (rootProcessor.getFacet().equals(Facets.OUTPUT) && isProcessable(rootProcessor.getOwner())) { //we have a tree structure!
		    	IProcessable p = (IProcessable)rootProcessor.getOwner();
		    	long shiftTime = p.getProcessingTime();// - calculateProcessorTime(p);
		    	if (processor.getOverallProcessingTime() > shiftTime) {
		    		processor.addProcessingTime(-shiftTime);
		    	}
		    }
		}
	}
	*/
	
	private String getKey(IComponent component) {
		String key = component.getClass().getCanonicalName()+"_"+component.getLocator().toString();
		return key;
	}
	
	private List<IProcessor> get(IComponent component) {
		return registry.get(getKey(component));
	}
	
	private void register(IComponent component, List<IProcessor> processors) {
		String key = getKey(component);
		registry.put(key, processors);
		componentMap.put(key, component);
	}
	
	public void addProcessor(IProcessor processor) {
		IComponent owner = processor.getOwner();
		List<IProcessor> list = get(owner);
		if (list == null) {
			list = new ArrayList<IProcessor>();
			register(owner,list);
		}
		if (!list.contains(processor)) {
			//check that there is only one output processor and only one input processor
			if (processor.getFacet().equals(Facets.INPUT)) {
				for (IProcessor p : list) {
					if (p.getFacet().equals(Facets.INPUT) && !p.getClass().equals(processor.getClass()))
						processor.setFacet(Facets.HIDDEN); //first registered input processor wins. all instances of this class are considered
				}
			}
			if (processor.getFacet().equals(Facets.OUTPUT)) {
				for (IProcessor p : list) {
					if (p.getFacet().equals(Facets.OUTPUT) && !p.getClass().equals(processor.getClass()))
						p.setFacet(Facets.HIDDEN); //last registered output processor wins. all instances of this class are considered
				}
			}
			list.add(processor);
		}
	}
	
	public void addLoad(ILoad load) {
		List<IProcessor> list = get(load);
		if (list == null) {
			register(load, new ArrayList<IProcessor>());
		}
	}
	
	public void clear() {
		registry.clear();
		componentMap.clear();
	}
	
	public List<IProcessor> getProcessorsByOwner(IComponent component) {
		if (component != null) {
			List<IProcessor> result = get(component);
			return result != null ? result : new ArrayList<IProcessor>();
		}
		else {
			List<IProcessor> result = new ArrayList<IProcessor>();
			for (List<IProcessor> ppc : registry.values()) {
				result.addAll(ppc);
			}
			return result;
		}
	}
	
	private boolean isLoad(IComponent c) {
		return (c instanceof ILoad);
	}
	
	private long calculateProcessorTime(IComponent component) {
		long result = 0;
		List<IProcessor> processors = get(component);
		if (processors != null && !processors.isEmpty()) { //table based processable. take processors
			IProcessor p = processors.get(processors.size()-1);
			//log.info("In Load: " +p.getClass() + ":" + p.getOwnProcessingTime());
			result += p.getOverallProcessingTime();
			//log.info(result);
		}
		return result;
	}
	 
	private long calculateLoadTime(IComponent component) {
		long result = 0;
		if ((isLoad(component))) {
			result += calculateProcessorTime(component);
			ILoad load = (ILoad)component;
			result = load.getProcessingTime() - result;
		} 
		return result;
	}
/*	
	private boolean hasNoProcessorOfFacet(List<IProcessor> processors, Facets facet) {
		for (IProcessor p : processors) {
			if (facet.equals(p.getFacet())) return false;
		}
		return true;
	}
	
	private void fixMissingInput(ExecutionDetail d, IProcessor p) {
		if (p.getSourceProcessor() != null) {
			d.setProcessedInputRows(Integer.valueOf(p.getSourceProcessor().getRowsAccepted()).longValue()); //if we have just hidden processors and no own input processor
			d.setInputCalls(d.getInputCalls()+1);
		}
	}
*/
	
	private void fixMissingOutput(ExecutionDetail d, IComponent c) {
		if (c instanceof ITransform && d.getProcessedOutputRows().longValue() == 0 && !isLoad(c)) {
			d.setProcessedOutputRows(d.getProcessedInputRows()); //we have no special output processor.
			d.setOutputCalls(d.getInputCalls());
		}
	}
	
	public Map<String,ExecutionDetail> getExecutionDetails() {
		Map<String,ExecutionDetail> result = new LinkedHashMap<String,ExecutionDetail>();
		for (String k : registry.keySet()) {
			IComponent c = componentMap.get(k);
			ExecutionDetail d = result.get(c.getLocator().toString());
			if (d == null) {
				d = new ExecutionDetail();
				d.setLocator(c.getLocator().toString());
				try {
					d.setScope(ITypes.MainManagers.valueOf(c.getLocator().getManager()));
					d.setType(c.getConfigurator().getXML().getAttributeValue("type"));
				}
				catch (Exception e) {
					e.printStackTrace();
				}
				result.put(d.getLocator(),d);
			} else {
				if (d.getType()==null) // happens for View Sources
					d.setType(c.getConfigurator().getXML().getAttributeValue("type"));
			}
			List<IProcessor> processors = get(c);
			for (IProcessor p : processors) {
				//log.info(p.getFacet() + ":" + c.getName() + ":" +p.getClass() + ":" + p.getOwnProcessingTime());
				if (!isLoad(c)) 
					d.setRuntime(d.getRuntime().longValue() + p.getOwnProcessingTime()/1000);   // time in micro seconds
				switch(p.getFacet()) {
				case INPUT: {
					d.setProcessedInputRows(d.getProcessedInputRows().longValue() + p.getRowsAccepted());					
					d.setInputCalls(d.getInputCalls()+1);
					break;
				}
				case OUTPUT: {
					if (!(c instanceof IView)) {
						d.setProcessedOutputRows(d.getProcessedOutputRows().longValue() + p.getRowsAccepted());
						d.setOutputCalls(d.getOutputCalls()+1);
					}
					break;
				}
				default:
				}
			}
			fixMissingOutput(d, c);
		}
		//2nd pass special treatment for loads, which have own overall processing time
		for (String k : registry.keySet()) {
			IComponent c = componentMap.get(k);
			if (isLoad(c)) {
				ExecutionDetail d = result.get(c.getLocator().toString());
				d.setRuntime(d.getRuntime()+calculateLoadTime(c)/1000);  //time in micro seconds
			}
		}
		/*
		for (ExecutionDetail d : result.values()) {
			d.setRuntime(d.getRuntime() / 1000000); //strip runtime to ms
		}
		*/
		return result;
	}

}
