package com.jedox.etl.service.test;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;

import com.jedox.etl.service.ComponentDependencyDescriptor;
import com.jedox.etl.service.ComponentTypeDescriptor;
import com.jedox.etl.service.ETLService;
import com.jedox.etl.service.ExecutionDescriptor;
import com.jedox.etl.service.ResultDescriptor;
import com.jedox.etl.service.ServerStatus;
import com.jedox.etl.service.Variable;
import com.jedox.etl.core.component.Locator;
import com.jedox.etl.core.config.ConfigConverter;
import com.jedox.etl.core.config.ConfigManager;
import com.jedox.etl.core.config.Settings;
import com.jedox.etl.core.config.XMLReader;
import com.jedox.etl.core.execution.ThreadedExecutor.SyncModes;
import com.jedox.etl.core.util.NamingUtil;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.service.ComponentOutputDescriptor;
import com.jedox.etl.core.logging.LogManager;


public class Main {

	private static ETLService service = new ETLService();
	//private static ETLService service = null;

	private static String readFileAsString(String filePath) throws Exception {
		XMLReader reader = new XMLReader();
		return XMLUtil.jdomToString(reader.readDocument(filePath).getRootElement());
	}


	private static void getComponentNames(String locator) throws Exception {
		//getComponent
		String result[] = service.getNames(locator);
		for (String s: result) {
			System.out.println(s);
		}
	}

	private static void getComponentLocators(String locator) throws Exception {
		//getComponent
		String result[] = service.getLocators(locator);
		for (String s: result) {
			System.out.println(s);
		}
	}


	private static ResultDescriptor[] getComponentConfigs(String[] locators) throws Exception {
		//getComponentConfig
		ResultDescriptor[] result = service.getComponentConfigs(locators);
		//String[] result = service.getComponentConfig(null, null);
		for (ResultDescriptor s: result) {
			System.out.println(s.getResult());
		}
		return result;
	}

	private static void executeComponents(String[] locators) throws Exception {
		//getComponentOutline
		ArrayList<ExecutionDescriptor> descriptors = new ArrayList<ExecutionDescriptor>();
		for (String locator : locators) {
			ExecutionDescriptor e = service.addExecution(locator, null);
			if (e.getValid()) {
				descriptors.add(e);
				service.runExecution(e.getId());
				//queryExecutions(null,null,null,0,0,null);
			}
		}
		for (ExecutionDescriptor d : descriptors) {
			//queryExecutions(null,null,null,0,0,"0 5");
			//service.stopExecution(d.getId());
			d = service.getExecutionStatus(d.getId(), true);
			//d = service.removeExecutions(new ExecutionDescriptor[]{d})[0];
			System.out.println(d.getId()+" "+d.getStatus());
			System.out.println(d.getErrors());
			System.out.println(d.getErrorMessage());
			System.out.println(service.getExecutionLog(d.getId(), "INFO", null).getResult());
			//System.out.println("log: "+service.getExecutionLog(d.getId(),null,null).getResult());
		}
	}

	private  static void getData(String locator, String format) throws Exception {
		//getComponentOutline
		ExecutionDescriptor s = service.getData(locator,new Variable[]{}, format,20,true);
		System.out.println(s.getStatus());
		System.out.println(s.getValid() + " "+ s.getErrorMessage());
		System.out.println(s.getResult());
	}

	private  static void fetchData(String locator, String format, int lines) throws Exception {
		ExecutionDescriptor s = service.prepareData(locator,new Variable[]{}, format,lines);
		s = service.getExecutionStatus(s.getId(), true);
		for (int i = 0; i < lines; i++) {
			ResultDescriptor d = service.fetchData(s.getResult(), i, 1);
			System.out.println(d.getResult());
		}
	}


	private  static void removeComponents(String[] locators) throws Exception {
		//getComponentOutline
		//String[] names = {"ImportRelDB"};
		service.removeComponents(locators);
	}


	private  static void addComponents(String[] locators, String[] configs) throws Exception {
		//getComponentOutline
		service.addComponents(locators,configs);
	}

	private static void drillThroughBulk(String export, String[] names, String[] values, int lines) throws Exception {
		//ResultDescriptor s = service.drillThrough(export, names, values,lines);
		//System.out.println(s.getResult());
	}

	private static void uploadFile(String name, byte[] data) throws Exception {
		System.out.println(service.uploadFile(name, data));
	}

	private static void queryExecutions(String project, String type, String name, long start, long stop, String status) throws Exception {
		ExecutionDescriptor[] results = service.getExecutionHistory(project, type, name, start, stop, status);
		for (ExecutionDescriptor r : results) {
			System.out.println(r.getId()+", "+r.getStatus()+", "+new Date(r.getStartDate()).toGMTString()+", "+new Date(r.getStopDate()).toGMTString()+", "+r.getProject()+"."+r.getType()+"."+r.getName());
		}
	}

	private static void getExecutionLog(long id) throws Exception {
		ResultDescriptor r = service.getExecutionLog(id,null,null);
		System.out.println(r.getResult());
	}

	private static void validateConfigs(String[] locators, String configs[]) throws Exception {
		ResultDescriptor[] results = service.validateComponents(locators,configs);
		for (ResultDescriptor s: results) System.out.println(s.getValid());
	}

	private static void getComponentTypes(String scope) throws Exception {
		ComponentTypeDescriptor[] results = service.getComponentTypes(scope);
		for (ComponentTypeDescriptor s: results) {
			System.out.println(s.getType());
			//System.out.println(s.getSchema());
			System.out.println(s.isTree());
		}
	}

	private static void getDimensions(String locator, String database, String cube, String mask) throws Exception {
		ResultDescriptor result = service.getOlapCubeDimensions(locator, cube, mask);
		System.out.println(result.getResult());
	}

	private static Element stringTojdom(String root) {
		try {
			Document doc = new SAXBuilder().build(new StringReader(root));
			Element element = doc.getRootElement();
			element.detach();
			return element;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}

	private static void testConfigManager() throws Exception {
		ResultDescriptor[] config = service.getComponentConfigs(new String[]{"importOLAP"});
		ConfigManager.getInstance().add(new ConfigConverter(),Locator.parse("importOLAP"), stringTojdom(config[0].getResult()));
		//System.out.println(ConfigManager.getInstance().getConfiguration(Locator.parse("importOLAP")));
		ConfigManager.getInstance().remove(Locator.parse("importOLAP"));
		if (ConfigManager.getInstance().getComponent(Locator.parse("importOLAP"), null) == null)
			System.out.println("removed");
		System.exit(0);
	}


	private static void getStatus() throws Exception {
		ServerStatus status = service.getServerStatus();
		System.out.println(status.getMaxMemory());
		System.out.println(status.getTotalMemory());
		System.out.println(status.getFreeMemory());
		System.out.println(status.getLogLevel());
		System.out.println(status.getProcessorsAvailable());
		System.out.println(status.getRunningExecutions());
		for (String store : status.getDatastores())
			System.out.println(store);
	}

	private static void getOutputDescription(String locator, String format) throws Exception {
		ComponentOutputDescriptor[] ds = service.getComponentOutputs(locator,null,format,true).getMetadata();
		System.out.println(locator);
		for (ComponentOutputDescriptor d : ds) {
			System.out.println(" "+d.getName());
			System.out.println("  "+d.getOriginalName());
			System.out.println("  "+d.getPosition());
			System.out.println("  "+d.getRole());
			System.out.println("  "+d.getType());
			System.out.println("  "+d.getDefaultValue());;
		}
	}

	private static void testComponent(String locator) throws Exception {
		ExecutionDescriptor d  = service.testComponent(locator,null,true);
		System.out.println(d.getStatus());
	}

	private static void getMetadata(String locator, Variable[] variables) throws Exception {
		ExecutionDescriptor d = service.getMetadata(locator, variables, null);
		if (d.getValid())
			System.out.println(d.getResult());
		else 
			System.err.println(d.getErrorMessage());
	}

	private static void printDependencies(String qname, Map<String,List<String>> graph, String indent) {
		List<String> list = graph.get(qname);
		if (list != null) {
			for (String s : list) {
				System.out.println(indent+qname+" <- "+s);
				printDependencies(s,graph,indent+"  ");
			}
		}
	}

	private static void printDependents(String qname, Map<String,List<String>> graph, String indent) {
		List<String> list = graph.get(qname);
		if (list != null) {
			for (String s : list) {
				System.out.println(indent+qname+" -> "+s);
				printDependents(s,graph,indent+"  ");
			}
		}
	}

	private static Map<String,List<String>> depDescriptorsToMap(ComponentDependencyDescriptor[] deps) {
		Map<String,List<String>> result = new HashMap<String,List<String>>();
		for (ComponentDependencyDescriptor d : deps) {
			List<String> list = new ArrayList<String>();
			for (String s : d.getComponents())
				list.add(s);
			result.put(d.getName(), list);
		}
		return result;
	}

	private static void getComponentGraph(String locator) throws Exception {
		ComponentDependencyDescriptor[] in = service.getComponentDependencies(locator);
		ComponentDependencyDescriptor[] out = service.getComponentDependents(locator);
		Map<String,List<String>> inMap = depDescriptorsToMap(in);
		Map<String,List<String>> outMap = depDescriptorsToMap(out);
		printDependencies(locator,inMap,"");
		printDependents(locator,outMap,"");
	}

	
	private static Variable newVariable(String name, String value) {
		Variable v = new Variable();
		v.setName(name);
		v.setValue(value);
		return v;
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
	try {
		//testConfigManager();
		// TODO Auto-generated method stub
	/*
		String[] names = { "Customer","Year","Month","Currency","Measures" };
		String[] values = { "1005","2006","1.2","EUR","Units" };
	*/
		String config = null;
		String drillthrough = "Biker_ETL.Orders2";
		String[] names = { "Years","Months","Datatypes","Measures","Customers","Channels","Orderlines","Products" };
		String[] values = { "2008","Feb","Actual","Units", "Demand Distributors","Fax/Phone/Mail","203010","Cross-150 Silver 56" };
		String[] values2 = { "2008","Jan","Actual","Units", "Demand Distributors","Fax/Phone/Mail","203010","Cross-150 Silver 56","2008","Feb","Actual","Units", "Demand Distributors","Fax/Phone/Mail","203010","Cross-150 Silver 56" };
		//String[] names = { "Years", "Measures"};
		//String[] values = {"2008", "Units", "2008", "Sales"};

//		String[] names = { "Customer","Year","Month","Currency","Measures" };
//		String[] values = { "10005","2006","1.2","EUR","Units" };

//		getConfiguration();

//		Persistence.getInstance().getQueryResult("select * from Basic", 0);

		String project = "importRelDB";
		LogManager.getInstance().setLevel(Settings.getInstance().getContext(Settings.ProjectsCtx).getProperty("loglevel"));
		//LogManager.getInstance().setLevel("DEBUG");

		config = readFileAsString("data/samples/"+project+".xml");
		addComponents(new String[] {project},new String[] {config});

		//getComponentGraph(project+".extracts.Customer_Extract");

//		validateConfigs(new String[] {project},new String[] {config});
		//config = readFileAsString("/Users/chris/Documents/workspace/etlclient/target/samples/importRelDB.xml");

//		getComponentSchemas(new String[]{"projects.default"});
//		getComponentTypes("extracts");
//		validateConfigs(new String[] {project},new String[]{config});

//		getComponentNames(null);
//		getComponentLocators(project+".jobs");
		//getComponentSchemas(new String[]{"transformers.default"});

//		List<Variable> v = new ArrayList<Variable>();
//		v.add(newVariable("selector","cube"));
//		v.add(newVariable("database","Demo"));
//		getMetadata(project+".connections.palodemo",v.toArray(new Variable[v.size()]));
		
		Variable syncModeGroup = new Variable();
		syncModeGroup.setName(NamingUtil.internal("syncMode"));
		syncModeGroup.setValue(SyncModes.PARALLEL.toString());
		
		Variable syncModeSingle = new Variable();
		syncModeSingle.setName(NamingUtil.internal("syncMode"));
		syncModeSingle.setValue(SyncModes.SINGLE.toString());
		
		List<String> locators = new ArrayList<String>();
		locators.add("importRelDB.sources.Sales_Extract");
		locators.add("importRelDB.jobs.default");
		locators.add("importRelDB.sources.Sales_Extract");
		locators.add("importRelDB.sources.Sales_Extract");
		
		//getComponentOutline
		ArrayList<ExecutionDescriptor> descriptors = new ArrayList<ExecutionDescriptor>();
		for (String locator : locators) {
			ExecutionDescriptor e = service.addExecution(locator, new Variable[]{locator.endsWith("default")?syncModeSingle:syncModeGroup});
			if (e.getValid()) {
				descriptors.add(e);
				service.runExecution(e.getId());
				//queryExecutions(null,null,null,0,0,null);
			}
		}
		int i = 0;
		for (ExecutionDescriptor d : descriptors) {
			i++;
			//queryExecutions(null,null,null,0,0,"0 5");
			//service.stopExecution(d.getId());
			if (i == 2) {
				Thread.currentThread().sleep(2000);
				service.stopExecution(d.getId());
			}
			d = service.getExecutionStatus(d.getId(), true);
			//d = service.removeExecutions(new ExecutionDescriptor[]{d})[0];
			System.out.println(d.getId()+" "+d.getStatus());
			System.out.println(d.getErrors());
			System.out.println(d.getErrorMessage());
			System.out.println(service.getExecutionLogPaged(d.getId(), "INFO", null, 0,5).getResult());
			System.out.println("Page2:");
			System.out.println(service.getExecutionLogPaged(d.getId(), "INFO", null, 5,0).getResult());
			//System.out.println("log: "+service.getExecutionLog(d.getId(),null,null).getResult());
		}
		
		
//		getComponentConfigs(new String[]{"importRelDB"});

//		getComponentTypes("extracts");

		//project+".sources.Sales_Normalize",

//		executeComponents(new String[]{project+".jobs.default"});
//		executeComponents(new String[]{project+".jobs.default"});
//		fetchData(project+".sources.Sales_Normalize","PCW",100);

//		testComponent(project+".jobs.default");

//		getComponentConfigs(new String[]{project+".extracts.Customer_Extract"});
//		getOutputDescription(project+".extracts.Customer_Extract",null);




//		getOutputDescription(project+".sources.Sales_Trans","PCWA");
//		getOutputDescription(project+".sources.Customer_Source","EA");
//		getOutputDescription(project+".sources.Year","NCWA");
//		getOutputDescription(project+".sources.CustomerRegions","FHW");

//		getData(project+".sources.Customer_Tree","PCWA");
//		getData(project+".sources.Customer_Source","EA");
//		getData(project+".sources.Year","NCWA");
//		getData(project+".sources.Month","FHW");

/*
		GregorianCalendar start = new GregorianCalendar();
		start.set(2009, 4, 27, 13, 14, 0);
		GregorianCalendar stop = new GregorianCalendar();
		stop.set(2009, 4, 27, 14, 14, 0);
*/
//		queryExecutions(null,null,null,0,0,null);
//		getExecutionLog(1867776);


//		getData(project+".sources.Customer_Tree");
//		drillThroughBulk(drillthrough, names, values,10);
//		drillThroughBulk(drillthrough, names, values2,10);
//		uploadFile("test","Ich bin ein Test.".getBytes());

//		queryExecutions("import*","*","*fault",0,new Date().getTime(),"10 20");
		//getExecutionLog("327680");


//		String[] pipelines = getComponentLocators("importRelDB.pipelines");
//		getComponentOutlines(pipelines);

//		testComponents(new String[] {"importBikerDrillthrough.jobs.default"});

		//getOutputDescription(project+".sources.CUSTOMER");
		//getOutputDescription(project+".sources.Year");
//		getOutputDescription(project+".transforms.Sales_Normalize");
//		getOutputDescription(project+".exports.DemoSales");

//		getComponentSchemas(new String[]{"sources.SQL"});
//		ConfigManager.getInstance().shutDown();
	}
	catch (Exception e) {
		System.err.println(e.getMessage());
		e.printStackTrace();
	}
		System.out.flush();
		System.exit(0);
	}

}
