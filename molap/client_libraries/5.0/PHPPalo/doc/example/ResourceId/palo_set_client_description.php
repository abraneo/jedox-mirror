<?php
	palo_set_client_description('This is my test PHP application version 1.0');
  
	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');  
	
	// registers a name for a PALO server 
	$connection_id_id = palo_init( HOST, PORT, USER, PASS);	
	
	// the created connection will have description in Session Manager 
?>
