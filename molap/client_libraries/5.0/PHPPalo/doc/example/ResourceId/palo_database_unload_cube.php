<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id = palo_init(HOST, PORT, USER, PASS);

	// create the variables $db_name & $cubename
	$db_name = "Demo";
	$cubename = "Sales";

	// unload the cube "Sales"
	palo_database_unload_cube($connection_id, $db_name, $cubename);

	// disconnect the connection
	palo_disconnect($connection_id);
?>