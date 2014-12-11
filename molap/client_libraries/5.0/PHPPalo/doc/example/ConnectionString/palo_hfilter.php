<?php
  
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// Filter elements of a subset by defining structur criteria
	palo_subset(CONN."/Demo","Products",1,NULL,
	palo_hfilter("Stationary PC's",FALSE,FALSE,2,NULL,NULL,NULL,2,3));
?>