<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// define the variables
	$db_name = 'Demo';
	$dimension_name = 'Products';	 	

	// Retrieve a subset of dimension elements 
	palo_dimension_info($connection_id, $db_name, $dimension_name);

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
