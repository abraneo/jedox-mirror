<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	

	// Retrieve a subset of dimension elements 	
	palo_subset($connection_id_id, "Demo", "Products", 1, NULL,
	palo_sort(1,2,NULL,1,3,2,1));

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>
    