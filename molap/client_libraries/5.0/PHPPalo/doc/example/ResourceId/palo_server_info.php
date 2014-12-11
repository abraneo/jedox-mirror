<?php 
   
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server      
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// retrieve information about the server
	palo_server_info($connection_id_id);

	// disconnect the connection to server
	palo_disconnect($connection_id_id); 
?>      
