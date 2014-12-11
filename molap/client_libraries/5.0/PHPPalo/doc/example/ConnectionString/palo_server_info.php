<?php 
   
	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server      
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);
	
	// retrieve information about the server
	palo_server_info(CONN);
?>      
