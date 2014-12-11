<?php
	
	// define the constants 
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init( HOST, PORT, USER, PASS);	
	
	// create the variables as follows
	$db_name = 'Demo';
	$dimension_name = 'Products';	
	$element_name = 'Desktop L';   
	 		
	// moves element Desktop L to the specified position in dimension Products
	palo_emove($connection_id, $db_name, $dimension_name, $element_name, 2);
	
	// disconnect the connection to server
	palo_disconnect($connection_id); 

?>
