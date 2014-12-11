<?php

	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// define the variables as follows
	$db_name = 'Demo';
	$dimension_id = 12;

	// get the name for $dimension_id
	print palo_get_dimension_name(CONN . '/' . $db_name, $dimension_id); 
?>

