<?php 

	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');

	// registers a name for a PALO server  
	$connection_id = palo_init(HOST, PORT, USER, PASS);	

	// disconnect
	palo_disconnect($connection_id);

	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');

	// define the variables
	$db_name = 'Demo';
	$cube_name = 'Sales';

	// build connection    
	$connection_id = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// clear the cube
	palo_cube_clear($connection_id, $db_name, $cube_name);

	// diconnect
	palo_disconnect($connection_id); 
?>