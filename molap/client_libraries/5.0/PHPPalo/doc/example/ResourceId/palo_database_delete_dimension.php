<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	    
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create the variable db_name
	$db_name = 'Demo';
	
	// delete dimension Years
	palo_database_delete_dimension($connection_id, $db_name, 'TestDimension');

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
