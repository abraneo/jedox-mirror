/**
 * Test cases for Class ETLService
 */
package com.jedox.etl.service.test;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import org.apache.axis2.AxisFault;
import org.junit.*;

import com.jedox.etl.service.ComponentOutputDescriptor;
import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.config.XMLReader;
import com.jedox.etl.core.util.XMLUtil;
import com.jedox.etl.service.ComponentDependencyDescriptor;
import com.jedox.etl.service.ComponentTypeDescriptor;
import com.jedox.etl.service.ETLService;
import com.jedox.etl.service.ExecutionDescriptor;
import com.jedox.etl.service.ResultDescriptor;
import com.jedox.etl.service.ServerStatus;
import com.jedox.etl.service.Variable;

/**
 * @author khaddadin
 *
 */
public class ETLServiceTest {

	private static ETLService service = null;
	private static String targetFileName; // the file_name used in the UploadFile method

	private static String projectName= ""; // the project name (eg. importBiker)	
	private static String jobName= ""; // the job name (eg. default)
	private static String projectName_withPath = ""; // XML file that correspond to a certain project at the client side
	private static String config = ""; // XML tag that correspond to a certain component (project,connection, .. ,etc)
	private static String componentNameinProjectName= ""; // a component in project with name projectName
	private static String olapconnectioninProjectName= ""; // a olap connection in project with name projectName
	private static String relconnectioninProjectName= ""; // a rel connection in project with name projectName
	private static String component_or_manager_NameinProjectName= ""; // a component in project with name projectName or a manager e.g project1.extracts

/*
	private static String projectName2= ""; // the project name (eg. importBiker)
	private static String projectName2_withPath = ""; // XML file that correspond to a certain project at the client side
	private static String config2 = ""; // XML tag that correspond to a certain component (project,connection, .. ,etc)
	private static String componentNameinProjectName2= ""; // a component in project with name projectName
	private static String olapconnectioninProjectName2= ""; // a olap connection in project with name projectName
	private static String relconnectioninProjectName2= ""; // a rel connection in project with name projectName
	private static String component_or_manager_NameinProjectName2= ""; // a component in project with name projectName2 or a manager e.g project2.extracts
*/

	private static String generalScope = ""; // general scope that is connected to a certain project
	private static String olapdatabaseName = ""; // olapdatabaseName
	private static String olapCubeName = ""; // olapdatabaseName
	private static String reldatabaseName = ""; // reldatabaseName
	private static String db_mask = "1000"; // database mask
	private static String dim_mask = ""; // dimension mask
	private static Hashtable<Integer, String> do_tests = new Hashtable<Integer, String>();
	private static String just_source = "";
	private static String single_component_config = "";

	private static void chooseTests(){
		//do_tests.put(1, "testgetServerStatus");
		//do_tests.put(2, "testUploadFile");
		do_tests.put(3, "testaddComponents");
		//do_tests.put(4, "testgetRelationalCatalog");
		//do_tests.put(5, "testgetRelationalColumns");
		//do_tests.put(6, "testgetComponentConfigs");
		//do_tests.put(7, "testgetComponentOutputs");
	    //do_tests.put(8, "testgetComponentTypes");
		//do_tests.put(9, "testtestComponents");
		//do_tests.put(10, "testgetLocators");
		//do_tests.put(11, "testgetNames");
		//do_tests.put(12, "testgetOlapCubes");
		//do_tests.put(13, "testgetOlapCubeDimensions");
		//do_tests.put(14, "testgetOlapDimensions");
		//do_tests.put(15, "testgetOlapDatabases");
		//do_tests.put(16, "testgetRelationalTables");
		//do_tests.put(17, "testremoveComponents");
		//do_tests.put(18, "testsave");
		do_tests.put(19, "testAddRunExecution");
		//do_tests.put(20, "testgetExecutionHistory");
		//do_tests.put(21, "testvalidateComponents");
		//do_tests.put(22, "testgetData");
		//do_tests.put(23, "testgetScopes");
		//do_tests.put(24, "testgetRelationalSchemas");
		//do_tests.put(25, "testMigrateComponents");
		//do_tests.put(26, "testgetExecutionLog");

		//do_tests.put(27, "testgetExecutionStatus");
		//do_tests.put(28,"testremoveExecution");

		//do_tests.put(28, "testPrepareData");//TO DO
		//do_tests.put(29, "testFetchData");//TO DO
		do_tests.put(30, "testDrillthrough");
		//do_tests.put(31, "testStopExecution");//TO DO
		
		//do_tests.put(28,"testgetComponentGraph");
		//do_tests.put(29,"testgetComponentDependencies");
		//do_tests.put(30,"testgetComponentDependents");	
		do_tests.put(31, "testgetMetadata");
	}

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		chooseTests();
		service = new ETLService();
		targetFileName = "target_file.txt";
		projectName = "importBiker";
		//projectName = "import_RM_WB_File_32";
		jobName = "default";
		projectName_withPath = "../../etlstandalone/target/samples/"+projectName+ ".xml";
		XMLReader reader = new XMLReader();
		config = XMLUtil.jdomToString(reader.readDocument(projectName_withPath).getRootElement());
		
		// componentNameinProjectName = ".jobs.default";
		// componentNameinProjectName = ".connections.Kapital";
		componentNameinProjectName = ".transforms.Customers";
		// componentNameinProjectName = ".extracts.CustomerRegions";
		// component_or_manager_NameinProjectName = ".transforms";

/*
		projectName2 = "importRelDB";
		projectName2_withPath = "../../etlstandalone/target/samples/"+projectName2+ ".xml";
		config2 = XMLUtil.jdomToString(reader.readDocument(projectName2_withPath).getRootElement());
		componentNameinProjectName2 = ".extracts.Customer_Source";
		component_or_manager_NameinProjectName2 = ".extracts";
*/
		generalScope = "loads";
		olapdatabaseName = "Biker_ETL";
		reldatabaseName = "./BikerOrderlines.csv";
		olapconnectioninProjectName = ".connections.biker";
		relconnectioninProjectName = ".connections.Orderlines_file";
/*
		relconnectioninProjectName2 = ".connections.MySQL_Conn";
		olapconnectioninProjectName2 = ".connections.palodemo";
*/
		olapCubeName = "Orders";
		db_mask = "1000";
		dim_mask = "111";

		//single_component_config = XMLUtil.jdomToString(reader.readDocument(just_source).getRootElement());
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
		ExecutionDescriptor[] eds =  service.getExecutionHistory(null, null, null, 0, 0, "5");
		for(ExecutionDescriptor ed: eds)
			service.stopExecution(ed.getId());
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testgetServerStatus() throws AxisFault {

		if (do_tests.contains("testgetServerStatus")) {
			System.out.println("... Starting testing getServerStatus() ...");
			ServerStatus currentStatus = service.getServerStatus();
			System.out
			.println("Total Memory " + currentStatus.getTotalMemory());
			System.out.println("Free Memory " + currentStatus.getFreeMemory());
			System.out.println("Max Memory " + currentStatus.getMaxMemory());
			System.out.println("Running Executions "
					+ currentStatus.getRunningExecutions());
			System.out.println("LogLevel " + currentStatus.getLogLevel());
			System.out.println("Data stores number "
					+ currentStatus.getDatastores().length);
			System.out.println("... end testing getServerStatus() ...");
		}
		else
		{
			System.out.println("testgetServerStatus is not performed!");
		}
	}

	@Test
	public void testAddRunExecution() throws AxisFault, InterruptedException {

		if (do_tests.contains("testAddRunExecution")) {
			System.out.println("... Starting testing AddRunExecution() ...");
			ExecutionDescriptor ed = service.addExecution(projectName+".jobs."+jobName, null);
			System.out.println("after adding: status is " + ed.getStatus() );
			ExecutionDescriptor ed2 =  service.runExecution(ed.getId());
			System.out.println("after termination: status is " + service.getExecutionStatus(ed.getId(), true).getStatus());
			System.out.println("... end testing AddRunExecution() ...");
		}
		else
		{
			System.out.println("testAddRunExecution is not performed!");
		}
	}

	@Test
	public void testUploadFile() {

		if (do_tests.contains("testUploadFile")) {
			System.out.println("... Starting testing UploadFile() ...");
			service.uploadFile(targetFileName, targetFileName.getBytes());
			System.out.println("... end testing UploadFile() ...");
		}
		else
		{
			System.out.println("testUploadFile is not performed!");
		}
	}

	@Test
	public void testMigrateComponents() throws IOException, ConfigurationException {

		if (do_tests.contains("testMigrateComponents")) {
			System.out.println("Start MigrateComponents:");
			XMLReader reader = new XMLReader();
			String config_test = XMLUtil.jdomToString(reader .readDocument("../../etlstandalone/target/samples/exportOLAPtoFile2.xml").getRootElement());;
			ResultDescriptor[] result = service.migrateComponents(new String[] { config_test });
			if(result[0]!= null) System.out.println(result[0].getResult());
			//String[] locators1 = service.getLocators(projectName2+".extracts");
			//result = service.addComponents(new String[]{projectName2+".extracts.bla"} , new String[]{single_component_config});
			System.out.println("end MigrateComponents");
		}
		else
		{
			System.out.println("testMigrateComponents is not performed!");
		}
	}

	@Test
	public void testExecutionStatus() throws AxisFault {

		if (do_tests.contains("testgetExecutionStatus")) {
			System.out.println("Start testExecutionStatus:");
			ExecutionDescriptor result = service.getExecutionStatus((long)163840,false);
			System.out.println(result.getStatus());
			System.out.println("end testgetExecutionStatus");
		}
		else
		{
			System.out.println("testExecutionStatus is not performed!");
		}
	}

	@Test
	public void testremoveExecution() throws IOException, ConfigurationException {

		if (do_tests.contains("testremoveExecution")) {
			System.out.println("Start testremoveExecution:");
			ExecutionDescriptor[] result = service.removeExecutions(new Long[]{(long)163840});
			if(result[0]!= null) System.out.println(result[0].getResult());
			System.out.println("end testremoveExecution");
		}
		else
		{
			System.out.println("testremoveExecution is not performed!");
		}
	}
	@Test
	public void testaddComponents() throws AxisFault {

		if (do_tests.contains("testaddComponents")) {
			System.out.println("Start addComponents:");
			ResultDescriptor[] result = service.addComponents(new String[] { projectName }, new String[] { config });
			if(result[0]!= null) System.out.println(result[0].getResult());
			//String[] locators1 = service.getLocators(projectName2+".extracts");
			//result = service.addComponents(new String[]{projectName2+".extracts.bla"} , new String[]{single_component_config});
			System.out.println("end addComponents:");
		}
		else
		{
			System.out.println("testaddComponents is not performed!");
		}
	}

	@Test
	public void testgetScopes() throws AxisFault{
		if (do_tests.contains("testgetScopes")) {
			System.out.println("getScopes :");
			String [] s = service.getScopes();
			for(String ss:s)
				System.out.println(ss);
		}
		else
		{
			System.out.println("testgetScopes is not performed!");
		}
	}

	@Test
	public void testgetRelationalCatalogs() throws AxisFault{
		if (do_tests.contains("testgetRelationalCatalog")) {
			System.out.println("getRelationalCatalog from "+ projectName
					+ relconnectioninProjectName);
			String[][] nameValuePairs = { { "selector", "catalog" } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ relconnectioninProjectName, v, null);
			System.out.println(execution.getResult());						
		}
		else
		{
			System.out.println("testgetRelationalCatalog is not performed!");
		}
	}

	@Test
	public void testgetRelationalSchemas() throws AxisFault{
		if (do_tests.contains("testgetRelationalSchemas")) {
			System.out.println("getRelationalSchemas from "+ projectName
					+ relconnectioninProjectName);
			String[][] nameValuePairs = { { "selector", "schema" } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ relconnectioninProjectName, v, null);
			System.out.println(execution.getResult());						
		}
		else
		{
			System.out.println("testgetRelationalSchemas is not performed!");
		}
	}

	@Test
	public void testgetRelationalColumns() throws AxisFault{
		if (do_tests.contains("testgetRelationalColumns")) {
			System.out.println("getRelationalColumns from "+ projectName
					+ relconnectioninProjectName);
			String[][] nameValuePairs = { { "selector", "column" } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ relconnectioninProjectName, v, null);
			System.out.println(execution.getResult());						
		}
		else
		{
			System.out.println("testgetRelationalColumns is not performed!");
		}
	}

	@Test
	public void testgetComponentConfigs() throws AxisFault{
		if (do_tests.contains("testgetComponentConfigs")) {
			System.out.println("... start testing getComponentConfigs() ...");
			System.out.println("... getComponentConfigs from " + projectName
					+ componentNameinProjectName);
			ResultDescriptor[] componentConfigs = service
			.getComponentConfigs(new String[] { projectName
					+ componentNameinProjectName });
			System.out.println(componentConfigs[0].getResult());
			System.out.println("... end testing getComponentConfigs() ...");
		}
		else
		{
			System.out.println("testgetComponentConfigs is not performed!");
		}
	}

	@Test
	public void testgetComponentOutputs() throws AxisFault{
		if (do_tests.contains("testgetComponentOutputs")) {
			System.out.println("... start testing getComponentOutputs() ...");
			System.out.println("... getComponentConfigs from " + projectName
					+ componentNameinProjectName);
			ExecutionDescriptor executionDescriptors = service.getComponentOutputs(projectName
											+ componentNameinProjectName, null, "pcwa",true);
			//ExecutionDescriptor executionDescriptors = service.getComponentOutputs("importRelDB.extracts.Currency", null, "PCWA");
			System.out.println("Name: " + executionDescriptors.getName() + " - "
						+ "Result: " + executionDescriptors.getMetadata());
			for(ComponentOutputDescriptor cod:executionDescriptors.getMetadata()){
				System.out.print(cod.getName());
				System.out.println( "   with origion:"+ cod.getOriginalName());
			}
			if(executionDescriptors.getErrors()!=0){
				System.out.println(" An error occurred" + executionDescriptors.getErrorMessage());
			}
			System.out.println("... end testing getComponentOutputs()) ...");
		}
		else
		{
			System.out.println("testgetComponentOutputs is not performed!");
		}
	}

	@Test
	public void testgetComponentTypes() throws AxisFault{
		if (do_tests.contains("testgetComponentTypes")) {
			System.out.println("... start testing getComponentTypes() ...");
			System.out.println("... getComponentTypes from " + generalScope);
			ComponentTypeDescriptor[] possibleComponents = service.getComponentTypes(generalScope);
			for (ComponentTypeDescriptor ComponentTypeDescriptor : possibleComponents) {
				System.out.print(ComponentTypeDescriptor.getType() + " : is tree: " + ComponentTypeDescriptor.isTree() + " - ");
			}
			System.out.println("\n... end testing getComponentTypes() ...");
		}
		else
		{
			System.out.println("testgetComponentTypes is not performed!");
		}
	}

	@Test
	public void testtestComponents() throws AxisFault{
		if (do_tests.contains("testtestComponents")) {
			System.out.println("... start testing testComponents() ...");
			System.out.println("... testComponents() from " + projectName
					+ componentNameinProjectName);
			//System.out.println("importRelDB.connections.MySQL_Conn");
			ExecutionDescriptor testResult = service.testComponent(projectName+ componentNameinProjectName,null,true);
			//ExecutionDescriptor testResult = service.testComponent("importRelDB.connections.MySQL_Conn",null);
			System.out.print(testResult.getStatus() + " - ");
			System.out.print(testResult.getErrorMessage());			
			System.out.println("\n... end testing testComponents() ...");
		}
		else
		{
			System.out.println("testtestComponents is not performed!");
		}
	}

	@Test
	public void testgetLocators() throws AxisFault{
		if (do_tests.contains("testgetLocators")) {
			System.out.println("... start testing getLocators ...");
			System.out.println("... getLocators from " + projectName
					+ component_or_manager_NameinProjectName);
			String[] locators = service.getLocators(projectName
					+ component_or_manager_NameinProjectName);
			for (String component : locators) {
				System.out.print(component + " - ");
			}
			System.out.println("\n... end testing getLocators ...");
		}
		else
		{
			System.out.println("testgetLocators is not performed!");
		}
	}

	@Test
	public void testsave() throws AxisFault{
		if (do_tests.contains("testsave")) {
			System.out.println("... start testing save ...");
			System.out.println("... save  " + projectName); // a component should work too!
			ResultDescriptor result = service.save(projectName);
			System.out.println("... save  was done  in " + result.getResult());
			System.out.println("\n... end testing save ...");
		}
		else
		{
			System.out.println("testgetLocators is not performed!");
		}
	}

	@Test
	public void testremoveComponents() throws AxisFault{
		if (do_tests.contains("testremoveComponents")) {
			System.out.println("... start testing testremoveComponents ...");
			System.out.println("... testremoveComponents from " + projectName
					+ componentNameinProjectName);
			ResultDescriptor[] result = service.removeComponents(new String[]{projectName
					+ componentNameinProjectName});
			if(result[0].getValid()){
				System.out.println("removing from " + projectName
						+ componentNameinProjectName + " is successful.");
			}else{
				System.out.println("removing from " + projectName
						+ componentNameinProjectName + " is NOT successful.");

			}
			System.out.println("\n... end testing getLocators ...");
		}
		else
		{
			System.out.println("testremoveComponents is not performed!");
		}
	}

	@Test
	public void testgetNames() throws AxisFault{
		if (do_tests.contains("testgetNames")) {
			System.out.println("... start testing getNames() ...");
			System.out.println("... getNames from " + projectName
					+ component_or_manager_NameinProjectName);
			String[] possibleComponents = service.getNames(projectName
					+ component_or_manager_NameinProjectName);
			for (String component : possibleComponents) {
				System.out.print(component + " - ");
			}
			System.out.println("\n... end testing getNames() ...");
		}
		else
		{
			System.out.println("testgetNames is not performed!");
		}
	}

	private Variable[] getVariablesMetadata(String[][] nameValuePairs) {
		ArrayList<Variable> list = new ArrayList<Variable>();
		for (String[] nameValuePair : nameValuePairs) {
			Variable v = new Variable();
			v.setName(nameValuePair[0]);
			v.setValue(nameValuePair[1]);			
			list.add(v);
		}
		if (list.size() == 0)
			return null;
		return list.toArray(new Variable[list.size()]);		
	}
	
	@Test
	public void testgetOlapCubes() throws AxisFault{
		if (do_tests.contains("testgetOlapCubes")) {
			System.out.println("... start testing getOlapCubes() ...");
			System.out.println("... getOlapCubes from " + projectName
					+ olapconnectioninProjectName + " with db "
					+ olapdatabaseName + " with mask " + db_mask);
//			ResultDescriptor cube = service.getOlapCubes(projectName
//					+ olapconnectioninProjectName, db_mask);

			String[][] nameValuePairs = { { "selector", "cube" }, {"database", olapdatabaseName }, {"mask", db_mask } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor cube = service.getMetadata(projectName
			+ olapconnectioninProjectName , v, null);
			System.out.println(cube.getResult());
			System.out.println("... end testing getOlapCubes() ...");
		}
		else
		{
			System.out.println("testgetOlapCubes is not performed!");
		}
	}

	@Test
	public void testgetOlapCubeDimensions() throws AxisFault{
		if (do_tests.contains("testgetOlapCubeDimensions")) {
			System.out.println("... start testing getOlapCubeDimensions() ...");
			System.out.println("... getOlapCubeDimensions from " + projectName
					+ olapconnectioninProjectName + " with db "
					+ olapdatabaseName + " with cube "+ olapCubeName + " with mask " + dim_mask);
			
			String[][] nameValuePairs = { { "selector", "dimension" }, {"database", "Demo"}, {"mask", "000" } };			
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ olapconnectioninProjectName, v, null);
			System.out.println(execution.getResult());
			System.out.println("... end testing getDimensions() ...");
		}
		else
		{
			System.out.println("testgetDimensions is not performed!");
		}

	}

	@Test
	public void testgetOlapDimensions() throws AxisFault{
		if (do_tests.contains("testgetOlapDimensions")) {
			System.out.println("... start testing getOlapDimensions() ...");
			System.out.println("... getCubeDimensions from " + projectName
					+ olapconnectioninProjectName + " with db "
					+ olapdatabaseName  + " with mask " + dim_mask);
			
			String[][] nameValuePairs = { { "selector", "dimension" }, {"database", olapdatabaseName }, {"mask", "000" } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ olapconnectioninProjectName, v, null);
			System.out.println(execution.getResult());
			System.out.println("... end testing testgetOlapDimensions() ...");
		}
		else
		{
			System.out.println("testgetCubeDimensions is not performed!");
		}

	}

	@Test
	public void testgetOlapDatabases() throws AxisFault{
		if (do_tests.contains("testgetOlapDatabases")) {
			System.out.println("... start testing getOLAPDatabases() ...");
			System.out.println("... getDatabases from " + projectName
					+ olapconnectioninProjectName + " with mask \"1\"");
			String[][] nameValuePairs = { { "selector", "database" },  {"mask", "1" } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ olapconnectioninProjectName, v, null);
			System.out.println(execution.getResult());
			System.out.println("... end testing getOlapDatabases() ...");
		}
		else
		{
			System.out.println("testgetOlapDatabases is not performed!");
		}
	}

	@Test
	public void testgetRelationalTables() throws AxisFault{
		if (do_tests.contains("testgetRelationalTables")) {
			System.out.println("... start testing getRelationalTables() ...");
			System.out.println("... getTables from " + projectName
					+ relconnectioninProjectName);
			String[][] nameValuePairs = { { "selector", "table" }, {"tableTypes", "TABLE" } };		
			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
			+ relconnectioninProjectName, v, null);
			System.out.println(execution.getResult());			
			System.out.println("... end testing getRelationalTables ...");
		}
		else
		{
			System.out.println("testgetRelationalTables is not performed!");
		}
	}

	@Test
	public void testgetMetadata() throws AxisFault{
		if (do_tests.contains("testgetMetadata")) {
			System.out.println("... start testing getMetadata() ...");
			System.out.println("... getTables from " + projectName
					+ relconnectioninProjectName);
			String[][] nameValuePairs = { { "selector", "selectorname" } };	
//			String[][] nameValuePairs = { { "selector", "selectorfilter" }, {"selectorname", "dimension" } };	

			Variable [] v = getVariablesMetadata(nameValuePairs);	
			ExecutionDescriptor execution = service.getMetadata(projectName
					+ olapconnectioninProjectName , v, null);
			System.out.println(execution.getResult());			
			System.out.println("... end testing getMetadata ...");
		}
		else
		{
			System.out.println("testgetMetadata is not performed!");
		}
	}
	
	
	@Test
	public void testgetExecutionHistory() throws AxisFault{
		if (do_tests.contains("testgetExecutionHistory")) {
			System.out.println("... start testing testgetExecutionHistory() ...");
			System.out.println("... getting History for " + projectName);
			ExecutionDescriptor[] History = service.getExecutionHistory("importBiker", "jobs", "Initdata", 0, 0, "");
			for(ExecutionDescriptor ed:History)
				System.out.println(ed.getId() + " - " + ed.getProject() + " - " + ed.getType() + " - " + ed.getName() + " - " + ed.getStatus());
			System.out.println("... end testing testgetExecutionHistory() ...");
		}
		else
		{
			System.out.println("testgetExecutionHistory is not performed!");
		}
	}

	@Test
	public void testgetExecutionLog() throws AxisFault{
		if (do_tests.contains("testgetExecutionLog")) {
			System.out.println("... start testing testgetExecutionLog() ...");
			System.out.println("... getting Log for " + projectName);
			ResultDescriptor Log = service.getExecutionLog((long)163840, "jobs", (long)0);
			System.out.println(Log.getResult());
			System.out.println("... end testing testgetExecutionHistory() ...");
		}
		else
		{
			System.out.println("testgetExecutionHistory is not performed!");
		}
	}

	@Test
	public void testvalidateComponents() throws AxisFault{
		if (do_tests.contains("testvalidateComponents")) {
			System.out.println("... start testing testvalidateComponents() ...");
			ResultDescriptor[] results = service.validateComponents(new String[]{projectName}, new String[]{config});
			for(ResultDescriptor result: results){
				System.out.println("result Validating " + projectName + " against " + config + " " + result.getResult());
				System.out.println("error Validating " + projectName + " against " + config + " " + result.getErrorMessage());
			}
			System.out.println("... end testing testvalidateComponents() ...");
		}
		else
		{
			System.out.println("testvalidateComponents is not performed!");
		}
	}

	@Test
	public void testgetData() throws AxisFault{
		if (do_tests.contains("testgetData")) {
			System.out.println("... start testing testgetData() ...");
			System.out.println(".. getting data from " + projectName+  componentNameinProjectName);
			ExecutionDescriptor ed = service.getData(projectName+componentNameinProjectName,new Variable[0], "", 20,true);
			System.out.println(ed.getResult());
			System.out.println("... end testing testgetData() ...");
		}
		else
		{
			System.out.println("testgetData is not performed!");
		}
	}

	@Test
	public void testgetComponentDependents() throws AxisFault{
		if (do_tests.contains("testgetComponentDependents")) {
			System.out.println("... start testing getComponentDependents() ...");
			System.out.println("... getComponentDependents from " + projectName
					+ componentNameinProjectName);
			ComponentDependencyDescriptor[] cdds = service.getComponentDependents(
				projectName+componentNameinProjectName);
			
			for (ComponentDependencyDescriptor cdd : cdds) {
				System.out.println("Component: "+cdd.getName());
				for (String s : cdd.getComponents()) {
					System.out.println(" <- "+s);
				}
			}			
			System.out.println("... end testing getComponentDependencies() ...");
		}
		else
		{
			System.out.println("testgetComponentDependencies is not performed!");
		}
	}	
	
	@Test
	public void testgetComponentDependencies() throws AxisFault{
		if (do_tests.contains("testgetComponentDependencies")) {
			System.out.println("... start testing getComponentDependencies() ...");
			System.out.println("... getComponentDependencies from " + projectName
					+ componentNameinProjectName);
			ComponentDependencyDescriptor[] cdds = service.getComponentDependencies(
				projectName+componentNameinProjectName);
			for (ComponentDependencyDescriptor cdd : cdds) {
				System.out.println("Component: "+cdd.getName());
				for (String s : cdd.getComponents()) {
					System.out.println(" -> "+s);
				}
			}			
			System.out.println("... end testing getComponentDependencies() ...");
		}
		else
		{
			System.out.println("testgetComponentDependencies is not performed!");
		}
	}
		
	@Test
	public void testgetComponentGraph() throws AxisFault{
		if (do_tests.contains("testgetComponentGraph")) {
			System.out.println("... start testing getComponentGraphs() ...");

			System.out.println("... getComponentGraph from " + projectName
					+ componentNameinProjectName);
			String[][] nameValuePairs = { { "viewType", "dependencies" } , { "layout", "tree" } };			
			Variable [] v = getVariablesMetadata(nameValuePairs);				
			ResultDescriptor componentGraph = service.calculateComponentGraph(
				projectName+componentNameinProjectName, v);
			if (componentGraph.getValid())
				System.out.println(componentGraph.getResult());
			else
				System.out.println("Error: "+componentGraph.getErrorMessage());
			System.out.println("... end testing getComponentGraph() ...");
		}
		else
		{
			System.out.println("testgetComponentConfigs is not performed!");
		}
	}


	@Test
	public void testDrillthrough() throws AxisFault{

		if (do_tests.contains("testDrillthrough")) {

		projectName = "importBikerDrillthrough";
		projectName_withPath = "../../etlstandalone/target/samples/"+projectName+ ".xml";
//		just_source = "../../etlstandalone/target/samples/xyz.xml";
//		XMLReader reader = new XMLReader();
//		ResultDescriptor[] result =null;
//		try {
//			config = XMLUtil.jdomToString(reader.readDocument(projectName_withPath).getRootElement());
//			result = service.addComponents(new String[] { projectName }, new String[] { config });
//		} catch (Exception e) {
//		}
//
//		projectName = "importBiker";
//		projectName_withPath = "../../etlstandalone/target/samples/"+projectName+ ".xml";
//
//		try {
//			config = XMLUtil.jdomToString(reader.readDocument(projectName_withPath).getRootElement());
//			result = service.addComponents(new String[] { projectName }, new String[] { config });
//		} catch (Exception e) {
//		}
//
//		ExecutionDescriptor ed = service.addExecution("importBiker.jobs.default", null);
//		ExecutionDescriptor i  = service.runExecution(ed.getId());
//		ed = service.addExecution("importBikerDrillthrough.jobs.default", null);
//		ExecutionDescriptor ii  = service.runExecution(ed.getId());
//
//		String datastore = "Biker_ETL.Orders2";
		String datastore = "BIKER_ETL.ORDERS2";
		String[] names = { "Years","Months","Datatypes","Measures","Customers","Channels","Orderlines","Products" };
		String[] values = { "2008","Jan","Feb","Actual","Units", "Demand Distributors","Fax/Phone/Mail","Cross-150 Silver 56" };
		int [] lengths = {1,2,1,1,1,1,0,1};
		//String[] values = {};
		//int [] lengths = {0,0,0,0,0,0,0,0};
	    //	String[] values =null;
		// int [] lengths = {1,1,1,1,1,1,1,1};

		System.out.println("... start testing drillThrough() ...");
		ResultDescriptor drillresponse = service.drillThrough(datastore, names, values, lengths,100);
		System.out.println("Valid: "+drillresponse.getValid());
		System.out.println("Message: "+drillresponse.getErrorMessage());
		System.out.println("Drillthrough Result:");
		System.out.println(drillresponse.getResult());
		}
		else
		{
			System.out.println("drillThrough is not performed!");
	}
	}

}
