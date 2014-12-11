<?php
	
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	    
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// create the variable db_name
	$db_name = 'Demo';
	
	// add dimension 
	palo_database_add_dimension(CONN . '/' . $db_name, 'TestDimension');
?>
