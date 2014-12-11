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
	$db_name = 'Demo';
	$cube_name = 'Sales';
	
	// load the cube Sales
	palo_database_load_cube(CONN . '/' . $db_name, $cube_name);
?>
