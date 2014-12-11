<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// define variables as follows	
	$db_name = 'Demo';
	
	// list all cubes in the specified database		
	palo_database_list_cubes($connection_id, $db_name, 0);

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
