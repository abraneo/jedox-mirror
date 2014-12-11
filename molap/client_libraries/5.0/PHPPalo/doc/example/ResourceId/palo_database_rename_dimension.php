<?php
	
	// create the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);
	
	// create the variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';
	$new_dimension_name = 'Warehouse';
	  
	// rename dimension 
	palo_database_rename_dimension($connection_id, $db_name,
								   $dimension_name, $new_dimension_name); 
	
	// disconnect the connection to server
	palo_disconnect($connection_id);
?>
