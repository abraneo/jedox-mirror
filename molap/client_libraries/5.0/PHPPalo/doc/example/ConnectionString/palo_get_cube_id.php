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
	$cube_name = 'Sales'; 

	// get the id for $cube_name
	palo_get_cube_id(CONN . '/' . $db_name, $cube_name); 
?>

