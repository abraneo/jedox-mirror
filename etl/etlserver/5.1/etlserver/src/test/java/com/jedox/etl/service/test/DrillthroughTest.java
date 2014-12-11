package com.jedox.etl.service.test;


import java.rmi.RemoteException;

import org.junit.BeforeClass;
import org.junit.Test;

import com.jedox.etl.service.ETLService;
import com.jedox.etl.service.ResultDescriptor;

@SuppressWarnings("unused")
public class DrillthroughTest {

	private static ETLService service = null;


	private static final String datastore = "Biker_ETL.Orders2";
	private static final String[] names = { "Years","Months","Datatypes","Measures","Customers","Channels","Orderlines","Products" };
	private static final String[] values = { "2008","Feb","Actual","Units", "Demand Distributors","Fax/Phone/Mail","203010","Cross-150 Silver 56" };
	//values = { null,null,null,null,null,null,null,null};

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		service = new ETLService();
	}

	@Test
	public void testgetServerStatus() throws RemoteException {
		System.out.println("... Starting test Drillthrough ...");
		// ToDo: add and execute projects importBiker and importBikerDrillthrough
//		ResultDescriptor drillresponse = service.drillThrough(datastore, names, values, 100);
//		System.out.println("Drillthrough Result:");
//		System.out.println(drillresponse.getResult());
	}


}
