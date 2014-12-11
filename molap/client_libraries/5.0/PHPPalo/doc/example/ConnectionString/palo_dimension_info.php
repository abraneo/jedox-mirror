<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// define the variables
	$db_name = 'Demo';
	$dimension_name = 'Products';	 	

	// Retrieve a subset of dimension elements 
	palo_dimension_info(CONN . '/' . $db_name, $dimension_name);
?>
