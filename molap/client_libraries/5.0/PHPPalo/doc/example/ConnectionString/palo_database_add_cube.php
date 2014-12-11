<?php
  // define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
   	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// create variables db_name and cube_name
	$db_name = "Demo";
	$cube_name = "SalesTest";
	
	// create array cube_data
	$cube_data = array('Products','Regions','Months','Years','Datatypes','Measures');
	
	// add cube SalesTest
	palo_database_add_cube(CONN . '/' . $db_name, $cube_name, $cube_data);
?>
