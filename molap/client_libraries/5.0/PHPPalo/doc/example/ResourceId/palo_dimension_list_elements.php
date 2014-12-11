<?php
		
   	// define the constants
	define('HOST', '127.0.0.1');
	define('PORT', '7777');
	define('USER', 'admin');
	define('PASS', 'admin');
	
	// registers a name for a PALO server
	$connection_id = palo_init(HOST, PORT, USER, PASS);	
	
	// create variable db_name 
	$db_name = 'Demo';
	
	// listing all elements contained in the specified dimension
	palo_dimension_list_elements($connection_id, $db_name, 'Years');

	// the same as above, but elements with insufficient rights will sorted out
	palo_dimension_list_elements($connection_id, $db_name, 'Years', true);

	// disconnect the connection to server
	palo_disconnect($connection_id);
?>

