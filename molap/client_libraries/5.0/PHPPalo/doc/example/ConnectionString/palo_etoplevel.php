<?php
	
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// create the variables db_name and dimension
	$db_name = 'Demo';
	$dimension_name = 'Products';
	 		
	// Returns the maximal consolidation level in the dimension Products
	palo_etoplevel(CONN . '/' . $db_name, $dimension_name);
?>
