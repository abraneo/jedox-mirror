<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);

	// Retrieve a subset of dimension elements 	
	palo_subset(CONN."/Demo", "Products", 1, NULL,
	palo_sort(1,2,NULL,1,3,2,1));
?>
    