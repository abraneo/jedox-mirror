<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server	
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);
	
	// create variables db_name and cube_name
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	// load the cube Sales
	palo_database_load_cube($connection_id_id, $db_name, $cube_name);

	// disconnect the connection to server
	palo_disconnect($connection_id)_id;
?>
