<?php
	
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	

	// create the variables db_name and dimension
	$db_name = 'Demo';
	$dimension_name = 'Products';
	 		
	// Returns the maximal consolidation level in the dimension Products
	palo_etoplevel($connection_id, $db_name, $dimension_name);

	// disconnect the connection to server
	palo_disconnect($connection_id); 
?>
