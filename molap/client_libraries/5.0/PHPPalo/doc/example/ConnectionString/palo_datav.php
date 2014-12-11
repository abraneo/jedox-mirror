<?php
	
	// define the constants			
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// create the variables db_name and cube_name
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	// specify the coordinates for the second datav
	// pick elements Desktop L and Desktop Pro
	$products = array(2,1,"Desktop L","Desktop Pro");
	// pick region Germany and France
	$region = array(2,1,"Germany","France");
	// pick months of the firs quarter
	$months = array(2,1,"Qtr. 1","");
	// pick the year 2005
	$years = array(2,1,"2005","");	

	// retrieves the values of the above specified coordinates
	palo_datav(CONN . '/' . $db_name, $cube_name, 
	$products, $region, $months, $years,"All Datatypes","Units");

	// the first datav retrieve value of "All Products" 
	//at the specified coordinates 
	palo_datav(CONN . '/' . $db_name, $cube_name, "All Products", 
			  array(5, 1, "Europe", "West", "East", "South", "North"), 
			  array(1, 5, "Year", "Qtr. 1", "Qtr. 2", "Qtr. 3", "Qtr. 4"),
			  "2002", "Budget", "Units"); 
?>
