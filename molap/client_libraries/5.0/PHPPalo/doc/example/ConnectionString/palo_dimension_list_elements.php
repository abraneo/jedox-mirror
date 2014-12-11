<?php
		
   	// define the constants
	define('CONN', 'LOCALHOST');
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection = palo_register_server(CONN, HOST, PORT, USER, PASS);	
	
	// create variable db_name 
	$db_name = 'Demo';
	
	// listing all elements contained in the specified dimension
	palo_dimension_list_elements(CONN . '/' . $db_name, 'Years');

	// the same as above, but elements with insufficient rights will sorted out
	palo_dimension_list_elements(CONN . '/' . $db_name, 'Years', true);
?>

