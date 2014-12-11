<?php
	
	// create the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id_id = palo_init(HOST, PORT, USER, PASS);
	 
	// create the variable db_name
	$db_name = 'Demo';
	
	// list all dimensions in the database Demo			
	palo_database_list_dimensions($connection_id_id, $db_name, 0);

	// disconnect the connection to server
	palo_disconnect($connection_id_id);
?>

