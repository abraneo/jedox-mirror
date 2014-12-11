<?php
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// Filter elements of a subset by defining structur criteria
	palo_subset($connection_id_id, "Demo","Products",1,NULL,
	palo_hfilter("Stationary PC's",FALSE,FALSE,2,NULL,NULL,NULL,2,3));

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>